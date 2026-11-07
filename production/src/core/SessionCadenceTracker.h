#pragma once

#include <QColor>
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <vector>

namespace ClinicalCore {

struct SuitePhase {
    QColor surface;
    QColor elevatedCard;
    QColor specularRim;
    QColor topRightHighlight;
    QColor keyLightTint;
    QColor cardSheen;
    QColor occludedRim;
    QColor textPrimary;
    QColor textSecondary;
    QColor accent;
};

struct ThemeSuite {
    QString name;
    SuitePhase morning;
    SuitePhase day;
    SuitePhase evening;
};

class SessionCadenceTracker : public QObject {
    Q_OBJECT

    Q_PROPERTY(double sessionHorizon READ sessionHorizon WRITE setSessionHorizon NOTIFY horizonChanged)
    Q_PROPERTY(int activeSuiteIndex READ activeSuiteIndex WRITE setActiveSuiteIndex NOTIFY suiteChanged)
    Q_PROPERTY(QString activeSuiteName READ activeSuiteName NOTIFY suiteChanged)

    Q_PROPERTY(QColor surfaceColor READ surfaceColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor elevatedCardColor READ elevatedCardColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor specularRimColor READ specularRimColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor keyLightTint READ keyLightTint NOTIFY paletteChanged)
    Q_PROPERTY(QColor topRightHighlightColor READ topRightHighlightColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor cardSheenColor READ cardSheenColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor occludedRimColor READ occludedRimColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor textPrimaryColor READ textPrimaryColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor textSecondaryColor READ textSecondaryColor NOTIFY paletteChanged)

    // Sun Position & Lighting Properties
    Q_PROPERTY(double sunAngleDeg READ sunAngleDeg NOTIFY lightingUpdated)
    Q_PROPERTY(double sunIntensity READ sunIntensity NOTIFY lightingUpdated)
    Q_PROPERTY(double shadowOffsetX READ shadowOffsetX NOTIFY lightingUpdated)
    Q_PROPERTY(double shadowOffsetY READ shadowOffsetY NOTIFY lightingUpdated)

    // Rainbow Chrome Idle Strike System
    Q_PROPERTY(bool rainbowChromeActive READ rainbowChromeActive NOTIFY chromeStateChanged)
    Q_PROPERTY(double chromeStrikeRate READ chromeStrikeRate NOTIFY chromeStateChanged)
    Q_PROPERTY(int lastTargetCardIndex READ lastTargetCardIndex NOTIFY chromeStrikeFired)

    Q_PROPERTY(bool windowActive READ windowActive NOTIFY windowActiveChanged)
    Q_PROPERTY(bool peripheralCueActive READ peripheralCueActive NOTIFY cadenceUpdated)
    Q_PROPERTY(double peripheralOffsetY READ peripheralOffsetY NOTIFY cadenceUpdated)
    Q_PROPERTY(double peripheralLuminance READ peripheralLuminance NOTIFY cadenceUpdated)
    Q_PROPERTY(double idleSeconds READ idleSeconds NOTIFY cadenceUpdated)

    Q_PROPERTY(double currentFps READ currentFps NOTIFY telemetryUpdated)
    Q_PROPERTY(double frameTimeMs READ frameTimeMs NOTIFY telemetryUpdated)

public:
    explicit SessionCadenceTracker(QObject* parent = nullptr);

    double sessionHorizon() const { return m_sessionHorizon; }
    void setSessionHorizon(double val);

    int activeSuiteIndex() const { return m_activeSuiteIndex; }
    void setActiveSuiteIndex(int idx);
    QString activeSuiteName() const;

    QColor surfaceColor() const;
    QColor elevatedCardColor() const;
    QColor specularRimColor() const;
    QColor keyLightTint() const;
    QColor topRightHighlightColor() const;
    QColor cardSheenColor() const;
    QColor occludedRimColor() const;
    QColor accentColor() const;
    QColor textPrimaryColor() const;
    QColor textSecondaryColor() const;

    double sunAngleDeg() const { return m_sunAngleDeg; }
    double sunIntensity() const { return m_sunIntensity; }
    double shadowOffsetX() const { return m_shadowOffsetX; }
    double shadowOffsetY() const { return m_shadowOffsetY; }

    bool rainbowChromeActive() const { return m_rainbowChromeActive; }
    double chromeStrikeRate() const { return m_chromeStrikeRate; }
    int lastTargetCardIndex() const { return m_lastTargetCardIndex; }

    bool windowActive() const { return m_windowActive; }
    bool peripheralCueActive() const { return m_peripheralCueActive; }
    double peripheralOffsetY() const { return m_peripheralOffsetY; }
    double peripheralLuminance() const { return m_peripheralLuminance; }
    double idleSeconds() const { return m_idleSeconds; }

    double currentFps() const { return m_currentFps; }
    double frameTimeMs() const { return m_frameTimeMs; }

    Q_INVOKABLE void recordUserActivity();
    Q_INVOKABLE void cycleHorizonPhase();
    Q_INVOKABLE void advanceToNextCycleSuite();
    Q_INVOKABLE void resetSessionClock();
    Q_INVOKABLE void notifyFrameSwapped();

signals:
    void horizonChanged();
    void suiteChanged();
    void paletteChanged();
    void lightingUpdated();
    void chromeStateChanged();
    void chromeStrikeFired(int cardIndex);
    void windowActiveChanged();
    void windowReEntered();
    void cadenceUpdated();
    void telemetryUpdated();

private slots:
    void onTick();
    void onAppStateChanged(Qt::ApplicationState state);

private:
    static QColor lerpColor(const QColor& a, const QColor& b, double t);
    static QColor desaturateColor(const QColor& c, double factor);
    void initSuites();

    QElapsedTimer m_sessionClock;
    QElapsedTimer m_idleClock;
    QElapsedTimer m_frameClock;
    QElapsedTimer m_chromeStrikeTimer;
    QTimer m_cadenceTimer;

    std::vector<ThemeSuite> m_suites;
    int m_activeSuiteIndex = 0;
    bool m_manualSuiteOverride = false;

    // 1 Day Cycle = 300 seconds (5 minutes)
    double m_sessionHorizon = 0.50; // Noon default
    bool m_manualHorizonOverride = false;
    bool m_windowActive = true;

    // Sun Physics
    double m_sunAngleDeg = 90.0;
    double m_sunIntensity = 1.0;
    double m_shadowOffsetX = 0.0;
    double m_shadowOffsetY = 10.0;

    // Rainbow Chrome State
    bool m_rainbowChromeActive = false;
    double m_chromeStrikeRate = 0.0;
    int m_lastTargetCardIndex = 0;
    double m_nextStrikeIntervalMs = 2000.0;

    double m_idleSeconds = 0.0;
    bool m_peripheralCueActive = false;
    double m_peripheralOffsetY = 0.0;
    double m_peripheralLuminance = 0.0;
    double m_phaseRadians = 0.0;

    int m_frameCount = 0;
    double m_currentFps = 60.0;
    double m_frameTimeMs = 16.6;
};

} // namespace ClinicalCore
