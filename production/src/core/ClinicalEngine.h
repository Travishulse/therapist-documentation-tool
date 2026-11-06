#pragma once

#include "ClinicalTypes.h"
#include "HybridSearchIndex.h"
#include <QAbstractListModel>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <array>
#include <vector>

namespace ClinicalCore {

class ClinicalEngine : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int activeSection READ activeSection WRITE setActiveSection NOTIFY activeSectionChanged)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QString selectedModality READ selectedModality WRITE setSelectedModality NOTIFY selectedModalityChanged)
    Q_PROPERTY(QString searchMode READ searchMode WRITE setSearchMode NOTIFY searchModeChanged)
    Q_PROPERTY(QStringList availableModalities READ availableModalities NOTIFY dataLoadedChanged)

    Q_PROPERTY(bool modalityHintActive READ modalityHintActive NOTIFY searchResultsChanged)
    Q_PROPERTY(QString detectedModality READ detectedModality NOTIFY searchResultsChanged)
    Q_PROPERTY(int detectedConfidencePercent READ detectedConfidencePercent NOTIFY searchResultsChanged)
    Q_PROPERTY(QString detectedCuesText READ detectedCuesText NOTIFY searchResultsChanged)

    Q_PROPERTY(int resultCount READ resultCount NOTIFY searchResultsChanged)
    Q_PROPERTY(QString topMatchTitle READ topMatchTitle NOTIFY searchResultsChanged)
    Q_PROPERTY(QString topMatchTag READ topMatchTag NOTIFY searchResultsChanged)
    Q_PROPERTY(QString topMatchText READ topMatchText NOTIFY searchResultsChanged)
    Q_PROPERTY(QString topMatchId READ topMatchId NOTIFY searchResultsChanged)
    Q_PROPERTY(bool topMatchSelected READ topMatchSelected NOTIFY selectionChanged)

    Q_PROPERTY(int sheetPage READ sheetPage WRITE setSheetPage NOTIFY sheetPageChanged)
    Q_PROPERTY(int sheetTotalPages READ sheetTotalPages NOTIFY sheetPageChanged)
    Q_PROPERTY(QVariantList sheetCards READ sheetCards NOTIFY sheetPageChanged)

    Q_PROPERTY(int themesCount READ themesCount NOTIFY selectionChanged)
    Q_PROPERTY(int interventionsCount READ interventionsCount NOTIFY selectionChanged)
    Q_PROPERTY(int safetyCount READ safetyCount NOTIFY selectionChanged)
    Q_PROPERTY(int planCount READ planCount NOTIFY selectionChanged)
    Q_PROPERTY(int totalSelectedCount READ totalSelectedCount NOTIFY selectionChanged)

    Q_PROPERTY(QVariantList sectionProgress READ sectionProgress NOTIFY selectionChanged)
    Q_PROPERTY(double endowedProgress READ endowedProgress NOTIFY selectionChanged)
    Q_PROPERTY(int recommendedNextSection READ recommendedNextSection NOTIFY selectionChanged)
    Q_PROPERTY(QVariantList selectedChips READ selectedChips NOTIFY selectionChanged)

    Q_PROPERTY(QString conjoinedNarrative READ conjoinedNarrative WRITE setManualNarrative NOTIFY narrativeChanged)
    Q_PROPERTY(bool isManualEdit READ isManualEdit NOTIFY narrativeChanged)
    Q_PROPERTY(int wordCount READ wordCount NOTIFY narrativeChanged)
    Q_PROPERTY(int charCount READ charCount NOTIFY narrativeChanged)
    Q_PROPERTY(QString lastToggleId READ lastToggleId NOTIFY itemToggled)
    Q_PROPERTY(double lastSearchLatencyMs READ lastSearchLatencyMs NOTIFY searchResultsChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        FullTextRole,
        CategoryRole,
        ModalityRole,
        ShortModalityRole,
        SeverityRole,
        ScoreRole,
        IsSelectedRole
    };

    explicit ClinicalEngine(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool loadFromJsonBytes(const QByteArray& rawJson);
    bool loadDefaultBanks();

    int activeSection() const { return static_cast<int>(m_activeSection); }
    void setActiveSection(int section);

    QString query() const { return m_query; }
    void setQuery(const QString& q);

    QString selectedModality() const { return m_selectedModality; }
    void setSelectedModality(const QString& mod);

    QString searchMode() const { return m_searchMode; }
    void setSearchMode(const QString& mode);

    QStringList availableModalities() const { return m_modalities; }

    bool modalityHintActive() const { return m_currentHint.active && m_currentHint.confidence >= 0.5; }
    QString detectedModality() const { return m_currentHint.modalityName; }
    int detectedConfidencePercent() const { return static_cast<int>(std::round(m_currentHint.confidence * 100.0)); }
    QString detectedCuesText() const { return m_currentHint.matchedCues.join(QStringLiteral(", ")); }

    int resultCount() const { return static_cast<int>(m_currentMatches.size()); }
    QString topMatchTitle() const;
    QString topMatchTag() const;
    QString topMatchText() const;
    QString topMatchId() const;
    bool topMatchSelected() const;

    int sheetPage() const { return m_sheetPage; }
    void setSheetPage(int page);
    int sheetTotalPages() const;
    QVariantList sheetCards() const;

    int themesCount() const { return static_cast<int>(m_selectedRecords[0].size()); }
    int interventionsCount() const { return static_cast<int>(m_selectedRecords[1].size()); }
    int safetyCount() const { return static_cast<int>(m_selectedRecords[2].size()); }
    int planCount() const { return static_cast<int>(m_selectedRecords[3].size()); }
    int totalSelectedCount() const;

    QVariantList sectionProgress() const;
    double endowedProgress() const;
    int recommendedNextSection() const;
    QVariantList selectedChips() const;

    QString conjoinedNarrative() const { return m_narrativeText; }
    void setManualNarrative(const QString& text);
    bool isManualEdit() const { return m_isManualEdit; }
    int wordCount() const;
    int charCount() const { return static_cast<int>(m_narrativeText.size()); }
    QString lastToggleId() const { return m_lastToggleId; }
    double lastSearchLatencyMs() const { return m_lastSearchLatencyMs; }

    int totalCorpusCount() const;

    Q_INVOKABLE void toggleItemById(const QString& id);
    Q_INVOKABLE void removeItemById(const QString& id);
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void regenerateNarrative();
    Q_INVOKABLE bool copyToClipboard() const;
    Q_INVOKABLE bool saveToDefaultNoteFile() const;
    Q_INVOKABLE void quickCompleteRoutineSection(int sectionIdx);
    Q_INVOKABLE void nextSheetPage();
    Q_INVOKABLE void prevSheetPage();
    Q_INVOKABLE void advanceToRecommendedSection();

signals:
    void dataLoadedChanged();
    void activeSectionChanged();
    void queryChanged();
    void selectedModalityChanged();
    void searchModeChanged();
    void searchResultsChanged();
    void sheetPageChanged();
    void selectionChanged();
    void narrativeChanged();
    void itemToggled(const QString& id, bool added);
    void clipboardCopied();
    void fileSaved(const QString& path);

private:
    void executeSearch();
    QString assembleConjoinedText() const;
    void saveDraftState() const;
    void loadDraftState();
    const ClinicalRecord* findRecordById(const QString& id) const;

    std::array<std::vector<ClinicalRecord>, 4> m_banks;
    std::array<HybridSearchIndex, 4> m_indices;
    QStringList m_modalities;

    SectionKind m_activeSection = SectionKind::Interventions;
    QString m_query;
    QString m_selectedModality = QStringLiteral("all");
    QString m_searchMode = QStringLiteral("hybrid");

    std::vector<RankedMatch> m_currentMatches;
    ModalityHint m_currentHint;
    int m_sheetPage = 0;
    double m_lastSearchLatencyMs = 0.0;

    std::array<std::vector<ClinicalRecord>, 4> m_selectedRecords;
    QSet<QString> m_selectedIds;
    QString m_narrativeText;
    bool m_isManualEdit = false;
    QString m_lastToggleId;
};

} // namespace ClinicalCore

