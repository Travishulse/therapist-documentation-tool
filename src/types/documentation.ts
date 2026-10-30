export type QuestionType = 'session_focus' | 'interventions' | 'risk_factors' | 'additional_info';
export type SearchMode = 'hybrid' | 'lexical' | 'semantic';

export interface QuestionConfig {
  id: QuestionType;
  label: string;
  badge: string;
  prompt: string;
  subPrompt: string;
  placeholder: string;
  targetCountText?: string;
  recommendedMin?: number;
  recommendedMax?: number;
}

export interface ClinicalItem {
  id: string;
  question: QuestionType;
  modality?: string;
  shortModality?: string;
  category: string;
  title: string;
  description: string;
  fullText: string;
  severity?: 'mild' | 'moderate' | 'elevated' | 'none';
  keywords?: string[];
}

export interface SearchResult {
  item: ClinicalItem;
  score: number;
  lexicalScore: number;
  semanticScore: number;
  matchedTerms: string[];
}

export interface DocumentationNote {
  session_focus: ClinicalItem[];
  interventions: ClinicalItem[];
  risk_factors: ClinicalItem[];
  additional_info: ClinicalItem[];
  customEdits: {
    session_focus?: string;
    interventions?: string;
    risk_factors?: string;
    additional_info?: string;
  };
}

export interface ModalityCategory {
  name: string;
  shortCode: string;
  count: number;
  color: string;
}
