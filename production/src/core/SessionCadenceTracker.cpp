#include "SessionCadenceTracker.h"
#include <QGuiApplication>
#include <algorithm>
#include <cmath>

namespace ClinicalCore {

SessionCadenceTracker::SessionCadenceTracker(QObject* parent)
    : QObject(parent)
{
    m_sessionClock.start();
    m_idleClock.start();
    m_frameClock.start();

    if (auto* app = qobject_cast<QGuiApplication*>(QGuiApplication::instance())) {
        m_windowActive = (app->applicationState() == Qt::ApplicationActive);
        connect(app, &QGuiApplication::applicationStateChanged,
                this, &SessionCadenceTracker::onAppStateChanged);
    }

    // 30 Hz cadence update for smooth sinusoidal peripheral motion when idle
    m_cadenceTimer.setInterval(33);
    connect(&m_cadenceTimer, &QTimer::timeout, this, &SessionCadenceTracker::onTick);
    m_cadenceTimer.start();
}

void SessionCadenceTracker::setSessionHorizon(double val) {
    const double clamped = std::clamp(val, 0.0, 1.0);
    if (std::abs(m_sessionHorizon - clamped) < 1e-4) {
        return;
    }
    m_sessionHorizon = clamped;
    m_manualHorizonOverride = true;
    emit horizonChanged();
    emit paletteChanged();
}

void SessionCadenceTracker::cycleHorizonPhase() {
    m_manualHorizonOverride = true;
    if (m_sessionHorizon < 0.35) {
        m_sessionHorizon = 0.52; // Calming Sage (15-35 min)
    } else if (m_sessionHorizon < 0.78) {
        m_sessionHorizon = 0.88; // Warm Amber Twilight (35-50 min)
    } else {
        m_sessionHorizon = 0.12; // Morning Slate (0-15 min)
    }
    emit horizonChanged();
    emit paletteChanged();
}

void SessionCadenceTracker::resetSessionClock() {
    m_sessionClock.restart();
    m_manualHorizonOverride = false;
    m_sessionHorizon = 0.08;
    emit horizonChanged();
    emit paletteChanged();
}

void SessionCadenceTracker::recordUserActivity() {
    m_idleClock.restart();
    if (m_peripheralCueActive || m_peripheralOffsetY != 0.0 || m_peripheralLuminance != 0.0) {
        m_idleSeconds = 0.0;
        m_peripheralCueActive = false;
        m_peripheralOffsetY = 0.0;
        m_peripheralLuminance = 0.0;
        emit cadenceUpdated();
    }
}

void SessionCadenceTracker::onAppStateChanged(Qt::ApplicationState state) {
    const bool nowActive = (state == Qt::ApplicationActive);
    if (m_windowActive == nowActive) {
        return;
    }
    const bool wasInactive = !m_windowActive;
    m_windowActive = nowActive;
    emit windowActiveChanged();
    emit paletteChanged();

    if (wasInactive && nowActive) {
        recordUserActivity();
        emit windowReEntered();
    }
}

void SessionCadenceTracker::onTick() {
    if (!m_manualHorizonOverride) {
        const double elapsedSec = static_cast<double>(m_sessionClock.elapsed()) / 1000.0;
        const double cycleTimeSec = 60.0;
        const double phaseInCycle = std::fmod(elapsedSec, cycleTimeSec) / cycleTimeSec;
        const double newHorizon = phaseInCycle;
        if (std::abs(newHorizon - m_sessionHorizon) >= 0.005) {
            m_sessionHorizon = newHorizon;
            emit horizonChanged();
            emit paletteChanged();
        }
    }

    // 2. Bartram, Ware, & Calvert (2003) magnocellular peripheral motion cue after 12s idle
    m_idleSeconds = static_cast<double>(m_idleClock.elapsed()) / 1000.0;
    constexpr double idleThresholdSec = 12.0;

    if (m_idleSeconds >= idleThresholdSec) {
        m_peripheralCueActive = true;
        const double ramp = std::clamp((m_idleSeconds - idleThresholdSec) / 4.0, 0.0, 1.0);
        m_phaseRadians += 0.094; // ~0.45 Hz at 33ms tick
        if (m_phaseRadians > 6.2831853) {
            m_phaseRadians -= 6.2831853;
        }
        // <= 2.4px smooth sinusoidal excursion (sub-startle threshold)
        m_peripheralOffsetY = std::sin(m_phaseRadians) * 2.4 * ramp;
        m_peripheralLuminance = (0.5 + 0.5 * std::sin(m_phaseRadians)) * ramp;
        emit cadenceUpdated();
    } else if (m_peripheralCueActive) {
        m_peripheralCueActive = false;
        m_peripheralOffsetY = 0.0;
        m_peripheralLuminance = 0.0;
        emit cadenceUpdated();
    }
}

void SessionCadenceTracker::notifyFrameSwapped() {
    ++m_frameCount;
    const qint64 elapsedMs = m_frameClock.elapsed();
    if (elapsedMs >= 500) {
        m_currentFps = std::clamp((static_cast<double>(m_frameCount) * 1000.0) / static_cast<double>(elapsedMs), 1.0, 240.0);
        m_frameTimeMs = 1000.0 / m_currentFps;
        m_frameCount = 0;
        m_frameClock.restart();
        emit telemetryUpdated();
    }
}

QColor SessionCadenceTracker::lerpColor(const QColor& a, const QColor& b, double t) {
    const double clamped = std::clamp(t, 0.0, 1.0);
    return QColor::fromRgbF(
        static_cast<float>(a.redF() + (b.redF() - a.redF()) * clamped),
        static_cast<float>(a.greenF() + (b.greenF() - a.greenF()) * clamped),
        static_cast<float>(a.blueF() + (b.blueF() - a.blueF()) * clamped),
        static_cast<float>(a.alphaF() + (b.alphaF() - a.alphaF()) * clamped)
    );
}

QColor SessionCadenceTracker::desaturateColor(const QColor& c, double factor) {
    float h = 0.0f, s = 0.0f, l = 0.0f, a = 1.0f;
    c.getHslF(&h, &s, &l, &a);
    s = static_cast<float>(s * std::clamp(1.0 - factor, 0.0, 1.0));
    return QColor::fromHslF(h < 0.0f ? 0.0f : h, s, l, a);
}

QColor SessionCadenceTracker::surfaceColor() const {
    // Softer, eye-comfort day-cycle palette:
    // Morning Slate (#1B222A) -> Calming Sage (#1A2421) -> Warm Twilight (#24201C)
    const QColor cMorning(0x1B, 0x22, 0x2A);
    const QColor cSage(0x1A, 0x24, 0x21);
    const QColor cAmber(0x24, 0x20, 0x1C);

    QColor activeColor = (m_sessionHorizon < 0.5)
        ? lerpColor(cMorning, cSage, m_sessionHorizon * 2.0)
        : lerpColor(cSage, cAmber, (m_sessionHorizon - 0.5) * 2.0);

    if (!m_windowActive) {
        return desaturateColor(activeColor, 0.25);
    }
    return activeColor;
}

QColor SessionCadenceTracker::elevatedCardColor() const {
    // Softer elevated card backgrounds with warm, low-glare contrast:
    // Morning (#222B36) -> Sage (#202C27) -> Twilight (#2C2621)
    const QColor cMorning(0x22, 0x2B, 0x36);
    const QColor cSage(0x20, 0x2C, 0x27);
    const QColor cAmber(0x2C, 0x26, 0x21);

    QColor activeColor = (m_sessionHorizon < 0.5)
        ? lerpColor(cMorning, cSage, m_sessionHorizon * 2.0)
        : lerpColor(cSage, cAmber, (m_sessionHorizon - 0.5) * 2.0);

    if (!m_windowActive) {
        return desaturateColor(activeColor, 0.25);
    }
    return activeColor;
}

QColor SessionCadenceTracker::specularRimColor() const {
    // Softer specular rims (gentle sky pastel, soft botanical sage, warm peach gold):
    const QColor cMorning(0x7D, 0xAF, 0xE2); // Soft daylight blue
    const QColor cSage(0x6A, 0xC4, 0x9B);    // Soft sage mint
    const QColor cAmber(0xE8, 0xA8, 0x4E);   // Soft twilight amber

    QColor activeColor = (m_sessionHorizon < 0.5)
        ? lerpColor(cMorning, cSage, m_sessionHorizon * 2.0)
        : lerpColor(cSage, cAmber, (m_sessionHorizon - 0.5) * 2.0);

    if (!m_windowActive) {
        return desaturateColor(activeColor, 0.35);
    }
    return activeColor;
}

QColor SessionCadenceTracker::keyLightTint() const {
    const QColor cMorning(0xA5, 0xCE, 0xF5, 42);
    const QColor cSage(0x86, 0xD9, 0xB6, 42);
    const QColor cAmber(0xF5, 0xD4, 0x8A, 42);

    return (m_sessionHorizon < 0.5)
        ? lerpColor(cMorning, cSage, m_sessionHorizon * 2.0)
        : lerpColor(cSage, cAmber, (m_sessionHorizon - 0.5) * 2.0);
}

QColor SessionCadenceTracker::topRightHighlightColor() const {
    const QColor cMorning(0xBD, 0xDE, 0xFF, 210);
    const QColor cSage(0xA2, 0xF2, 0xCF, 210);
    const QColor cAmber(0xFF, 0xDF, 0x9E, 210);

    QColor activeColor = (m_sessionHorizon < 0.5)
        ? lerpColor(cMorning, cSage, m_sessionHorizon * 2.0)
        : lerpColor(cSage, cAmber, (m_sessionHorizon - 0.5) * 2.0);

    if (!m_windowActive) {
        return desaturateColor(activeColor, 0.35);
    }
    return activeColor;
}

QColor SessionCadenceTracker::cardSheenColor() const {
    const QColor cMorning(0xB8, 0xDC, 0xFF, 44);
    const QColor cSage(0x9C, 0xEE, 0xC9, 44);
    const QColor cAmber(0xFF, 0xDE, 0x9E, 44);

    return (m_sessionHorizon < 0.5)
        ? lerpColor(cMorning, cSage, m_sessionHorizon * 2.0)
        : lerpColor(cSage, cAmber, (m_sessionHorizon - 0.5) * 2.0);
}

QColor SessionCadenceTracker::occludedRimColor() const {
    const QColor cMorning(0x11, 0x18, 0x21);
    const QColor cSage(0x10, 0x19, 0x16);
    const QColor cAmber(0x18, 0x14, 0x11);

    return (m_sessionHorizon < 0.5)
        ? lerpColor(cMorning, cSage, m_sessionHorizon * 2.0)
        : lerpColor(cSage, cAmber, (m_sessionHorizon - 0.5) * 2.0);
}

} // namespace ClinicalCore

