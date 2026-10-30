import type { ClinicalItem, SearchMode, SearchResult } from '../types/documentation';
import { BM25Index } from './bm25Index';
import { VectorIndex } from './vectorIndex';

export interface ModalityClassification {
  readonly topModality: string;
  readonly confidence: number;
  readonly detectedCues: string[];
}

export class HybridSearchEngine {
  public readonly items: readonly ClinicalItem[];
  private bm25: BM25Index;
  private vectorIndex: VectorIndex;

  private modalityCues: Record<string, string[]> = {
    'Cognitive Behavioral Therapy (CBT)': [
      'thought', 'distortion', 'automatic', 'belief', 'catastrophiz', 'reframe', 'restructur',
      'socratic', 'evidence', 'worry', 'anxiety', 'trigger', 'activation', 'exposure', 'depress'
    ],
    'Dialectical Behavior Therapy (DBT)': [
      'mindful', 'wise mind', 'distress tolerance', 'radical acceptance', 'emotion regulation',
      'interpersonal', 'dear man', 'fast', 'give', 'tipp', 'stop skill', 'urge surf', 'opposite action',
      'chain analysis', 'impulse', 'overwhelm'
    ],
    'Acceptance and Commitment Therapy (ACT)': [
      'acceptance', 'defusion', 'unhook', 'values', 'committed action', 'experiential avoidance',
      'flexibility', 'present moment', 'self-as-context', 'willingness', 'struggle switch'
    ],
    'Cognitive Processing Therapy (CPT)': [
      'trauma', 'stuck point', 'assimilated', 'overaccommodat', 'blame', 'safety', 'trust',
      'power', 'control', 'esteem', 'intimacy', 'impact statement'
    ],
    'Gottman Method': [
      'gottman', 'four horsemen', 'criticism', 'contempt', 'defensiveness', 'stonewalling',
      'repair attempt', 'flooding', 'bids for connection', 'fondness', 'admiration', 'marital', 'partner'
    ],
    'Family Systems Therapy': [
      'family', 'triangle', 'systemic', 'boundary', 'subsystem', 'alliance', 'coalition',
      'differentiation', 'intergenerational', 'relational pattern'
    ],
    'Psychodynamic Therapy': [
      'unconscious', 'defense mechanism', 'repression', 'transference', 'attachment', 'early childhood',
      'family of origin', 'insight', 'conflict', 'ambivalence', 'guilt', 'shame'
    ],
    'Motivational Interviewing': [
      'ambivalence', 'change talk', 'readiness', 'importance ruler', 'confidence ruler',
      'decisional balance', 'motivation', 'discrepancy', 'resistance'
    ],
    'Discernment Counseling': [
      'discernment', 'mixed agenda', 'leaning in', 'leaning out', 'divorce', 'separation',
      'decision to stay or leave', 'relationship commitment'
    ],
    'Trauma-Focused Therapy': [
      'ptsd', 'hyperarousal', 'flashback', 'grounding', 'nightmare', 'somatic', 'containment',
      'safety stabilization', 'intrusive'
    ],
    'Person-Centered Therapy': [
      'unconditional positive regard', 'empathy', 'congruence', 'active listening',
      'client-centered', 'validation', 'authenticity', 'self-actualization'
    ],
    'CBT for Couples': [
      'couples cbt', 'partner thoughts', 'communication patterns', 'mutual reframing',
      'relationship cognitive distortion', 'shared problem solving'
    ]
  };

  constructor(items: readonly ClinicalItem[]) {
    this.items = items;
    this.bm25 = new BM25Index(items);
    this.vectorIndex = new VectorIndex(items);
  }

  public classifyIntent(query: string): ModalityClassification | null {
    const lower = query.toLowerCase();
    let bestModality = '';
    let maxMatches = 0;
    let detected: string[] = [];

    for (const [modality, cues] of Object.entries(this.modalityCues)) {
      const matched = cues.filter((c) => lower.includes(c));
      if (matched.length > maxMatches) {
        maxMatches = matched.length;
        bestModality = modality;
        detected = matched;
      }
    }

    if (maxMatches > 0) {
      return {
        topModality: bestModality,
        confidence: Math.min(1.0, 0.4 + maxMatches * 0.25),
        detectedCues: detected,
      };
    }
    return null;
  }

  public search(
    query: string,
    filterModality: string = 'all',
    searchMode: SearchMode = 'hybrid',
    limit: number = 25
  ): { results: SearchResult[]; classification: ModalityClassification | null } {
    const classification = this.classifyIntent(query);

    if (!query.trim()) {
      const lexicalOnly = this.bm25.search('', filterModality, limit);
      return { results: lexicalOnly, classification: null };
    }

    if (searchMode === 'lexical') {
      const results = this.bm25.search(query, filterModality, limit);
      return { results, classification };
    }

    if (searchMode === 'semantic') {
      const semMatches = this.vectorIndex.search(query, filterModality, limit);
      const results = semMatches.map((m) => ({
        item: m.item,
        score: m.similarity,
        lexicalScore: 0,
        semanticScore: m.similarity,
        matchedTerms: [],
      }));
      return { results, classification };
    }

    const lexicalResults = this.bm25.search(query, filterModality, limit * 2);
    const semanticResults = this.vectorIndex.search(query, filterModality, limit * 2);

    const scoreMap = new Map<string, {
      item: ClinicalItem;
      rrfScore: number;
      lexicalScore: number;
      semanticScore: number;
      matchedTerms: string[];
    }>();

    const rrfConstant = 60;

    lexicalResults.forEach((res, rank) => {
      const rrf = 1 / (rrfConstant + rank + 1);
      scoreMap.set(res.item.id, {
        item: res.item,
        rrfScore: rrf * 0.6,
        lexicalScore: res.lexicalScore,
        semanticScore: 0,
        matchedTerms: res.matchedTerms,
      });
    });

    semanticResults.forEach((res, rank) => {
      const rrf = 1 / (rrfConstant + rank + 1);
      const current = scoreMap.get(res.item.id);
      if (current) {
        current.rrfScore += rrf * 0.4;
        current.semanticScore = res.similarity;
      } else {
        scoreMap.set(res.item.id, {
          item: res.item,
          rrfScore: rrf * 0.4,
          lexicalScore: 0,
          semanticScore: res.similarity,
          matchedTerms: [],
        });
      }
    });

    if (classification && filterModality === 'all') {
      for (const entry of scoreMap.values()) {
        if (entry.item.modality === classification.topModality) {
          entry.rrfScore *= 1.0 + classification.confidence * 0.5;
        }
      }
    }

    const sorted = Array.from(scoreMap.values())
      .sort((a, b) => b.rrfScore - a.rrfScore)
      .slice(0, limit);

    const maxRrf = sorted.length > 0 ? sorted[0].rrfScore : 1.0;

    const results: SearchResult[] = sorted.map((entry) => ({
      item: entry.item,
      score: entry.rrfScore / (maxRrf || 1.0),
      lexicalScore: entry.lexicalScore,
      semanticScore: entry.semanticScore,
      matchedTerms: entry.matchedTerms,
    }));

    return { results, classification };
  }
}
