import React from 'react';
import type { QuestionType, QuestionConfig } from '../types/documentation';
import { Target, Activity, ShieldAlert, FileText, CheckCircle2 } from 'lucide-react';

interface Props {
  currentQuestion: QuestionType;
  onSelectQuestion: (q: QuestionType) => void;
  selectionCounts: Record<QuestionType, number>;
}

export const QUESTION_CONFIGS: QuestionConfig[] = [
  {
    id: 'session_focus',
    label: 'Session Focus',
    badge: '1-3 Topics',
    prompt: 'List the 1-3 main topics focused on during the session.',
    subPrompt: 'Identify primary clinical presentations, distressing themes, or presenting symptoms.',
    placeholder: 'Search session topics (e.g., anxiety, communication, grief, trauma)...',
    recommendedMin: 1,
    recommendedMax: 3
  },
  {
    id: 'interventions',
    label: 'Interventions',
    badge: '3-5 Interventions',
    prompt: 'What 3-5 interventions were used to address the client\'s presenting problems and goals?',
    subPrompt: 'Search across 615+ clinical interventions across 12 evidence-based modalities.',
    placeholder: 'Type symptom or intervention (e.g., cognitive restructuring, radical acceptance, gottman)...',
    recommendedMin: 3,
    recommendedMax: 5
  },
  {
    id: 'risk_factors',
    label: 'Risk Factors',
    badge: 'Risk & Safety',
    prompt: 'Additional Risk Factors (e.g., substance abuse, NSSI, intimate partner violence, etc.)',
    subPrompt: 'Document acute or monitored clinical risk indicators and safety screening.',
    placeholder: 'Search risk factors (e.g., NSSI, passive ideation, substance, safety plan)...',
    recommendedMin: 0,
    recommendedMax: 3
  },
  {
    id: 'additional_info',
    label: 'Additional Info',
    badge: 'Plan & Goals',
    prompt: 'Additional Information (e.g., homework, goals for next session, referral provided, etc.)',
    subPrompt: 'Record between-session homework assignments, follow-up scheduling, and care coordination.',
    placeholder: 'Search additional items (e.g., thought record, psychiatrist referral, exposure goal)...',
    recommendedMin: 0,
    recommendedMax: 3
  }
];

export const QuestionSelector: React.FC<Props> = ({
  currentQuestion,
  onSelectQuestion,
  selectionCounts
}) => {
  const activeConfig = QUESTION_CONFIGS.find(q => q.id === currentQuestion)!;

  const getIcon = (id: QuestionType) => {
    switch (id) {
      case 'session_focus': return <Target className="w-3.5 h-3.5" />;
      case 'interventions': return <Activity className="w-3.5 h-3.5" />;
      case 'risk_factors': return <ShieldAlert className="w-3.5 h-3.5" />;
      case 'additional_info': return <FileText className="w-3.5 h-3.5" />;
    }
  };

  return (
    <div className="bg-white border border-[#E6E3DC] rounded-2xl p-4 shadow-xs">
      {/* Top Navigation Tabs */}
      <div className="flex flex-wrap items-center gap-2 border-b border-[#EFECE6] pb-3">
        {QUESTION_CONFIGS.map((q) => {
          const isActive = q.id === currentQuestion;
          const count = selectionCounts[q.id] || 0;
          const isComplete = q.recommendedMin ? count >= q.recommendedMin : count > 0;

          return (
            <button
              key={q.id}
              onClick={() => onSelectQuestion(q.id)}
              className={`flex items-center gap-2 px-3.5 py-2 rounded-xl text-xs font-semibold transition-all duration-150 cursor-pointer ${
                isActive
                  ? 'bg-[#4A6B5D] text-white shadow-xs'
                  : 'bg-[#F2F0EB] text-[#555C58] hover:bg-[#EAE7E0] hover:text-[#242927]'
              }`}
            >
              {getIcon(q.id)}
              <span>{q.label}</span>
              {count > 0 && (
                <span className={`px-1.5 py-0.5 rounded-full text-[10px] font-bold ${
                  isActive ? 'bg-white/20 text-white' : 'bg-[#4A6B5D]/15 text-[#3D5B4F]'
                }`}>
                  {count}
                </span>
              )}
              {isComplete && (
                <CheckCircle2 className={`w-3.5 h-3.5 ${isActive ? 'text-emerald-200' : 'text-[#4A6B5D]'}`} />
              )}
            </button>
          );
        })}
      </div>

      {/* Active Question Banner & Clinical Guidance */}
      <div className="mt-3.5 flex flex-col md:flex-row md:items-center justify-between gap-2">
        <div>
          <div className="flex items-center gap-2">
            <span className="text-[11px] font-bold tracking-wider uppercase text-[#4A6B5D] bg-[#EDF3F0] px-2 py-0.5 rounded border border-[#D5E3DC]">
              Active Field: {activeConfig.label}
            </span>
            <span className="text-xs text-[#717A75]">
              Recommended: {activeConfig.badge}
            </span>
          </div>
          <h2 className="text-base font-bold text-[#242927] mt-1">
            {activeConfig.prompt}
          </h2>
          <p className="text-xs text-[#636C67] mt-0.5">
            {activeConfig.subPrompt}
          </p>
        </div>

        {/* Selected count badge */}
        <div className="flex items-center gap-2 bg-[#F6F5F2] px-3.5 py-1.5 rounded-xl border border-[#E8E5DF] self-start md:self-auto">
          <div className="text-right">
            <div className="text-[10px] uppercase tracking-wider text-[#79827D] font-medium">Selected</div>
            <div className="text-sm font-extrabold text-[#3D5B4F]">
              {selectionCounts[currentQuestion] || 0}
              {activeConfig.recommendedMax ? ` / ${activeConfig.recommendedMax}` : ''}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
