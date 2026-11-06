#include "core/ClinicalEngine.h"
#include "core/SessionCadenceTracker.h"
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("LizWegnerSoftwareCollective"));
    app.setApplicationName(QStringLiteral("ClinicalDocumentationCompanion"));
    app.setApplicationVersion(QStringLiteral("2.0.0"));
    app.setWindowIcon(QIcon(QStringLiteral(":/ProductionApp/app_icon.png")));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    ClinicalCore::ClinicalEngine clinicalEngine;
    ClinicalCore::SessionCadenceTracker cadenceTracker;

    const QStringList args = app.arguments();
    const bool smokeTestMode = args.contains(QStringLiteral("--smoke-test"));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("clinicalEngine"), &clinicalEngine);
    engine.rootContext()->setContextProperty(QStringLiteral("cadenceTracker"), &cadenceTracker);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("ProductionApp", "MainScene");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (auto* topWin = qobject_cast<QQuickWindow*>(engine.rootObjects().first())) {
        QObject::connect(topWin, &QQuickWindow::frameSwapped,
                         &cadenceTracker, &ClinicalCore::SessionCadenceTracker::notifyFrameSwapped);
    }

    if (smokeTestMode) {
        QTimer::singleShot(450, &app, []() {
            QCoreApplication::exit(0);
        });
    }

    return app.exec();
}
