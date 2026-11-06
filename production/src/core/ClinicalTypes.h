#pragma once

#include <QString>
#include <QStringList>
#include <vector>

namespace ClinicalCore {

enum class SectionKind : int {
    Themes = 0,
    Interventions = 1,
    Safety = 2,
    Plan = 3
};

struct ClinicalRecord {
    QString id;
    SectionKind section = SectionKind::Interventions;
    QString modality;
    QString shortModality;
    QString category;
    QString title;
    QString description;
    QString fullText;
    QString severity;
};

struct RankedMatch {
    const ClinicalRecord* record = nullptr;
    double score = 0.0;
    double lexicalScore = 0.0;
    double semanticScore = 0.0;
    QStringList matchedTerms;
};

struct ModalityHint {
    bool active = false;
    QString modalityName;
    double confidence = 0.0;
    QStringList matchedCues;
};

} // namespace ClinicalCore

