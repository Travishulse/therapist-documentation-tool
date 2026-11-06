#include "../src/core/ClinicalEngine.h"
#include "../src/core/SessionCadenceTracker.h"
#include <QGuiApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QtTest>
#include <cmath>

class ProductionSuiteTest : public QObject {
    Q_OBJECT

private:
    static void verifyNoTextOverflowInSubtree(QQuickItem* item, QStringList& violations) {
        if (!item || !item->isVisible() || item->opacity() <= 0.01) {
            return;
        }
        const QString className = QString::fromLatin1(item->metaObject()->className());
        if (className.contains(QStringLiteral("QQuickText")) && !className.contains(QStringLiteral("Input")) && !className.contains(QStringLiteral("Edit"))) {
            QQuickItem* p = item->parentItem();
            if (p && p->width() > 8.0 && p->height() > 8.0) {
                const double itemRight = item->x() + item->width();
                const double parentW = p->width();
                if (item->x() < -2.0 || itemRight > parentW + 2.5) {
                    const QString txt = item->property("text").toString();
                    violations.append(QStringLiteral("Horizontal overflow: text='%1' x=%2 w=%3 parentW=%4")
                                      .arg(txt.left(30))
                                      .arg(item->x())
                                      .arg(item->width())
                                      .arg(parentW));
                }
            }
        }
        const QList<QQuickItem*> children = item->childItems();
        for (QQuickItem* child : children) {
            verifyNoTextOverflowInSubtree(child, violations);
        }
    }

    static void countBottomLeftShadows(QQuickItem* item, int& shadowCount) {
        if (!item || !item->isVisible()) {
            return;
        }
        const bool isCardBottomLeftShadow = (item->x() <= -1.0 && item->y() >= 1.0);
        const bool isSurfaceBottomLeftShadow = (item->x() >= 4.0 && item->x() <= 13.0 && item->y() >= 7.0 && item->width() >= 50.0);
        if (isCardBottomLeftShadow || isSurfaceBottomLeftShadow) {
            const QVariant colVar = item->property("color");
            if (colVar.isValid()) {
                const QColor c = colVar.value<QColor>();
                if (c.alpha() > 15 && c.alpha() < 140 && c.red() < 20 && c.green() < 20 && c.blue() < 30) {
                    ++shadowCount;
                }
            }
        }
        const QList<QQuickItem*> children = item->childItems();
        for (QQuickItem* child : children) {
            countBottomLeftShadows(child, shadowCount);
        }
    }

private slots:
    void testEmbeddedCorpusLoaded() {
        ClinicalCore::ClinicalEngine engine;
        QVERIFY2(engine.totalCorpusCount() >= 1500, "Expected full clinical corpus (>=1500 records) loaded from embedded resource.");
        QVERIFY2(engine.availableModalities().size() == 20, "Expected all 20 therapeutic modalities.");
    }

    void testClinicalCorpusLengthsAndSentenceIntegrity() {
        QFile f(QStringLiteral(":/data/clinical_banks.json"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        QVERIFY(doc.isObject());
        const QJsonObject root = doc.object();

        const QStringList bankKeys = {
            QStringLiteral("sessionFocus"),
            QStringLiteral("interventions"),
            QStringLiteral("riskFactors"),
            QStringLiteral("additionalInfo")
        };

        int totalChecked = 0;
        for (const QString& key : bankKeys) {
            const QJsonArray arr = root.value(key).toArray();
            QVERIFY(!arr.isEmpty());
            for (const QJsonValue& val : arr) {
                const QJsonObject obj = val.toObject();
                const QString id = obj.value(QStringLiteral("id")).toString();
                const QString title = obj.value(QStringLiteral("title")).toString();
                const QString desc = obj.value(QStringLiteral("description")).toString();
                const QString cat = obj.value(QStringLiteral("category")).toString();

                QVERIFY2(!title.isEmpty() && title.size() <= 60,
                         qPrintable(QStringLiteral("Invalid title length in %1 (%2): %3").arg(id, key, title)));
                QVERIFY2(desc.size() >= 35 && desc.endsWith(QLatin1Char('.')),
                         qPrintable(QStringLiteral("Truncated or malformed description in %1 (%2): %3").arg(id, key, desc)));
                QVERIFY2(!cat.isEmpty() && cat.size() <= 28 && cat.at(0).isUpper(),
                         qPrintable(QStringLiteral("Invalid category badge in %1 (%2): %3").arg(id, key, cat)));
                ++totalChecked;
            }
        }
        QVERIFY(totalChecked >= 1550);
    }

    void testSubMillisecondHybridSearch() {
        ClinicalCore::ClinicalEngine engine;
        engine.setActiveSection(1);
        engine.setSearchMode(QStringLiteral("hybrid"));
        engine.setQuery(QStringLiteral("cognitive restructuring automatic thoughts"));

        QVERIFY(engine.resultCount() > 0);
        QVERIFY2(engine.lastSearchLatencyMs() < 25.0, "Hybrid search must complete in < 25ms.");
        QVERIFY(engine.topMatchTitle().contains(QStringLiteral("Cognitive"), Qt::CaseInsensitive)
                || engine.topMatchText().contains(QStringLiteral("CBT"), Qt::CaseInsensitive));
        QVERIFY(engine.modalityHintActive());
        QCOMPARE(engine.detectedModality(), QStringLiteral("CBT"));
    }

    void testCowanFourCardSheetChunking() {
        ClinicalCore::ClinicalEngine engine;
        engine.setActiveSection(1);
        engine.setQuery(QString());

        const QVariantList cards = engine.sheetCards();
        QCOMPARE(cards.size(), 4);
        QVERIFY(engine.sheetTotalPages() >= 5);

        const int initialPage = engine.sheetPage();
        engine.nextSheetPage();
        QCOMPARE(engine.sheetPage(), initialPage + 1);
    }

    void testEndowedProgressAndConjoinedNarrative() {
        ClinicalCore::ClinicalEngine engine;
        engine.clearAll();

        QVERIFY(std::abs(engine.endowedProgress() - 0.12) < 0.01);
        QCOMPARE(engine.recommendedNextSection(), 0);

        engine.quickCompleteRoutineSection(0);
        QCOMPARE(engine.themesCount(), 1);
        QCOMPARE(engine.recommendedNextSection(), 1);

        engine.toggleItemById(QStringLiteral("cbt_1"));
        engine.toggleItemById(QStringLiteral("act_1"));
        engine.toggleItemById(QStringLiteral("dbt_1"));
        QCOMPARE(engine.interventionsCount(), 3);
        QCOMPARE(engine.recommendedNextSection(), 2);

        engine.quickCompleteRoutineSection(2);
        engine.quickCompleteRoutineSection(3);
        QCOMPARE(engine.recommendedNextSection(), -1);
        QVERIFY(std::abs(engine.endowedProgress() - 1.0) < 0.001);

        const QString note = engine.conjoinedNarrative();
        QVERIFY(note.contains(QStringLiteral("INTERVENTIONS USED:")));
        QVERIFY(note.contains(QStringLiteral("ADDITIONAL RISK FACTORS:")));
        QVERIFY(note.contains(QStringLiteral("ADDITIONAL INFORMATION & PLAN:")));
        QVERIFY(engine.wordCount() > 25);
    }

    void testCadenceAndTopRightLightingPalette() {
        ClinicalCore::SessionCadenceTracker tracker;
        tracker.setSessionHorizon(0.10);
        const QColor morning = tracker.surfaceColor();
        const QColor topRightHighlight = tracker.topRightHighlightColor();
        const QColor occludedRim = tracker.occludedRimColor();
        const QColor cardSheen = tracker.cardSheenColor();

        QVERIFY(topRightHighlight.lightnessF() > tracker.specularRimColor().lightnessF());
        QVERIFY(occludedRim.lightnessF() < tracker.elevatedCardColor().lightnessF());
        QVERIFY(cardSheen.alpha() > 20 && cardSheen.alpha() < 90);

        tracker.setSessionHorizon(0.90);
        const QColor twilight = tracker.surfaceColor();
        QVERIFY(morning != twilight);

        tracker.recordUserActivity();
        QVERIFY(!tracker.peripheralCueActive());
        QCOMPARE(tracker.peripheralOffsetY(), 0.0);
    }

    void testSceneRatiosShadowsAndTextContainment() {
        QQuickStyle::setStyle(QStringLiteral("Basic"));
        ClinicalCore::ClinicalEngine clinicalEngine;
        ClinicalCore::SessionCadenceTracker cadenceTracker;

        clinicalEngine.clearAll();
        clinicalEngine.toggleItemById(QStringLiteral("sf_3"));
        clinicalEngine.toggleItemById(QStringLiteral("psychodynamic_37"));
        clinicalEngine.toggleItemById(QStringLiteral("rf_1"));
        clinicalEngine.toggleItemById(QStringLiteral("ai_5"));

        QQmlApplicationEngine qmlEngine;
        qmlEngine.rootContext()->setContextProperty(QStringLiteral("clinicalEngine"), &clinicalEngine);
        qmlEngine.rootContext()->setContextProperty(QStringLiteral("cadenceTracker"), &cadenceTracker);

        const QString qmlPath = QStringLiteral(PRODUCTION_SRC_DIR) + QStringLiteral("/ui/MainScene.qml");
        qmlEngine.load(QUrl::fromLocalFile(qmlPath));
        QVERIFY2(!qmlEngine.rootObjects().isEmpty(), "Failed to load MainScene.qml");

        auto* win = qobject_cast<QQuickWindow*>(qmlEngine.rootObjects().first());
        QVERIFY(win != nullptr);
        win->show();
        QVERIFY(QTest::qWaitForWindowExposed(win));

        const QString captureDir = qEnvironmentVariable("CAPTURE_UI_DIR");

        for (int state = 0; state <= 3; ++state) {
            win->setProperty("widgetState", state);
            if (state == 1) {
                clinicalEngine.setActiveSection(1);
                clinicalEngine.setQuery(QStringLiteral("cognitive"));
            } else {
                clinicalEngine.setQuery(QString());
            }
            QTest::qWait(420);

            QStringList violations;
            verifyNoTextOverflowInSubtree(win->contentItem(), violations);
            QVERIFY2(violations.isEmpty(),
                     qPrintable(QStringLiteral("State %1 text overflow violations:\n%2")
                                .arg(state)
                                .arg(violations.join(QLatin1Char('\n')))));

            int shadowCount = 0;
            countBottomLeftShadows(win->contentItem(), shadowCount);
            QVERIFY2(shadowCount >= 4,
                     qPrintable(QStringLiteral("Expected bottom-left cast shadows in state %1, found %2")
                                .arg(state)
                                .arg(shadowCount)));

            if (!captureDir.isEmpty()) {
                QDir().mkpath(captureDir);
                const QImage img = win->grabWindow();
                img.save(captureDir + QStringLiteral("/state_%1.png").arg(state));
            }
        }
    }

    void testZeroForbiddenWordsInProductionSource() {
        const QString srcRoot = QStringLiteral(PRODUCTION_SRC_DIR);
        QDirIterator it(srcRoot, QDir::Files, QDirIterator::Subdirectories);
        const QRegularExpression forbiddenRe(QStringLiteral("(adhd|attention|focus|function)"),
                                             QRegularExpression::CaseInsensitiveOption);

        while (it.hasNext()) {
            const QString filePath = it.next();
            QFile f(filePath);
            QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
            const QString content = QString::fromUtf8(f.readAll());
            const QRegularExpressionMatch m = forbiddenRe.match(content);
            QVERIFY2(!m.hasMatch(),
                     qPrintable(QStringLiteral("Forbidden term '%1' found in %2")
                                .arg(m.captured(0), filePath)));
        }
    }
};

QTEST_MAIN(ProductionSuiteTest)
#include "tst_production_suite.moc"
