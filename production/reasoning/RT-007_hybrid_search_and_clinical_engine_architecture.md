# Reasoning Tree: Native C++20 Hybrid Search & Clinical Engine Architecture

**Decision ID:** RT-007  
**Status:** Approved  
**Date:** 2026-11-06  
**Context:** Designing the core C++20 backend (`production/src/core/`) to load, index, and query all 615+ clinical interventions and multi-category documentation items with sub-millisecond latency, Reciprocal Rank Fusion (BM25 + TF-IDF Cosine Vector), automatic modality detection, and crash-safe local draft persistence.

---

## 1. Academic & Information Retrieval Foundations

1. **Probabilistic Okapi BM25 Lexical Ranking**
   * *Citation:* **Robertson, S. E., & Zaragoza, H. (2009)**. *The Probabilistic Relevance Framework: BM25 and Beyond*. **City University London & Yahoo! Research**, *Foundations and Trends in Information Retrieval*, 3(4), 333–389.
   * *Finding:* BM25 saturates term frequency ($k_1 = 1.4$) and normalizes document length ($b = 0.75$) while weighting title matches and prefix expansions, preventing verbose clinical descriptions from unfairly dominating concise intervention titles.

2. **Reciprocal Rank Fusion (RRF) for Hybrid Lexical + Semantic Retrieval**
   * *Citation:* **Cormack, G. V., Clarke, C. L., & Buettcher, S. (2009)**. *Reciprocal rank fusion outperforms condorcet and individual rank learning methods*. **University of Waterloo**, *Proceedings of the 32nd International ACM SIGIR Conference*, 758–759.
   * *Finding:* Combining heterogeneous rankers (exact/prefix BM25 lexical matching and TF-IDF normalized cosine vector similarity) via Reciprocal Rank Fusion:
     $$\text{RRF}(d) = \sum_{r \in \{L, S\}} \frac{w_r}{60 + \text{rank}_r(d)}$$
     consistently outperforms either ranker alone without requiring fragile score calibration.

---

## 2. Reasoning Tree Structure

```
[Root: Sub-Millisecond Clinical Search & Narrative Synthesis in C++20]
 │
 ├── [Hard Constraints]
 │    ├── C1: 100% Offline & Zero External Network Calls (HIPAA / Clinical Privacy)
 │    ├── C2: < 1.0 ms query execution time on 615+ clinical bank items
 │    ├── C3: Embedded Resource Fallback (Works standalone even if external JSON is moved)
 │    └── C4: Seamless QML Property Binding (`QAbstractListModel` + `Q_PROPERTY` signals)
 │
 ├── [Branch A: External SQLite / Full-Text Search Process]
 │    ├── Evaluation: Querying an external database file on disk per keystroke.
 │    └── Pruning Justification (Rejected):
 │         └── ❌ Disk I/O overhead and external file locking complexity; unnecessary for an in-memory clinical corpus.
 │
 ├── [Branch B: JavaScript-Only Filtering Inside QML]
 │    ├── Evaluation: Running array scans inside QML's V4 JS engine.
 │    └── Pruning Justification (Rejected):
 │         └── ❌ Blocks the UI render thread during rapid typing and lacks C++20 contiguous memory cache locality.
 │
 └── [Branch C: Native C++20 In-Memory Hybrid Engine (`ClinicalEngine` + `ClinicalCardModel`)] ---> [SELECTED]
      ├── Component 1: `ClinicalEngine` (C++20 `QObject` Controller)
      │    • Embeds `clinical_banks.json` directly into the Qt Resource System (`:/data/clinical_banks.json`)
      │      so the binary is 100% self-contained.
      │    • Builds an in-memory inverted index (BM25) + normalized TF-IDF cosine vector index at startup (< 8ms).
      │    • Classifies therapeutic modality cues automatically (CBT, ACT, DBT, CPT, Gottman, Family Systems, etc.).
      │
      ├── Component 2: Conjoined Clinical Narrative Assembler
      │    • Automatically formats selected cards across all 4 sections (`Themes`, `Interventions`, `Safety`, `Plan`)
      │      into structured EHR-ready prose, preserving manual clinician edits until explicitly re-conjoined.
      │
      └── Component 3: Persistent Local Draft & Cadence Tracker
           • Auto-saves selected item IDs and narrative drafts to `QStandardPaths::AppLocalDataLocation`.
           • Tracks session elapsed horizon ($t \in [0, 1]$) and idle intervals to drive subtle peripheral visual cues.
```

