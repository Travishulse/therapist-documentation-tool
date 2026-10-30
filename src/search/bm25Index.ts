import type { ClinicalItem, SearchResult } from '../types/documentation';

interface DocPosting {
  docId: string;
  freq: number;
}

export class BM25Index {
  private items: Map<string, ClinicalItem> = new Map();
  private invertedIndex: Map<string, DocPosting[]> = new Map();
  private docLengths: Map<string, number> = new Map();
  private avgDocLength: number = 0;
  private totalDocs: number = 0;
  private k1: number = 1.4;
  private b: number = 0.75;

  constructor(items: readonly ClinicalItem[]) {
    this.buildIndex(items);
  }

  private tokenize(text: string): string[] {
    return text
      .toLowerCase()
      .replace(/[^a-z0-9\s-]/g, ' ')
      .split(/[\s-]+/)
      .filter((w) => w.length > 1);
  }

  public buildIndex(items: readonly ClinicalItem[]): void {
    this.items.clear();
    this.invertedIndex.clear();
    this.docLengths.clear();

    this.totalDocs = items.length;
    let totalLength = 0;

    for (const item of items) {
      this.items.set(item.id, item);

      const tokens = [
        ...this.tokenize(item.title),
        ...this.tokenize(item.title),
        ...this.tokenize(item.category),
        ...(item.modality ? this.tokenize(item.modality) : []),
        ...this.tokenize(item.description),
      ];

      this.docLengths.set(item.id, tokens.length);
      totalLength += tokens.length;

      const tfMap = new Map<string, number>();
      for (const tok of tokens) {
        tfMap.set(tok, (tfMap.get(tok) || 0) + 1);
      }

      for (const [tok, count] of tfMap.entries()) {
        const postings = this.invertedIndex.get(tok) || [];
        postings.push({ docId: item.id, freq: count });
        this.invertedIndex.set(tok, postings);
      }
    }

    this.avgDocLength = this.totalDocs > 0 ? totalLength / this.totalDocs : 1;
  }

  public search(
    query: string,
    filterModality?: string,
    limit: number = 30
  ): SearchResult[] {
    const queryTokens = this.tokenize(query);
    if (queryTokens.length === 0) {
      let candidates = Array.from(this.items.values());
      if (filterModality && filterModality !== 'all') {
        candidates = candidates.filter(
          (c) => c.modality === filterModality || c.shortModality === filterModality
        );
      }
      return candidates.slice(0, limit).map((item) => ({
        item,
        score: 1.0,
        lexicalScore: 1.0,
        semanticScore: 0.0,
        matchedTerms: [],
      }));
    }

    const docScores = new Map<string, { score: number; matchedTerms: Set<string> }>();

    for (const qToken of queryTokens) {
      const matchingTerms = this.findMatchingTerms(qToken);

      for (const { term, isPrefix } of matchingTerms) {
        const postings = this.invertedIndex.get(term) || [];
        const docFreq = postings.length;
        if (docFreq === 0) continue;

        const idf = Math.log(1 + (this.totalDocs - docFreq + 0.5) / (docFreq + 0.5));
        const prefixDiscount = isPrefix ? 0.75 : 1.0;

        for (const posting of postings) {
          const item = this.items.get(posting.docId);
          if (!item) continue;

          if (filterModality && filterModality !== 'all') {
            if (item.modality !== filterModality && item.shortModality !== filterModality) {
              continue;
            }
          }

          const docLen = this.docLengths.get(posting.docId) || this.avgDocLength;
          const numerator = posting.freq * (this.k1 + 1);
          const denominator =
            posting.freq + this.k1 * (1 - this.b + this.b * (docLen / this.avgDocLength));
          const termScore = idf * (numerator / denominator) * prefixDiscount;

          const current = docScores.get(posting.docId) || {
            score: 0,
            matchedTerms: new Set<string>(),
          };
          current.score += termScore;
          current.matchedTerms.add(term);
          docScores.set(posting.docId, current);
        }
      }
    }

    const lowerQuery = query.toLowerCase().trim();
    if (queryTokens.length > 1) {
      for (const [docId, entry] of docScores.entries()) {
        const item = this.items.get(docId);
        if (!item) continue;
        if (item.fullText.toLowerCase().includes(lowerQuery)) {
          entry.score += 8.0;
        } else if (item.title.toLowerCase().includes(lowerQuery)) {
          entry.score += 15.0;
        }
      }
    }

    const sorted = Array.from(docScores.entries())
      .sort((a, b) => b[1].score - a[1].score)
      .slice(0, limit);

    const maxScore = sorted.length > 0 ? sorted[0][1].score : 1.0;

    return sorted.map(([docId, entry]) => ({
      item: this.items.get(docId)!,
      score: entry.score / (maxScore || 1.0),
      lexicalScore: entry.score,
      semanticScore: 0.0,
      matchedTerms: Array.from(entry.matchedTerms),
    }));
  }

  private findMatchingTerms(prefix: string): { term: string; isPrefix: boolean }[] {
    const results: { term: string; isPrefix: boolean }[] = [];
    if (this.invertedIndex.has(prefix)) {
      results.push({ term: prefix, isPrefix: false });
    }

    if (prefix.length >= 3) {
      let count = 0;
      for (const key of this.invertedIndex.keys()) {
        if (key !== prefix && key.startsWith(prefix)) {
          results.push({ term: key, isPrefix: true });
          count++;
          if (count >= 8) break;
        }
      }
    }

    return results;
  }
}
