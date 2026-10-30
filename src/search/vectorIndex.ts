import type { ClinicalItem } from '../types/documentation';

export interface VectorSearchResult {
  item: ClinicalItem;
  similarity: number;
}

export class VectorIndex {
  private items: readonly ClinicalItem[] = [];
  private itemVectors: Float32Array[] = [];
  private termDict: Map<string, number> = new Map();
  private idfWeights: Float32Array = new Float32Array(0);

  constructor(items: readonly ClinicalItem[]) {
    this.items = items;
    this.buildLocalVectorSpace(items);
  }

  private buildLocalVectorSpace(items: readonly ClinicalItem[]): void {
    const docTokensList: string[][] = [];
    const dfMap = new Map<string, number>();

    for (const item of items) {
      const text = `${item.title} ${item.category} ${item.modality || ''} ${item.description}`.toLowerCase();
      const tokens = Array.from(new Set(text.match(/[a-z]{3,}/g) || []));
      docTokensList.push(tokens);
      for (const t of tokens) {
        dfMap.set(t, (dfMap.get(t) || 0) + 1);
      }
    }

    const vocab = Array.from(dfMap.keys()).sort();
    this.termDict.clear();
    vocab.forEach((term, idx) => {
      this.termDict.set(term, idx);
    });

    const docCount = items.length;
    this.idfWeights = new Float32Array(vocab.length);
    for (let i = 0; i < vocab.length; i++) {
      const df = dfMap.get(vocab[i]) || 1;
      this.idfWeights[i] = Math.log((docCount + 1) / (df + 0.5));
    }

    this.itemVectors = [];
    for (let docIdx = 0; docIdx < docCount; docIdx++) {
      const tokens = docTokensList[docIdx];
      const vec = new Float32Array(vocab.length);
      let norm = 0;

      for (const tok of tokens) {
        const termIdx = this.termDict.get(tok);
        if (termIdx !== undefined) {
          const val = this.idfWeights[termIdx];
          vec[termIdx] = val;
          norm += val * val;
        }
      }

      norm = Math.sqrt(norm);
      if (norm > 0) {
        for (let i = 0; i < vec.length; i++) {
          vec[i] /= norm;
        }
      }
      this.itemVectors.push(vec);
    }
  }

  public search(
    query: string,
    filterModality?: string,
    limit: number = 30
  ): VectorSearchResult[] {
    const queryTokens = query.toLowerCase().match(/[a-z]{3,}/g) || [];
    if (queryTokens.length === 0) return [];

    const qVec = new Float32Array(this.termDict.size);
    let qNorm = 0;
    for (const tok of queryTokens) {
      const termIdx = this.termDict.get(tok);
      if (termIdx !== undefined) {
        const val = this.idfWeights[termIdx];
        qVec[termIdx] += val;
      }
    }

    for (let i = 0; i < qVec.length; i++) {
      qNorm += qVec[i] * qVec[i];
    }
    qNorm = Math.sqrt(qNorm);
    if (qNorm === 0) return [];

    for (let i = 0; i < qVec.length; i++) {
      qVec[i] /= qNorm;
    }

    const results: VectorSearchResult[] = [];

    for (let i = 0; i < this.items.length; i++) {
      const item = this.items[i];
      if (filterModality && filterModality !== 'all') {
        if (item.modality !== filterModality && item.shortModality !== filterModality) {
          continue;
        }
      }

      const docVec = this.itemVectors[i];
      let dotProduct = 0;
      for (let j = 0; j < qVec.length; j++) {
        if (qVec[j] > 0 && docVec[j] > 0) {
          dotProduct += qVec[j] * docVec[j];
        }
      }

      if (dotProduct > 0.05) {
        results.push({ item, similarity: dotProduct });
      }
    }

    results.sort((a, b) => b.similarity - a.similarity);
    return results.slice(0, limit);
  }
}
