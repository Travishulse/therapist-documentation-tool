#include "HybridSearchIndex.h"
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <cmath>

namespace ClinicalCore {

QStringList HybridSearchIndex::tokenizeLexical(const QString& text) {
    QString lower = text.toLower();
    static const QRegularExpression nonAlphaNum(QStringLiteral("[^a-z0-9\\s\\-]"));
    static const QRegularExpression splitter(QStringLiteral("[\\s\\-]+"));
    lower.replace(nonAlphaNum, QStringLiteral(" "));
    const QStringList raw = lower.split(splitter, Qt::SkipEmptyParts);
    QStringList out;
    out.reserve(raw.size());
    for (const QString& w : raw) {
        if (w.size() > 1) {
            out.append(w);
        }
    }
    return out;
}

QStringList HybridSearchIndex::tokenizeSemantic(const QString& text) {
    const QString lower = text.toLower();
    static const QRegularExpression wordRe(QStringLiteral("[a-z]{3,}"));
    QRegularExpressionMatchIterator it = wordRe.globalMatch(lower);
    QSet<QString> uniqueSet;
    while (it.hasNext()) {
        uniqueSet.insert(it.next().captured(0));
    }
    return uniqueSet.values();
}

void HybridSearchIndex::build(const std::vector<ClinicalRecord>& records) {
    m_records = &records;
    m_invertedIndex.clear();
    m_docLengths.assign(records.size(), 0);
    m_vocabIndex.clear();
    m_idfWeights.clear();
    m_sparseVectors.assign(records.size(), {});

    if (records.empty()) {
        m_avgDocLength = 1.0;
        return;
    }

    long long totalTokens = 0;
    QHash<QString, int> dfMap;
    std::vector<QStringList> semDocTokens(records.size());

    for (size_t i = 0; i < records.size(); ++i) {
        const ClinicalRecord& rec = records[i];
        QStringList lexTokens;
        lexTokens.append(tokenizeLexical(rec.title));
        lexTokens.append(tokenizeLexical(rec.title)); // Double weight on title
        lexTokens.append(tokenizeLexical(rec.category));
        if (!rec.modality.isEmpty()) {
            lexTokens.append(tokenizeLexical(rec.modality));
        }
        if (!rec.shortModality.isEmpty()) {
            lexTokens.append(tokenizeLexical(rec.shortModality));
        }
        lexTokens.append(tokenizeLexical(rec.description));

        m_docLengths[i] = static_cast<int>(lexTokens.size());
        totalTokens += lexTokens.size();

        QHash<QString, int> tfMap;
        for (const QString& tok : lexTokens) {
            tfMap[tok] += 1;
        }
        for (auto it = tfMap.constBegin(); it != tfMap.constEnd(); ++it) {
            m_invertedIndex[it.key()].push_back(Posting{static_cast<int>(i), it.value()});
        }

        const QString combined = rec.title + QLatin1Char(' ') + rec.category + QLatin1Char(' ')
            + rec.modality + QLatin1Char(' ') + rec.description;
        semDocTokens[i] = tokenizeSemantic(combined);
        for (const QString& st : semDocTokens[i]) {
            dfMap[st] += 1;
        }
    }

    m_avgDocLength = static_cast<double>(totalTokens) / static_cast<double>(records.size());
    if (m_avgDocLength <= 0.0) {
        m_avgDocLength = 1.0;
    }

    QStringList vocab = dfMap.keys();
    vocab.sort();
    m_idfWeights.resize(vocab.size(), 0.0f);
    const double docCount = static_cast<double>(records.size());

    for (int idx = 0; idx < vocab.size(); ++idx) {
        m_vocabIndex.insert(vocab[idx], idx);
        const int df = dfMap.value(vocab[idx], 1);
        m_idfWeights[idx] = static_cast<float>(std::log((docCount + 1.0) / (static_cast<double>(df) + 0.5)));
    }

    for (size_t i = 0; i < records.size(); ++i) {
        std::vector<std::pair<int, float>> vec;
        vec.reserve(semDocTokens[i].size());
        float normSq = 0.0f;
        for (const QString& tok : semDocTokens[i]) {
            const int termIdx = m_vocabIndex.value(tok, -1);
            if (termIdx >= 0) {
                const float w = m_idfWeights[termIdx];
                vec.emplace_back(termIdx, w);
                normSq += w * w;
            }
        }
        const float norm = std::sqrt(normSq);
        if (norm > 0.0f) {
            for (auto& pair : vec) {
                pair.second /= norm;
            }
        }
        std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
        m_sparseVectors[i] = std::move(vec);
    }
}

std::vector<HybridSearchIndex::TermMatch> HybridSearchIndex::findMatchingTerms(const QString& prefix) const {
    std::vector<TermMatch> out;
    if (m_invertedIndex.contains(prefix)) {
        out.push_back(TermMatch{prefix, false});
    }
    if (prefix.size() >= 3) {
        int count = 0;
        for (auto it = m_invertedIndex.constBegin(); it != m_invertedIndex.constEnd(); ++it) {
            const QString& key = it.key();
            if (key != prefix && key.startsWith(prefix)) {
                out.push_back(TermMatch{key, true});
                ++count;
                if (count >= 8) {
                    break;
                }
            }
        }
    }
    return out;
}

static bool matchesModalityFilter(const ClinicalRecord& rec, const QString& modalityFilter) {
    if (modalityFilter.isEmpty() || modalityFilter == QLatin1String("all")) {
        return true;
    }
    return rec.modality == modalityFilter || rec.shortModality == modalityFilter;
}

std::vector<RankedMatch> HybridSearchIndex::searchLexical(
    const QString& query,
    const QString& modalityFilter,
    int limit) const
{
    std::vector<RankedMatch> results;
    if (!m_records || m_records->empty()) {
        return results;
    }

    const QStringList qTokens = tokenizeLexical(query);
    if (qTokens.isEmpty()) {
        for (const ClinicalRecord& rec : *m_records) {
            if (matchesModalityFilter(rec, modalityFilter)) {
                results.push_back(RankedMatch{&rec, 1.0, 1.0, 0.0, {}});
                if (static_cast<int>(results.size()) >= limit) {
                    break;
                }
            }
        }
        return results;
    }

    struct Accumulator {
        double score = 0.0;
        QSet<QString> matched;
    };
    QHash<int, Accumulator> docScores;

    constexpr double k1 = 1.4;
    constexpr double b = 0.75;
    const double totalDocs = static_cast<double>(m_records->size());

    for (const QString& qTok : qTokens) {
        const auto termMatches = findMatchingTerms(qTok);
        for (const TermMatch& tm : termMatches) {
            const auto postingsIt = m_invertedIndex.constFind(tm.term);
            if (postingsIt == m_invertedIndex.constEnd() || postingsIt->empty()) {
                continue;
            }
            const double docFreq = static_cast<double>(postingsIt->size());
            const double idf = std::log(1.0 + (totalDocs - docFreq + 0.5) / (docFreq + 0.5));
            const double discount = tm.isPrefix ? 0.75 : 1.0;

            for (const Posting& post : *postingsIt) {
                const ClinicalRecord& rec = (*m_records)[post.docIndex];
                if (!matchesModalityFilter(rec, modalityFilter)) {
                    continue;
                }
                const double docLen = static_cast<double>(m_docLengths[post.docIndex]);
                const double num = static_cast<double>(post.frequency) * (k1 + 1.0);
                const double den = static_cast<double>(post.frequency) + k1 * (1.0 - b + b * (docLen / m_avgDocLength));
                const double termScore = idf * (num / den) * discount;

                Accumulator& acc = docScores[post.docIndex];
                acc.score += termScore;
                acc.matched.insert(tm.term);
            }
        }
    }

    const QString lowerTrimmed = query.trimmed().toLower();
    if (qTokens.size() > 1 && !lowerTrimmed.isEmpty()) {
        for (auto it = docScores.begin(); it != docScores.end(); ++it) {
            const ClinicalRecord& rec = (*m_records)[it.key()];
            if (rec.title.toLower().contains(lowerTrimmed)) {
                it->score += 15.0;
            } else if (rec.fullText.toLower().contains(lowerTrimmed)) {
                it->score += 8.0;
            }
        }
    }

    struct IndexedScore {
        int docIdx;
        double score;
        QStringList matched;
    };
    std::vector<IndexedScore> sorted;
    sorted.reserve(docScores.size());
    for (auto it = docScores.constBegin(); it != docScores.constEnd(); ++it) {
        sorted.push_back(IndexedScore{it.key(), it->score, it->matched.values()});
    }
    std::sort(sorted.begin(), sorted.end(), [](const IndexedScore& a, const IndexedScore& b) {
        return a.score > b.score;
    });

    if (static_cast<int>(sorted.size()) > limit) {
        sorted.resize(limit);
    }

    const double maxScore = sorted.empty() ? 1.0 : std::max(1e-6, sorted.front().score);
    results.reserve(sorted.size());
    for (const IndexedScore& is : sorted) {
        results.push_back(RankedMatch{
            &(*m_records)[is.docIdx],
            is.score / maxScore,
            is.score,
            0.0,
            is.matched
        });
    }
    return results;
}

std::vector<RankedMatch> HybridSearchIndex::searchSemantic(
    const QString& query,
    const QString& modalityFilter,
    int limit) const
{
    std::vector<RankedMatch> results;
    if (!m_records || m_records->empty()) {
        return results;
    }

    const QStringList qTokens = tokenizeSemantic(query);
    if (qTokens.isEmpty()) {
        return results;
    }

    QHash<int, float> qMap;
    for (const QString& tok : qTokens) {
        const int idx = m_vocabIndex.value(tok, -1);
        if (idx >= 0) {
            qMap[idx] += m_idfWeights[idx];
        }
    }
    if (qMap.isEmpty()) {
        return results;
    }

    float qNormSq = 0.0f;
    for (auto it = qMap.constBegin(); it != qMap.constEnd(); ++it) {
        qNormSq += it.value() * it.value();
    }
    const float qNorm = std::sqrt(qNormSq);
    if (qNorm <= 0.0f) {
        return results;
    }
    for (auto it = qMap.begin(); it != qMap.end(); ++it) {
        it.value() /= qNorm;
    }

    struct SimEntry {
        int docIdx;
        double sim;
    };
    std::vector<SimEntry> matches;
    for (size_t i = 0; i < m_records->size(); ++i) {
        const ClinicalRecord& rec = (*m_records)[i];
        if (!matchesModalityFilter(rec, modalityFilter)) {
            continue;
        }
        float dot = 0.0f;
        for (const auto& pair : m_sparseVectors[i]) {
            const auto qIt = qMap.constFind(pair.first);
            if (qIt != qMap.constEnd()) {
                dot += (*qIt) * pair.second;
            }
        }
        if (dot > 0.05f) {
            matches.push_back(SimEntry{static_cast<int>(i), static_cast<double>(dot)});
        }
    }

    std::sort(matches.begin(), matches.end(), [](const SimEntry& a, const SimEntry& b) {
        return a.sim > b.sim;
    });
    if (static_cast<int>(matches.size()) > limit) {
        matches.resize(limit);
    }

    results.reserve(matches.size());
    for (const SimEntry& se : matches) {
        results.push_back(RankedMatch{
            &(*m_records)[se.docIdx],
            se.sim,
            0.0,
            se.sim,
            {}
        });
    }
    return results;
}

ModalityHint HybridSearchIndex::classifyModality(const QString& query) const {
    ModalityHint hint;
    const QString lower = query.trimmed().toLower();
    if (lower.isEmpty()) {
        return hint;
    }

    struct CueRule {
        QString shortCode;
        QString fullLabel;
        QStringList cues;
    };

    static const std::vector<CueRule> rules = {
        {QStringLiteral("CBT"), QStringLiteral("Cognitive Behavioral Therapy (CBT)"), {
            QStringLiteral("thought"), QStringLiteral("distortion"), QStringLiteral("automatic"),
            QStringLiteral("belief"), QStringLiteral("catastroph"), QStringLiteral("reframe"),
            QStringLiteral("restructur"), QStringLiteral("socratic"), QStringLiteral("evidence"),
            QStringLiteral("worry"), QStringLiteral("anxiety"), QStringLiteral("activation")
        }},
        {QStringLiteral("DBT"), QStringLiteral("Dialectical Behavior Therapy (DBT)"), {
            QStringLiteral("mindful"), QStringLiteral("wise mind"), QStringLiteral("distress tolerance"),
            QStringLiteral("radical acceptance"), QStringLiteral("emotion regulation"),
            QStringLiteral("dear man"), QStringLiteral("tipp"), QStringLiteral("stop skill"),
            QStringLiteral("urge surf"), QStringLiteral("opposite action"), QStringLiteral("chain analysis")
        }},
        {QStringLiteral("ACT"), QStringLiteral("Acceptance and Commitment Therapy (ACT)"), {
            QStringLiteral("acceptance"), QStringLiteral("defusion"), QStringLiteral("unhook"),
            QStringLiteral("values"), QStringLiteral("committed action"), QStringLiteral("avoidance"),
            QStringLiteral("flexibility"), QStringLiteral("present moment"), QStringLiteral("willingness")
        }},
        {QStringLiteral("CPT"), QStringLiteral("Cognitive Processing Therapy (CPT)"), {
            QStringLiteral("stuck point"), QStringLiteral("assimilated"), QStringLiteral("overaccommodat"),
            QStringLiteral("impact statement"), QStringLiteral("blame"), QStringLiteral("esteem")
        }},
        {QStringLiteral("Gottman"), QStringLiteral("Gottman Method"), {
            QStringLiteral("gottman"), QStringLiteral("four horsemen"), QStringLiteral("criticism"),
            QStringLiteral("contempt"), QStringLiteral("defensiveness"), QStringLiteral("stonewalling"),
            QStringLiteral("repair attempt"), QStringLiteral("flooding"), QStringLiteral("bids"),
            QStringLiteral("fondness"), QStringLiteral("marital")
        }},
        {QStringLiteral("Family Systems"), QStringLiteral("Family Systems Therapy"), {
            QStringLiteral("family"), QStringLiteral("triangle"), QStringLiteral("systemic"),
            QStringLiteral("boundary"), QStringLiteral("subsystem"), QStringLiteral("coalition"),
            QStringLiteral("differentiation"), QStringLiteral("intergenerational")
        }},
        {QStringLiteral("Psychodynamic"), QStringLiteral("Psychodynamic Therapy"), {
            QStringLiteral("unconscious"), QStringLiteral("defense"), QStringLiteral("repression"),
            QStringLiteral("transference"), QStringLiteral("early childhood"), QStringLiteral("origin"),
            QStringLiteral("insight"), QStringLiteral("conflict")
        }},
        {QStringLiteral("Motivational"), QStringLiteral("Motivational Interviewing"), {
            QStringLiteral("ambivalence"), QStringLiteral("change talk"), QStringLiteral("readiness"),
            QStringLiteral("importance ruler"), QStringLiteral("confidence ruler"), QStringLiteral("decisional")
        }},
        {QStringLiteral("Discernment"), QStringLiteral("Discernment Counseling"), {
            QStringLiteral("discernment"), QStringLiteral("mixed agenda"), QStringLiteral("leaning in"),
            QStringLiteral("leaning out"), QStringLiteral("divorce"), QStringLiteral("separation")
        }},
        {QStringLiteral("Person-Centered"), QStringLiteral("Person-Centered Therapy"), {
            QStringLiteral("unconditional positive regard"), QStringLiteral("empathy"),
            QStringLiteral("congruence"), QStringLiteral("active listening"), QStringLiteral("validation")
        }},
        {QStringLiteral("SFBT"), QStringLiteral("SFBT"), {
            QStringLiteral("miracle question"), QStringLiteral("exception"), QStringLiteral("scaling question"),
            QStringLiteral("preferred future"), QStringLiteral("sfbt"), QStringLiteral("coping scale")
        }},
        {QStringLiteral("ERP"), QStringLiteral("Exposure and Response Prevention (ERP)"), {
            QStringLiteral("erp"), QStringLiteral("ritual"), QStringLiteral("compulsion"),
            QStringLiteral("obsession"), QStringLiteral("habituation"), QStringLiteral("inhibitory"),
            QStringLiteral("response prevention"), QStringLiteral("hierarchy")
        }},
        {QStringLiteral("Attachment"), QStringLiteral("Attachment-Based Therapy"), {
            QStringLiteral("attachment"), QStringLiteral("caregiver"), QStringLiteral("abandonment"),
            QStringLiteral("anxious-avoidant"), QStringLiteral("pursue-withdraw"), QStringLiteral("secure base")
        }},
        {QStringLiteral("Somatic"), QStringLiteral("Somatic Therapy"), {
            QStringLiteral("somatic"), QStringLiteral("interocept"), QStringLiteral("body scan"),
            QStringLiteral("nervous system"), QStringLiteral("autonomic"), QStringLiteral("grounding"),
            QStringLiteral("hyperarousal"), QStringLiteral("posture")
        }},
        {QStringLiteral("Existential"), QStringLiteral("Existential Therapy"), {
            QStringLiteral("existential"), QStringLiteral("meaning"), QStringLiteral("mortality"),
            QStringLiteral("isolation"), QStringLiteral("freedom"), QStringLiteral("purpose")
        }}
    };

    int bestCount = 0;
    for (const CueRule& rule : rules) {
        QStringList matched;
        for (const QString& c : rule.cues) {
            if (lower.contains(c)) {
                matched.append(c);
            }
        }
        if (matched.size() > bestCount) {
            bestCount = matched.size();
            hint.active = true;
            hint.modalityName = rule.shortCode;
            hint.confidence = std::min(1.0, 0.4 + static_cast<double>(bestCount) * 0.25);
            hint.matchedCues = matched;
        }
    }
    return hint;
}

std::vector<RankedMatch> HybridSearchIndex::search(
    const QString& query,
    const QString& modalityFilter,
    const QString& mode,
    int limit,
    ModalityHint* outHint) const
{
    const ModalityHint hint = classifyModality(query);
    if (outHint) {
        *outHint = hint;
    }

    if (query.trimmed().isEmpty()) {
        if (outHint) {
            *outHint = ModalityHint{};
        }
        return searchLexical(QString(), modalityFilter, limit);
    }

    if (mode == QLatin1String("lexical")) {
        return searchLexical(query, modalityFilter, limit);
    }
    if (mode == QLatin1String("semantic")) {
        return searchSemantic(query, modalityFilter, limit);
    }

    const auto lexResults = searchLexical(query, modalityFilter, limit * 2);
    const auto semResults = searchSemantic(query, modalityFilter, limit * 2);

    struct RrfEntry {
        const ClinicalRecord* record = nullptr;
        double rrfScore = 0.0;
        double lexicalScore = 0.0;
        double semanticScore = 0.0;
        QStringList matchedTerms;
    };

    QHash<QString, RrfEntry> fused;
    constexpr double rrfK = 60.0;

    for (size_t rank = 0; rank < lexResults.size(); ++rank) {
        const RankedMatch& m = lexResults[rank];
        const double rrf = 0.6 / (rrfK + static_cast<double>(rank) + 1.0);
        fused.insert(m.record->id, RrfEntry{
            m.record,
            rrf,
            m.lexicalScore,
            0.0,
            m.matchedTerms
        });
    }

    for (size_t rank = 0; rank < semResults.size(); ++rank) {
        const RankedMatch& m = semResults[rank];
        const double rrf = 0.4 / (rrfK + static_cast<double>(rank) + 1.0);
        auto it = fused.find(m.record->id);
        if (it != fused.end()) {
            it->rrfScore += rrf;
            it->semanticScore = m.semanticScore;
        } else {
            fused.insert(m.record->id, RrfEntry{
                m.record,
                rrf,
                0.0,
                m.semanticScore,
                {}
            });
        }
    }

    if (hint.active && (modalityFilter.isEmpty() || modalityFilter == QLatin1String("all"))) {
        for (auto it = fused.begin(); it != fused.end(); ++it) {
            if (it->record->shortModality == hint.modalityName
                || it->record->modality == hint.modalityName) {
                it->rrfScore *= (1.0 + hint.confidence * 0.5);
            }
        }
    }

    std::vector<RrfEntry> sorted;
    sorted.reserve(fused.size());
    for (auto it = fused.constBegin(); it != fused.constEnd(); ++it) {
        sorted.push_back(it.value());
    }
    std::sort(sorted.begin(), sorted.end(), [](const RrfEntry& a, const RrfEntry& b) {
        return a.rrfScore > b.rrfScore;
    });

    if (static_cast<int>(sorted.size()) > limit) {
        sorted.resize(limit);
    }

    const double maxRrf = sorted.empty() ? 1.0 : std::max(1e-6, sorted.front().rrfScore);
    std::vector<RankedMatch> out;
    out.reserve(sorted.size());
    for (const RrfEntry& e : sorted) {
        out.push_back(RankedMatch{
            e.record,
            e.rrfScore / maxRrf,
            e.lexicalScore,
            e.semanticScore,
            e.matchedTerms
        });
    }
    return out;
}

} // namespace ClinicalCore

