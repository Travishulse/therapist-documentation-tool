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

export const QUESTION_CONFIGS: QuestionConfig[] = [
  {
    id: 'session_focus',
    label: 'Session Themes',
    badge: '1-3 Themes',
    prompt: 'List the 1-3 main clinical themes addressed during the session.',
    subPrompt: 'Identify primary clinical presentations, distressing patterns, or presenting symptoms (160 themes indexed).',
    placeholder: 'Search session themes (e.g., anxiety, communication, grief, trauma, boundaries)...',
    recommendedMin: 1,
    recommendedMax: 3
  },
  {
    id: 'interventions',
    label: 'Interventions',
    badge: '3-5 Interventions',
    prompt: 'What 3-5 interventions were used to address the client\'s presenting problems and goals?',
    subPrompt: 'Search across 1,308 clinical interventions spanning 20 evidence-based therapeutic modalities.',
    placeholder: 'Type symptom or intervention (e.g., cognitive restructuring, radical acceptance, gottman, sfbt)...',
    recommendedMin: 3,
    recommendedMax: 5
  },
  {
    id: 'risk_factors',
    label: 'Safety & Risk',
    badge: 'Risk & Safety',
    prompt: 'Additional Risk Factors (e.g., substance misuse, NSSI, intimate partner safety, routine screening)',
    subPrompt: 'Document acute or monitored clinical risk indicators and baseline safety screening.',
    placeholder: 'Search risk indicators (e.g., NSSI, passive ideation, substance, no acute risk)...',
    recommendedMin: 1,
    recommendedMax: 3
  },
  {
    id: 'additional_info',
    label: 'Plan & Next Steps',
    badge: 'Plan & Goals',
    prompt: 'Additional Information (e.g., between-session homework, next session goals, referrals, scheduling)',
    subPrompt: 'Record between-session assignments, follow-up scheduling, and care coordination.',
    placeholder: 'Search plan items (e.g., thought record, psychiatric referral, weekly session)...',
    recommendedMin: 1,
    recommendedMax: 3
  }
];

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
