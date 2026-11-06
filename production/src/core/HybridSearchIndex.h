#pragma once

#include "ClinicalTypes.h"
#include <QHash>
#include <QString>
#include <QStringList>
#include <vector>

namespace ClinicalCore {

class HybridSearchIndex {
public:
    HybridSearchIndex() = default;

    void build(const std::vector<ClinicalRecord>& records);

    std::vector<RankedMatch> search(
        const QString& query,
        const QString& modalityFilter,
        const QString& mode,
        int limit,
        ModalityHint* outHint = nullptr) const;

    ModalityHint classifyModality(const QString& query) const;

private:
    struct Posting {
        int docIndex = 0;
        int frequency = 0;
    };

    struct TermMatch {
        QString term;
        bool isPrefix = false;
    };

    static QStringList tokenizeLexical(const QString& text);
    static QStringList tokenizeSemantic(const QString& text);
    std::vector<TermMatch> findMatchingTerms(const QString& prefix) const;

    std::vector<RankedMatch> searchLexical(
        const QString& query,
        const QString& modalityFilter,
        int limit) const;

    std::vector<RankedMatch> searchSemantic(
        const QString& query,
        const QString& modalityFilter,
        int limit) const;

    const std::vector<ClinicalRecord>* m_records = nullptr;
    QHash<QString, std::vector<Posting>> m_invertedIndex;
    std::vector<int> m_docLengths;
    double m_avgDocLength = 1.0;

    // Semantic TF-IDF vector space
    QHash<QString, int> m_vocabIndex;
    std::vector<float> m_idfWeights;
    std::vector<std::vector<std::pair<int, float>>> m_sparseVectors;
};

} // namespace ClinicalCore

