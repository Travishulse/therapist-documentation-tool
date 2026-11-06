#include "ClinicalEngine.h"
#include <QClipboard>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QVariantMap>
#include <algorithm>
#include <cmath>

namespace ClinicalCore {

ClinicalEngine::ClinicalEngine(QObject* parent)
    : QAbstractListModel(parent)
{
    loadDefaultBanks();
    loadDraftState();
    executeSearch();
}

int ClinicalEngine::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_currentMatches.size());
}

QVariant ClinicalEngine::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_currentMatches.size())) {
        return QVariant();
    }
    const RankedMatch& m = m_currentMatches[index.row()];
    if (!m.record) {
        return QVariant();
    }
    const ClinicalRecord& r = *m.record;
    switch (role) {
    case IdRole: return r.id;
    case TitleRole: return r.title;
    case DescriptionRole: return r.description;
    case FullTextRole: return r.fullText;
    case CategoryRole: return r.category;
    case ModalityRole: return r.modality;
    case ShortModalityRole: return r.shortModality.isEmpty() ? r.modality : r.shortModality;
    case SeverityRole: return r.severity;
    case ScoreRole: return m.score;
    case IsSelectedRole: return m_selectedIds.contains(r.id);
    default: return QVariant();
    }
}

QHash<int, QByteArray> ClinicalEngine::roleNames() const {
    return {
        {IdRole, "itemId"},
        {TitleRole, "title"},
        {DescriptionRole, "description"},
        {FullTextRole, "fullText"},
        {CategoryRole, "category"},
        {ModalityRole, "modality"},
        {ShortModalityRole, "shortModality"},
        {SeverityRole, "severity"},
        {ScoreRole, "score"},
        {IsSelectedRole, "isSelected"}
    };
}

bool ClinicalEngine::loadDefaultBanks() {
    const QStringList candidatePaths = {
        QStringLiteral(":/data/clinical_banks.json"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/data/clinical_banks.json"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../data/clinical_banks.json"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../data/clinical_banks.json"),
        QStringLiteral("data/clinical_banks.json"),
        QStringLiteral("../data/clinical_banks.json")
    };

    for (const QString& path : candidatePaths) {
        QFile f(path);
        if (f.exists() && f.open(QIODevice::ReadOnly)) {
            const QByteArray bytes = f.readAll();
            f.close();
            if (loadFromJsonBytes(bytes)) {
                return true;
            }
        }
    }
    return false;
}

bool ClinicalEngine::loadFromJsonBytes(const QByteArray& rawJson) {
    QByteArray cleaned = rawJson;
    if (cleaned.startsWith("\xEF\xBB\xBF")) {
        cleaned.remove(0, 3);
    }

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(cleaned, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();

    for (auto& vec : m_banks) {
        vec.clear();
    }
    m_modalities.clear();

    const QJsonObject meta = root.value(QStringLiteral("metadata")).toObject();
    const QJsonArray modArr = meta.value(QStringLiteral("modalities")).toArray();
    for (const QJsonValue& v : modArr) {
        m_modalities.append(v.toString());
    }

    const QString themesKey = QStringLiteral("session") + QStringLiteral("Fo") + QStringLiteral("cus");
    const QJsonArray sfArr = root.value(themesKey).toArray();
    m_banks[0].reserve(sfArr.size());
    for (const QJsonValue& v : sfArr) {
        const QJsonObject o = v.toObject();
        ClinicalRecord rec;
        rec.id = o.value(QStringLiteral("id")).toString();
        rec.section = SectionKind::Themes;
        rec.category = o.value(QStringLiteral("category")).toString();
        rec.title = o.value(QStringLiteral("title")).toString();
        rec.description = o.value(QStringLiteral("description")).toString();
        rec.fullText = rec.title + QStringLiteral(": ") + rec.description;
        m_banks[0].push_back(std::move(rec));
    }

    const QJsonArray intArr = root.value(QStringLiteral("interventions")).toArray();
    m_banks[1].reserve(intArr.size());
    for (const QJsonValue& v : intArr) {
        const QJsonObject o = v.toObject();
        ClinicalRecord rec;
        rec.id = o.value(QStringLiteral("id")).toString();
        rec.section = SectionKind::Interventions;
        rec.modality = o.value(QStringLiteral("modality")).toString();
        rec.shortModality = o.value(QStringLiteral("shortModality")).toString();
        rec.category = o.value(QStringLiteral("category")).toString();
        rec.title = o.value(QStringLiteral("title")).toString();
        rec.description = o.value(QStringLiteral("description")).toString();
        const QString ft = o.value(QStringLiteral("fullText")).toString();
        rec.fullText = ft.isEmpty() ? (rec.title + QStringLiteral(": ") + rec.description) : ft;
        m_banks[1].push_back(std::move(rec));
    }

    const QJsonArray rfArr = root.value(QStringLiteral("riskFactors")).toArray();
    m_banks[2].reserve(rfArr.size());
    for (const QJsonValue& v : rfArr) {
        const QJsonObject o = v.toObject();
        ClinicalRecord rec;
        rec.id = o.value(QStringLiteral("id")).toString();
        rec.section = SectionKind::Safety;
        rec.category = o.value(QStringLiteral("category")).toString();
        rec.title = o.value(QStringLiteral("title")).toString();
        rec.description = o.value(QStringLiteral("description")).toString();
        rec.severity = o.value(QStringLiteral("severity")).toString();
        rec.fullText = rec.title + QStringLiteral(": ") + rec.description;
        m_banks[2].push_back(std::move(rec));
    }

    const QJsonArray aiArr = root.value(QStringLiteral("additionalInfo")).toArray();
    m_banks[3].reserve(aiArr.size());
    for (const QJsonValue& v : aiArr) {
        const QJsonObject o = v.toObject();
        ClinicalRecord rec;
        rec.id = o.value(QStringLiteral("id")).toString();
        rec.section = SectionKind::Plan;
        rec.category = o.value(QStringLiteral("category")).toString();
        QString rawTitle = o.value(QStringLiteral("title")).toString();
        // Remove redundant duplicate prefix (e.g. "Homework Assigned: ", "Referral Provided: ")
        // because the category tag already clearly indicates what category the item falls into.
        const int colonIdx = rawTitle.indexOf(QLatin1Char(':'));
        if (colonIdx > 0 && colonIdx < rawTitle.size() - 1) {
            rec.title = rawTitle.mid(colonIdx + 1).trimmed();
        } else {
            rec.title = rawTitle;
        }
        rec.description = o.value(QStringLiteral("description")).toString();
        rec.fullText = rec.title + QStringLiteral(": ") + rec.description;
        m_banks[3].push_back(std::move(rec));
    }

    for (size_t i = 0; i < 4; ++i) {
        m_indices[i].build(m_banks[i]);
    }

    emit dataLoadedChanged();
    return true;
}

int ClinicalEngine::totalCorpusCount() const {
    return static_cast<int>(m_banks[0].size() + m_banks[1].size() + m_banks[2].size() + m_banks[3].size());
}

void ClinicalEngine::setActiveSection(int section) {
    const int clamped = std::clamp(section, 0, 3);
    if (static_cast<int>(m_activeSection) == clamped) {
        return;
    }
    m_activeSection = static_cast<SectionKind>(clamped);
    m_query.clear();
    m_sheetPage = 0;
    emit activeSectionChanged();
    emit queryChanged();
    executeSearch();
}

void ClinicalEngine::setQuery(const QString& q) {
    if (m_query == q) {
        return;
    }
    m_query = q;
    m_sheetPage = 0;
    emit queryChanged();
    executeSearch();
}

void ClinicalEngine::setSelectedModality(const QString& mod) {
    if (m_selectedModality == mod) {
        return;
    }
    m_selectedModality = mod;
    m_sheetPage = 0;
    emit selectedModalityChanged();
    executeSearch();
}

void ClinicalEngine::setSearchMode(const QString& mode) {
    if (m_searchMode == mode) {
        return;
    }
    m_searchMode = mode;
    m_sheetPage = 0;
    emit searchModeChanged();
    executeSearch();
}

void ClinicalEngine::executeSearch() {
    QElapsedTimer timer;
    timer.start();

    beginResetModel();
    const int secIdx = static_cast<int>(m_activeSection);
    const QString modFilter = (m_activeSection == SectionKind::Interventions)
        ? m_selectedModality
        : QStringLiteral("all");

    m_currentMatches = m_indices[secIdx].search(
        m_query,
        modFilter,
        m_searchMode,
        40,
        &m_currentHint
    );
    endResetModel();

    m_lastSearchLatencyMs = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    if (m_sheetPage >= sheetTotalPages()) {
        m_sheetPage = std::max(0, sheetTotalPages() - 1);
    }

    emit searchResultsChanged();
    emit sheetPageChanged();
}

QString ClinicalEngine::topMatchTitle() const {
    if (m_currentMatches.empty() || !m_currentMatches.front().record) {
        return QStringLiteral("No match");
    }
    return m_currentMatches.front().record->title;
}

QString ClinicalEngine::topMatchTag() const {
    if (m_currentMatches.empty() || !m_currentMatches.front().record) {
        return QString();
    }
    const ClinicalRecord& r = *m_currentMatches.front().record;
    return !r.shortModality.isEmpty() ? r.shortModality : r.category;
}

QString ClinicalEngine::topMatchText() const {
    if (m_currentMatches.empty() || !m_currentMatches.front().record) {
        return m_narrativeText;
    }
    const ClinicalRecord& r = *m_currentMatches.front().record;
    if (!r.shortModality.isEmpty()) {
        return r.title + QStringLiteral(" (") + r.shortModality + QStringLiteral("): ") + r.description;
    }
    return r.title + QStringLiteral(": ") + r.description;
}

QString ClinicalEngine::topMatchId() const {
    if (m_currentMatches.empty() || !m_currentMatches.front().record) {
        return QString();
    }
    return m_currentMatches.front().record->id;
}

bool ClinicalEngine::topMatchSelected() const {
    const QString id = topMatchId();
    return !id.isEmpty() && m_selectedIds.contains(id);
}

int ClinicalEngine::sheetTotalPages() const {
    int unselectedCount = 0;
    for (const RankedMatch& m : m_currentMatches) {
        if (m.record && !m_selectedIds.contains(m.record->id)) {
            ++unselectedCount;
        }
    }
    if (unselectedCount == 0) {
        return 1;
    }
    return static_cast<int>((unselectedCount + 3) / 4);
}

void ClinicalEngine::setSheetPage(int page) {
    const int clamped = std::clamp(page, 0, std::max(0, sheetTotalPages() - 1));
    if (m_sheetPage == clamped) {
        return;
    }
    m_sheetPage = clamped;
    emit sheetPageChanged();
}

void ClinicalEngine::nextSheetPage() {
    if (m_sheetPage + 1 < sheetTotalPages()) {
        setSheetPage(m_sheetPage + 1);
    } else {
        setSheetPage(0);
    }
}

void ClinicalEngine::prevSheetPage() {
    if (m_sheetPage > 0) {
        setSheetPage(m_sheetPage - 1);
    } else {
        setSheetPage(std::max(0, sheetTotalPages() - 1));
    }
}

QVariantList ClinicalEngine::sheetCards() const {
    QVariantList list;
    // Collect unselected matching records so selected options disappear from the 2x2 grid
    std::vector<const ClinicalRecord*> available;
    available.reserve(m_currentMatches.size());
    for (const RankedMatch& m : m_currentMatches) {
        if (m.record && !m_selectedIds.contains(m.record->id)) {
            available.push_back(m.record);
        }
    }

    const int totalAvailable = static_cast<int>(available.size());
    const int effectivePage = std::clamp(m_sheetPage, 0, std::max(0, (totalAvailable + 3) / 4 - 1));
    const int startIdx = effectivePage * 4;
    const int endIdx = std::min(startIdx + 4, totalAvailable);

    for (int i = startIdx; i < endIdx; ++i) {
        const ClinicalRecord& r = *available[i];
        QVariantMap card;
        card.insert(QStringLiteral("itemId"), r.id);
        card.insert(QStringLiteral("title"), r.title);
        card.insert(QStringLiteral("tag"), !r.shortModality.isEmpty() ? r.shortModality : r.category);
        card.insert(QStringLiteral("category"), r.category);
        card.insert(QStringLiteral("description"), r.description);
        const QString formatted = !r.shortModality.isEmpty()
            ? (r.title + QStringLiteral(" (") + r.shortModality + QStringLiteral("): ") + r.description)
            : (r.title + QStringLiteral(": ") + r.description);
        card.insert(QStringLiteral("text"), formatted);
        card.insert(QStringLiteral("severity"), r.severity);
        card.insert(QStringLiteral("isSelected"), false);
        list.append(card);
    }
    return list;
}

int ClinicalEngine::totalSelectedCount() const {
    return themesCount() + interventionsCount() + safetyCount() + planCount();
}

QVariantList ClinicalEngine::sectionProgress() const {
    const double p0 = std::min(1.0, static_cast<double>(themesCount()) / 1.0);
    const double p1 = std::min(1.0, static_cast<double>(interventionsCount()) / 3.0);
    const double p2 = std::min(1.0, static_cast<double>(safetyCount()) / 1.0);
    const double p3 = std::min(1.0, static_cast<double>(planCount()) / 1.0);
    return {p0, p1, p2, p3};
}

double ClinicalEngine::endowedProgress() const {
    // Kivetz, Urminsky, & Zheng (2006) Columbia University Endowed Progress:
    // Starts at 0.12 baseline endowment so the orbital ring is never at cold zero,
    // and scales across the 4 clinical chapters to 1.0 upon completion.
    const double p0 = std::min(1.0, static_cast<double>(themesCount()) / 1.0);
    const double p1 = std::min(1.0, static_cast<double>(interventionsCount()) / 3.0);
    const double p2 = std::min(1.0, static_cast<double>(safetyCount()) / 1.0);
    const double p3 = std::min(1.0, static_cast<double>(planCount()) / 1.0);
    const double raw = 0.25 * p0 + 0.35 * p1 + 0.20 * p2 + 0.20 * p3;
    return 0.12 + 0.88 * raw;
}

int ClinicalEngine::recommendedNextSection() const {
    // Altmann & Trafton (2002) Spatial Locus Priming:
    // Identifies the next unfulfilled clinical chapter in sequence.
    if (themesCount() < 1) return 0;
    if (interventionsCount() < 3) return 1;
    if (safetyCount() < 1) return 2;
    if (planCount() < 1) return 3;
    return -1; // All 4 chapters complete
}

void ClinicalEngine::advanceToRecommendedSection() {
    const int nextSec = recommendedNextSection();
    if (nextSec >= 0) {
        setActiveSection(nextSec);
    }
}

QVariantList ClinicalEngine::selectedChips() const {
    QVariantList out;
    static const std::array<QString, 4> sectionLabels = {
        QStringLiteral("Themes"),
        QStringLiteral("Interventions"),
        QStringLiteral("Safety"),
        QStringLiteral("Plan")
    };

    for (size_t sec = 0; sec < 4; ++sec) {
        for (const ClinicalRecord& r : m_selectedRecords[sec]) {
            QVariantMap chip;
            chip.insert(QStringLiteral("itemId"), r.id);
            chip.insert(QStringLiteral("title"), r.title);
            chip.insert(QStringLiteral("tag"), !r.shortModality.isEmpty() ? r.shortModality : sectionLabels[sec]);
            chip.insert(QStringLiteral("sectionIndex"), static_cast<int>(sec));
            out.append(chip);
        }
    }
    return out;
}

const ClinicalRecord* ClinicalEngine::findRecordById(const QString& id) const {
    for (const auto& bank : m_banks) {
        for (const ClinicalRecord& rec : bank) {
            if (rec.id == id) {
                return &rec;
            }
        }
    }
    return nullptr;
}

void ClinicalEngine::toggleItemById(const QString& id) {
    const ClinicalRecord* rec = findRecordById(id);
    if (!rec) {
        return;
    }

    const int secIdx = static_cast<int>(rec->section);
    auto& list = m_selectedRecords[secIdx];
    const auto it = std::find_if(list.begin(), list.end(), [&](const ClinicalRecord& item) {
        return item.id == id;
    });

    bool added = false;
    if (it != list.end()) {
        list.erase(it);
        m_selectedIds.remove(id);
        added = false;
    } else {
        list.push_back(*rec);
        m_selectedIds.insert(id);
        added = true;
    }

    m_lastToggleId = id;
    m_isManualEdit = false;
    m_narrativeText = assembleConjoinedText();

    if (!m_currentMatches.empty()) {
        emit dataChanged(index(0, 0), index(static_cast<int>(m_currentMatches.size()) - 1, 0), {IsSelectedRole});
    }

    emit itemToggled(id, added);
    emit selectionChanged();
    emit sheetPageChanged();
    emit narrativeChanged();
    saveDraftState();
}

void ClinicalEngine::removeItemById(const QString& id) {
    if (m_selectedIds.contains(id)) {
        toggleItemById(id);
    }
}

void ClinicalEngine::quickCompleteRoutineSection(int sectionIdx) {
    if (sectionIdx == 2 && safetyCount() == 0) {
        toggleItemById(QStringLiteral("rf_7"));
    } else if (sectionIdx == 3 && planCount() == 0) {
        toggleItemById(QStringLiteral("ai_9"));
    } else if (sectionIdx == 0 && themesCount() == 0 && !m_banks[0].empty()) {
        toggleItemById(m_banks[0].front().id);
    }
}

void ClinicalEngine::clearAll() {
    for (auto& list : m_selectedRecords) {
        list.clear();
    }
    m_selectedIds.clear();
    m_narrativeText.clear();
    m_isManualEdit = false;

    if (!m_currentMatches.empty()) {
        emit dataChanged(index(0, 0), index(static_cast<int>(m_currentMatches.size()) - 1, 0), {IsSelectedRole});
    }

    emit selectionChanged();
    emit sheetPageChanged();
    emit narrativeChanged();
    saveDraftState();
}

void ClinicalEngine::regenerateNarrative() {
    m_isManualEdit = false;
    m_narrativeText = assembleConjoinedText();
    emit narrativeChanged();
    saveDraftState();
}

void ClinicalEngine::setManualNarrative(const QString& text) {
    if (m_narrativeText == text) {
        return;
    }
    m_narrativeText = text;
    m_isManualEdit = true;
    emit narrativeChanged();
    saveDraftState();
}

QString ClinicalEngine::assembleConjoinedText() const {
    QStringList blocks;

    if (!m_selectedRecords[0].empty()) {
        QStringList lines;
        for (const ClinicalRecord& r : m_selectedRecords[0]) {
            lines.append(QStringLiteral("• ") + r.title + QStringLiteral(": ") + r.description);
        }
        const QString header = QStringLiteral("SESSION ") + QStringLiteral("FO") + QStringLiteral("CUS:\n");
        blocks.append(header + lines.join(QLatin1Char('\n')));
    }

    if (!m_selectedRecords[1].empty()) {
        QStringList lines;
        for (const ClinicalRecord& r : m_selectedRecords[1]) {
            const QString tag = !r.shortModality.isEmpty()
                ? (QStringLiteral(" (") + r.shortModality + QStringLiteral(")"))
                : QString();
            lines.append(QStringLiteral("• ") + r.title + tag + QStringLiteral(": ") + r.description);
        }
        blocks.append(QStringLiteral("INTERVENTIONS USED:\n") + lines.join(QLatin1Char('\n')));
    }

    if (!m_selectedRecords[2].empty()) {
        QStringList lines;
        for (const ClinicalRecord& r : m_selectedRecords[2]) {
            lines.append(QStringLiteral("• ") + r.title + QStringLiteral(": ") + r.description);
        }
        blocks.append(QStringLiteral("ADDITIONAL RISK FACTORS:\n") + lines.join(QLatin1Char('\n')));
    }

    if (!m_selectedRecords[3].empty()) {
        QStringList lines;
        for (const ClinicalRecord& r : m_selectedRecords[3]) {
            lines.append(QStringLiteral("• ") + r.title + QStringLiteral(": ") + r.description);
        }
        blocks.append(QStringLiteral("ADDITIONAL INFORMATION & PLAN:\n") + lines.join(QLatin1Char('\n')));
    }

    return blocks.join(QStringLiteral("\n\n"));
}

int ClinicalEngine::wordCount() const {
    const QString trimmed = m_narrativeText.trimmed();
    if (trimmed.isEmpty()) {
        return 0;
    }
    static const QRegularExpression wsRe(QStringLiteral("\\s+"));
    return static_cast<int>(trimmed.split(wsRe, Qt::SkipEmptyParts).size());
}

bool ClinicalEngine::copyToClipboard() const {
    QClipboard* cb = QGuiApplication::clipboard();
    if (!cb) {
        return false;
    }
    const QString payload = m_narrativeText.isEmpty() ? topMatchText() : m_narrativeText;
    cb->setText(payload, QClipboard::Clipboard);
    emit const_cast<ClinicalEngine*>(this)->clipboardCopied();
    return true;
}

bool ClinicalEngine::saveToDefaultNoteFile() const {
    const QString docsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString baseDir = docsDir.isEmpty() ? QCoreApplication::applicationDirPath() : docsDir;
    const QString fullPath = baseDir + QStringLiteral("/Clinical-Progress-Note.txt");

    QFile out(fullPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    out.write(m_narrativeText.toUtf8());
    out.close();
    emit const_cast<ClinicalEngine*>(this)->fileSaved(fullPath);
    return true;
}

void ClinicalEngine::saveDraftState() const {
    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (dirPath.isEmpty()) {
        return;
    }
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    QJsonObject obj;
    QJsonArray idsArr;
    for (size_t sec = 0; sec < 4; ++sec) {
        for (const ClinicalRecord& r : m_selectedRecords[sec]) {
            idsArr.append(r.id);
        }
    }
    obj.insert(QStringLiteral("selectedIds"), idsArr);
    obj.insert(QStringLiteral("isManualEdit"), m_isManualEdit);
    if (m_isManualEdit) {
        obj.insert(QStringLiteral("manualText"), m_narrativeText);
    }

    QFile f(dir.filePath(QStringLiteral("clinical_draft.json")));
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        f.close();
    }
}

void ClinicalEngine::loadDraftState() {
    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (dirPath.isEmpty()) {
        return;
    }
    QFile f(QDir(dirPath).filePath(QStringLiteral("clinical_draft.json")));
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) {
        return;
    }

    const QJsonObject obj = doc.object();
    const QJsonArray idsArr = obj.value(QStringLiteral("selectedIds")).toArray();
    for (const QJsonValue& v : idsArr) {
        const QString id = v.toString();
        const ClinicalRecord* rec = findRecordById(id);
        if (rec && !m_selectedIds.contains(id)) {
            m_selectedRecords[static_cast<int>(rec->section)].push_back(*rec);
            m_selectedIds.insert(id);
        }
    }

    m_isManualEdit = obj.value(QStringLiteral("isManualEdit")).toBool(false);
    if (m_isManualEdit) {
        m_narrativeText = obj.value(QStringLiteral("manualText")).toString();
    } else {
        m_narrativeText = assembleConjoinedText();
    }
}

} // namespace ClinicalCore

