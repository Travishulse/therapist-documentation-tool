#pragma once

#include <QColor>
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

namespace ClinicalCore {

class SessionCadenceTracker : public QObject {
    Q_OBJECT

    Q_PROPERTY(double sessionHorizon READ sessionHorizon WRITE setSessionHorizon NOTIFY horizonChanged)
    Q_PROPERTY(QColor surfaceColor READ surfaceColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor elevatedCardColor READ elevatedCardColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor specularRimColor READ specularRimColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor keyLightTint READ keyLightTint NOTIFY paletteChanged)
    Q_PROPERTY(QColor topRightHighlightColor READ topRightHighlightColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor cardSheenColor READ cardSheenColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor occludedRimColor READ occludedRimColor NOTIFY paletteChanged)

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

    QColor surfaceColor() const;
    QColor elevatedCardColor() const;
    QColor specularRimColor() const;
    QColor keyLightTint() const;
    QColor topRightHighlightColor() const;
    QColor cardSheenColor() const;
    QColor occludedRimColor() const;

    bool windowActive() const { return m_windowActive; }
    bool peripheralCueActive() const { return m_peripheralCueActive; }
    double peripheralOffsetY() const { return m_peripheralOffsetY; }
    double peripheralLuminance() const { return m_peripheralLuminance; }
    double idleSeconds() const { return m_idleSeconds; }

    double currentFps() const { return m_currentFps; }
    double frameTimeMs() const { return m_frameTimeMs; }

    Q_INVOKABLE void recordUserActivity();
    Q_INVOKABLE void cycleHorizonPhase();
    Q_INVOKABLE void resetSessionClock();
    Q_INVOKABLE void notifyFrameSwapped();

signals:
    void horizonChanged();
    void paletteChanged();
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

    QElapsedTimer m_sessionClock;
    QElapsedTimer m_idleClock;
    QElapsedTimer m_frameClock;
    QTimer m_cadenceTimer;

    double m_sessionHorizon = 0.18;
    bool m_manualHorizonOverride = false;
    bool m_windowActive = true;

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

