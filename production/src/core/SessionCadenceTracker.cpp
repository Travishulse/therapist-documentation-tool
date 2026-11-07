#include "SessionCadenceTracker.h"
#include <QGuiApplication>
#include <algorithm>
#include <cmath>

namespace ClinicalCore {

SessionCadenceTracker::SessionCadenceTracker(QObject* parent)
    : QObject(parent)
{
    initSuites();

    m_sessionClock.start();
    m_idleClock.start();
    m_frameClock.start();
    m_chromeStrikeTimer.start();

    if (auto* app = qobject_cast<QGuiApplication*>(QGuiApplication::instance())) {
        m_windowActive = (app->applicationState() == Qt::ApplicationActive);
        connect(app, &QGuiApplication::applicationStateChanged,
                this, &SessionCadenceTracker::onAppStateChanged);
    }

    // 30 Hz tick for cadence, sun motion, and rainbow chrome checks
    m_cadenceTimer.setInterval(33);
    connect(&m_cadenceTimer, &QTimer::timeout, this, &SessionCadenceTracker::onTick);
    m_cadenceTimer.start();
}

void SessionCadenceTracker::initSuites() {
    m_suites.clear();
    m_suites.reserve(10);

    // 1: Birch Wood -> Botanical Sage -> Spiced Persimmon
    m_suites.push_back({
        "Birch Wood -> Botanical Sage -> Spiced Persimmon",
        { QColor("#EADCC9"), QColor("#F4EAE0"), QColor(215, 182, 142, 140), QColor(240, 218, 185, 190), QColor(255, 246, 230, 115), QColor(248, 230, 205, 65), QColor(65, 46, 28, 20), QColor("#32271C"), QColor("#635242"), QColor("#8A6440") },
        { QColor("#E2EBE4"), QColor("#EDF3EE"), QColor(118, 166, 136, 115), QColor(160, 206, 177, 165), QColor(255, 252, 244, 90), QColor(195, 226, 207, 50), QColor(32, 48, 38, 25), QColor("#1F2E23"), QColor("#4A6352"), QColor("#3D6E4E") },
        { QColor("#2E1E16"), QColor("#3C281F"), QColor(232, 130, 66, 115), QColor(255, 175, 115, 155), QColor(255, 195, 140, 65), QColor(247, 159, 94, 45), QColor(18, 10, 6, 165), QColor("#FCEEE5"), QColor("#D8BAA9"), QColor("#E88242") }
    });

    // 2: Golden Oak -> Forest Moss & Olive -> Sunset Amber
    m_suites.push_back({
        "Golden Oak -> Forest Moss & Olive -> Sunset Amber",
        { QColor("#EAD8BE"), QColor("#F5E7D2"), QColor(218, 178, 120, 140), QColor(246, 220, 170, 190), QColor(255, 248, 230, 115), QColor(250, 232, 198, 65), QColor(68, 48, 22, 20), QColor("#352714"), QColor("#6A5537"), QColor("#946A33") },
        { QColor("#E5E9DF"), QColor("#EEF2E9"), QColor(132, 160, 112, 115), QColor(176, 204, 158, 165), QColor(255, 252, 242, 90), QColor(208, 228, 194, 50), QColor(38, 48, 30, 25), QColor("#242E1D"), QColor("#526348"), QColor("#4E6838") },
        { QColor("#2C1F15"), QColor("#3A2A1E"), QColor(238, 146, 53, 115), QColor(255, 185, 110, 155), QColor(255, 208, 140, 65), QColor(255, 169, 77, 45), QColor(16, 10, 6, 165), QColor("#FDF2E6"), QColor("#D8BEA6"), QColor("#EE9235") }
    });

    // 3: Bleached Cedar -> Dusty Rose -> Terracotta Ember
    m_suites.push_back({
        "Bleached Cedar -> Dusty Rose -> Terracotta Ember",
        { QColor("#E8DCD2"), QColor("#F3E9E0"), QColor(215, 185, 162, 140), QColor(244, 222, 204, 190), QColor(255, 248, 236, 115), QColor(250, 234, 220, 65), QColor(64, 44, 32, 20), QColor("#342820"), QColor("#685447"), QColor("#8E5F46") },
        { QColor("#F2E4E6"), QColor("#FAF0F1"), QColor(214, 156, 168, 115), QColor(245, 196, 206, 165), QColor(255, 250, 248, 90), QColor(250, 220, 226, 50), QColor(58, 34, 40, 25), QColor("#362026"), QColor("#6E4E56"), QColor("#8C4B58") },
        { QColor("#301E17"), QColor("#3E2920"), QColor(231, 119, 71, 115), QColor(255, 168, 128, 155), QColor(255, 198, 150, 65), QColor(245, 146, 101, 45), QColor(18, 10, 6, 165), QColor("#FCEEE7"), QColor("#D8B8AB"), QColor("#E77747") }
    });

    // 4: Honey Pine -> Wild Peony Blush -> Golden Campfire
    m_suites.push_back({
        "Honey Pine -> Wild Peony Blush -> Golden Campfire",
        { QColor("#EBDAC4"), QColor("#F5E9D8"), QColor(220, 184, 134, 140), QColor(248, 224, 184, 190), QColor(255, 250, 238, 115), QColor(252, 236, 206, 65), QColor(65, 46, 24, 20), QColor("#362615"), QColor("#6A5338"), QColor("#946733") },
        { QColor("#F5E5E2"), QColor("#FAF1EE"), QColor(224, 162, 150, 115), QColor(250, 200, 190, 165), QColor(255, 252, 248, 90), QColor(255, 226, 218, 50), QColor(60, 36, 32, 25), QColor("#38231F"), QColor("#725049"), QColor("#965349") },
        { QColor("#2B1E13"), QColor("#38271A"), QColor(242, 157, 56, 115), QColor(255, 195, 115, 155), QColor(255, 218, 145, 65), QColor(255, 184, 92, 45), QColor(16, 10, 4, 165), QColor("#FDF3E7"), QColor("#D8BFA7"), QColor("#F29D38") }
    });

    // 5: Sandalwood -> Earthy Mauve Pink -> Spiced Paprika
    m_suites.push_back({
        "Sandalwood -> Earthy Mauve Pink -> Spiced Paprika",
        { QColor("#E7D9CB"), QColor("#F2E7DC"), QColor(212, 180, 150, 140), QColor(242, 218, 194, 190), QColor(255, 248, 238, 115), QColor(248, 230, 212, 65), QColor(62, 44, 28, 20), QColor("#33261A"), QColor("#665240"), QColor("#8A623F") },
        { QColor("#EFE2E4"), QColor("#F7ECEE"), QColor(206, 156, 168, 115), QColor(238, 196, 206, 165), QColor(255, 250, 248, 90), QColor(245, 218, 224, 50), QColor(54, 34, 40, 25), QColor("#342026"), QColor("#6A4B54"), QColor("#804E5C") },
        { QColor("#2F1D18"), QColor("#3D2721"), QColor(226, 116, 75, 115), QColor(255, 166, 130, 155), QColor(255, 196, 155, 65), QColor(240, 145, 109, 45), QColor(18, 10, 6, 165), QColor("#FCEEE8"), QColor("#D8B7AC"), QColor("#E2744B") }
    });

    // 6: Pale Ash Wood -> Heather Lavender -> Apricot Twilight
    m_suites.push_back({
        "Pale Ash Wood -> Heather Lavender -> Apricot Twilight",
        { QColor("#E7DFD4"), QColor("#F2EBE1"), QColor(210, 190, 165, 140), QColor(240, 225, 204, 190), QColor(255, 250, 240, 115), QColor(246, 234, 218, 65), QColor(60, 48, 35, 20), QColor("#322A20"), QColor("#635645"), QColor("#846B4F") },
        { QColor("#EAE4F0"), QColor("#F4F0F8"), QColor(192, 168, 212, 115), QColor(224, 204, 240, 165), QColor(255, 252, 252, 90), QColor(235, 222, 246, 50), QColor(48, 38, 56, 25), QColor("#2B2134"), QColor("#5D4E68"), QColor("#664E78") },
        { QColor("#2D201A"), QColor("#3C2B23"), QColor(233, 139, 80, 115), QColor(255, 184, 135, 155), QColor(255, 208, 160, 65), QColor(250, 171, 115, 45), QColor(18, 10, 6, 165), QColor("#FDF0E8"), QColor("#D9BBB0"), QColor("#E98B50") }
    });

    // 7: Raw Teak Wood -> Calming Wisteria -> Citrus Ember
    m_suites.push_back({
        "Raw Teak Wood -> Calming Wisteria -> Citrus Ember",
        { QColor("#E6D7C2"), QColor("#F2E6D4"), QColor(215, 180, 135, 140), QColor(244, 218, 180, 190), QColor(255, 248, 235, 115), QColor(248, 230, 202, 65), QColor(64, 46, 26, 20), QColor("#342616"), QColor("#675238"), QColor("#8E6535") },
        { QColor("#E6E2EE"), QColor("#F2EFF8"), QColor(182, 164, 208, 115), QColor(216, 200, 238, 165), QColor(255, 252, 252, 90), QColor(230, 218, 244, 50), QColor(45, 36, 54, 25), QColor("#292132"), QColor("#584C66"), QColor("#594B73") },
        { QColor("#2A1D12"), QColor("#382718"), QColor(245, 159, 46, 115), QColor(255, 198, 110, 155), QColor(255, 218, 140, 65), QColor(255, 189, 84, 45), QColor(16, 10, 4, 165), QColor("#FDF2E5"), QColor("#D8BEA3"), QColor("#F59F2E") }
    });

    // 8: Washed Birch -> Deep Plum Fig -> Desert Canyon
    m_suites.push_back({
        "Washed Birch -> Deep Plum Fig -> Desert Canyon",
        { QColor("#E9DDCF"), QColor("#F4E9DC"), QColor(215, 184, 148, 140), QColor(244, 222, 192, 190), QColor(255, 248, 238, 115), QColor(248, 232, 210, 65), QColor(64, 46, 30, 20), QColor("#33261A"), QColor("#665240"), QColor("#88623E") },
        { QColor("#ECE3EA"), QColor("#F6EFF4"), QColor(196, 158, 186, 115), QColor(228, 196, 220, 165), QColor(255, 250, 252, 90), QColor(238, 212, 232, 50), QColor(52, 34, 48, 25), QColor("#321F2C"), QColor("#66495D"), QColor("#6F4563") },
        { QColor("#311E16"), QColor("#40281F"), QColor(235, 124, 64, 115), QColor(255, 172, 120, 155), QColor(255, 202, 150, 65), QColor(248, 153, 96, 45), QColor(18, 10, 6, 165), QColor("#FCEEE6"), QColor("#D8B8A8"), QColor("#EB7C40") }
    });

    // 9: Coastal Driftwood -> Sea Slate Blue -> Smoked Marigold
    m_suites.push_back({
        "Coastal Driftwood -> Sea Slate Blue -> Smoked Marigold",
        { QColor("#E5DCD3"), QColor("#F0E8E1"), QColor(208, 185, 164, 140), QColor(238, 222, 205, 190), QColor(255, 248, 240, 115), QColor(244, 232, 220, 65), QColor(60, 46, 36, 20), QColor("#312822"), QColor("#63554B"), QColor("#826653") },
        { QColor("#E0E9ED"), QColor("#ECF3F6"), QColor(138, 180, 196, 115), QColor(178, 215, 230, 165), QColor(250, 252, 255, 90), QColor(205, 230, 240, 50), QColor(30, 46, 54, 25), QColor("#1B2A32"), QColor("#455E6B"), QColor("#3B6778") },
        { QColor("#2A1E14"), QColor("#37281B"), QColor(240, 152, 50, 115), QColor(255, 192, 114, 155), QColor(255, 214, 140, 65), QColor(255, 180, 78, 45), QColor(16, 10, 4, 165), QColor("#FDF2E6"), QColor("#D8BEA4"), QColor("#F09832") }
    });

    // 10: Nordic Birch -> River Mist & Cornflower -> Baltic Amber
    m_suites.push_back({
        "Nordic Birch -> River Mist & Cornflower -> Baltic Amber",
        { QColor("#E7DDD0"), QColor("#F2E9DE"), QColor(214, 185, 152, 140), QColor(244, 222, 196, 190), QColor(255, 248, 238, 115), QColor(248, 232, 214, 65), QColor(62, 45, 30, 20), QColor("#32261B"), QColor("#655341"), QColor("#866341") },
        { QColor("#E2E6EE"), QColor("#EEF2F8"), QColor(142, 168, 206, 115), QColor(182, 204, 238, 165), QColor(250, 252, 255, 90), QColor(210, 224, 246, 50), QColor(32, 42, 56, 25), QColor("#1D2736"), QColor("#4A5A72"), QColor("#3D597A") },
        { QColor("#2B1D16"), QColor("#39271E"), QColor(234, 134, 68, 115), QColor(255, 178, 125, 155), QColor(255, 204, 150, 65), QColor(245, 160, 100, 45), QColor(18, 10, 6, 165), QColor("#FCEEE7"), QColor("#D8BAAB"), QColor("#EA8644") }
    });
}

QString SessionCadenceTracker::activeSuiteName() const {
    if (m_activeSuiteIndex >= 0 && m_activeSuiteIndex < static_cast<int>(m_suites.size())) {
        return m_suites[m_activeSuiteIndex].name;
    }
    return QStringLiteral("Natural Suite");
}

void SessionCadenceTracker::setActiveSuiteIndex(int idx) {
    const int clamped = std::clamp(idx, 0, static_cast<int>(m_suites.size() - 1));
    if (m_activeSuiteIndex == clamped) return;
    m_activeSuiteIndex = clamped;
    m_manualSuiteOverride = true;
    emit suiteChanged();
    emit paletteChanged();
}

void SessionCadenceTracker::advanceToNextCycleSuite() {
    m_activeSuiteIndex = (m_activeSuiteIndex + 1) % static_cast<int>(m_suites.size());
    emit suiteChanged();
    emit paletteChanged();
}

void SessionCadenceTracker::setSessionHorizon(double val) {
    const double clamped = std::clamp(val, 0.0, 1.0);
    if (std::abs(m_sessionHorizon - clamped) < 1e-4) return;
    m_sessionHorizon = clamped;
    m_manualHorizonOverride = true;
    emit horizonChanged();
    emit paletteChanged();
}

void SessionCadenceTracker::cycleHorizonPhase() {
    m_manualHorizonOverride = true;
    if (m_sessionHorizon < 0.35) {
        m_sessionHorizon = 0.50; // Noon (Day)
    } else if (m_sessionHorizon < 0.68) {
        m_sessionHorizon = 0.88; // Evening Orange
    } else {
        m_sessionHorizon = 0.12; // Morning Wood
        advanceToNextCycleSuite();
    }
    emit horizonChanged();
    emit paletteChanged();
}

void SessionCadenceTracker::resetSessionClock() {
    m_sessionClock.restart();
    m_manualHorizonOverride = false;
    m_sessionHorizon = 0.50;
    emit horizonChanged();
    emit paletteChanged();
}

void SessionCadenceTracker::recordUserActivity() {
    m_idleClock.restart();
    if (m_rainbowChromeActive || m_peripheralCueActive || m_peripheralOffsetY != 0.0 || m_peripheralLuminance != 0.0) {
        m_idleSeconds = 0.0;
        m_peripheralCueActive = false;
        m_peripheralOffsetY = 0.0;
        m_peripheralLuminance = 0.0;
        m_rainbowChromeActive = false;
        m_chromeStrikeRate = 0.0;
        emit cadenceUpdated();
        emit chromeStateChanged();
    }
}

void SessionCadenceTracker::onAppStateChanged(Qt::ApplicationState state) {
    const bool nowActive = (state == Qt::ApplicationActive);
    if (m_windowActive == nowActive) return;
    m_windowActive = nowActive;
    emit windowActiveChanged();
    emit paletteChanged();

    if (nowActive) {
        recordUserActivity();
        emit windowReEntered();
    }
}

void SessionCadenceTracker::onTick() {
    // 1. Five-minute continuous Day Cycle (300 seconds)
    constexpr double kDayCycleSec = 300.0;
    const double elapsedSec = static_cast<double>(m_sessionClock.elapsed()) / 1000.0;

    if (!m_manualHorizonOverride) {
        const double cycleProgress = std::fmod(elapsedSec, kDayCycleSec) / kDayCycleSec;
        const int calculatedSuiteIdx = static_cast<int>(elapsedSec / kDayCycleSec) % static_cast<int>(m_suites.size());

        if (!m_manualSuiteOverride && calculatedSuiteIdx != m_activeSuiteIndex) {
            m_activeSuiteIndex = calculatedSuiteIdx;
            emit suiteChanged();
        }

        if (std::abs(cycleProgress - m_sessionHorizon) >= 0.003) {
            m_sessionHorizon = cycleProgress;
            emit horizonChanged();
            emit paletteChanged();
        }
    }

    // 2. Celestial Sun Physics (Left Morning -> Overhead Zenith Noon -> Right Evening Sunset)
    // Ray angle sweeps from 180 (left) -> 90 (overhead) -> 0 (right)
    m_sunAngleDeg = 180.0 - m_sessionHorizon * 180.0;
    m_sunIntensity = 0.45 + std::sin(m_sessionHorizon * M_PI) * 0.55;

    const double rad = m_sunAngleDeg * (M_PI / 180.0);
    m_shadowOffsetX = -std::cos(rad) * 12.0;
    m_shadowOffsetY = std::max(3.0, std::sin(rad) * 12.0);
    emit lightingUpdated();

    // 3. Rainbow Chrome Idle Sky-Strike System
    // Triggers after 2s idle, ramps to 5.0 strikes/sec at 180s (3 minutes)
    m_idleSeconds = static_cast<double>(m_idleClock.elapsed()) / 1000.0;
    if (m_idleSeconds >= 2.0) {
        if (!m_rainbowChromeActive) {
            m_rainbowChromeActive = true;
            m_chromeStrikeTimer.restart();
        }

        if (m_idleSeconds < 180.0) {
            const double ramp = std::pow((m_idleSeconds - 2.0) / 178.0, 1.35);
            m_chromeStrikeRate = 0.2 + ramp * 4.8;
        } else {
            m_chromeStrikeRate = 5.0; // Maximum ceiling at 3 minutes!
        }
        emit chromeStateChanged();

        const double intervalMs = (1.0 / m_chromeStrikeRate) * 1000.0;
        if (m_chromeStrikeTimer.elapsed() >= intervalMs) {
            m_chromeStrikeTimer.restart();
            m_lastTargetCardIndex = (m_lastTargetCardIndex + 1) % 4;
            emit chromeStrikeFired(m_lastTargetCardIndex);
        }
    } else if (m_rainbowChromeActive) {
        m_rainbowChromeActive = false;
        m_chromeStrikeRate = 0.0;
        emit chromeStateChanged();
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

// Active suite phase interpolation
QColor SessionCadenceTracker::surfaceColor() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    QColor c = (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.surface, s.day.surface, m_sessionHorizon * 2.0)
        : lerpColor(s.day.surface, s.evening.surface, (m_sessionHorizon - 0.5) * 2.0);
    return m_windowActive ? c : desaturateColor(c, 0.25);
}

QColor SessionCadenceTracker::elevatedCardColor() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    QColor c = (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.elevatedCard, s.day.elevatedCard, m_sessionHorizon * 2.0)
        : lerpColor(s.day.elevatedCard, s.evening.elevatedCard, (m_sessionHorizon - 0.5) * 2.0);
    return m_windowActive ? c : desaturateColor(c, 0.25);
}

QColor SessionCadenceTracker::specularRimColor() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    QColor c = (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.specularRim, s.day.specularRim, m_sessionHorizon * 2.0)
        : lerpColor(s.day.specularRim, s.evening.specularRim, (m_sessionHorizon - 0.5) * 2.0);
    return m_windowActive ? c : desaturateColor(c, 0.35);
}

QColor SessionCadenceTracker::keyLightTint() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    return (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.keyLightTint, s.day.keyLightTint, m_sessionHorizon * 2.0)
        : lerpColor(s.day.keyLightTint, s.evening.keyLightTint, (m_sessionHorizon - 0.5) * 2.0);
}

QColor SessionCadenceTracker::topRightHighlightColor() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    QColor c = (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.topRightHighlight, s.day.topRightHighlight, m_sessionHorizon * 2.0)
        : lerpColor(s.day.topRightHighlight, s.evening.topRightHighlight, (m_sessionHorizon - 0.5) * 2.0);
    return m_windowActive ? c : desaturateColor(c, 0.35);
}

QColor SessionCadenceTracker::cardSheenColor() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    return (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.cardSheen, s.day.cardSheen, m_sessionHorizon * 2.0)
        : lerpColor(s.day.cardSheen, s.evening.cardSheen, (m_sessionHorizon - 0.5) * 2.0);
}

QColor SessionCadenceTracker::occludedRimColor() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    return (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.occludedRim, s.day.occludedRim, m_sessionHorizon * 2.0)
        : lerpColor(s.day.occludedRim, s.evening.occludedRim, (m_sessionHorizon - 0.5) * 2.0);
}

QColor SessionCadenceTracker::accentColor() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    return (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.accent, s.day.accent, m_sessionHorizon * 2.0)
        : lerpColor(s.day.accent, s.evening.accent, (m_sessionHorizon - 0.5) * 2.0);
}

QColor SessionCadenceTracker::textPrimaryColor() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    return (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.textPrimary, s.day.textPrimary, m_sessionHorizon * 2.0)
        : lerpColor(s.day.textPrimary, s.evening.textPrimary, (m_sessionHorizon - 0.5) * 2.0);
}

QColor SessionCadenceTracker::textSecondaryColor() const {
    const auto& s = m_suites[m_activeSuiteIndex];
    return (m_sessionHorizon < 0.5)
        ? lerpColor(s.morning.textSecondary, s.day.textSecondary, m_sessionHorizon * 2.0)
        : lerpColor(s.day.textSecondary, s.evening.textSecondary, (m_sessionHorizon - 0.5) * 2.0);
}

} // namespace ClinicalCore
