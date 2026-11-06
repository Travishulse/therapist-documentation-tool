import React from 'react';
import { QUESTION_CONFIGS, type QuestionType } from '../types/documentation';
import { Target, Activity, ShieldAlert, FileText, CheckCircle2, Sparkles } from 'lucide-react';

interface Props {
  currentQuestion: QuestionType;
  onSelectQuestion: (q: QuestionType) => void;
  selectionCounts: Record<QuestionType, number>;
  recommendedNext: QuestionType | null;
  peripheralPulseActive: boolean;
  onQuickCompleteRoutine: (q: QuestionType) => void;
}

export const QuestionSelector: React.FC<Props> = ({
  currentQuestion,
  onSelectQuestion,
  selectionCounts,
  recommendedNext,
  peripheralPulseActive,
  onQuickCompleteRoutine
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
    <div
      style={{
        backgroundImage: 'linear-gradient(225deg, rgba(255,255,255,0.98) 0%, rgba(251,250,247,0.95) 45%, rgba(244,242,236,0.96) 100%)',
        boxShadow: '-5px 6px 14px -3px rgba(24, 34, 29, 0.10), -2px 2px 5px -1px rgba(24, 34, 29, 0.06), inset -1px 1px 0px rgba(255, 255, 255, 0.95)'
      }}
      className="backdrop-blur-xs border border-t-[#F8F6F2] border-r-[#F8F6F2] border-b-[#D4CFC5] border-l-[#D4CFC5] rounded-2xl p-4 transition-colors duration-500"
    >
      {/* Top 4-Chapter Navigation Rail (Cowan 4-Chunk Bound) */}
      <div className="flex flex-wrap items-center justify-between gap-2 border-b border-[#EFECE6] pb-3">
        <div className="flex flex-wrap items-center gap-2">
          {QUESTION_CONFIGS.map((q, idx) => {
            const isActive = q.id === currentQuestion;
            const count = selectionCounts[q.id] || 0;
            const isComplete = q.recommendedMin ? count >= q.recommendedMin : count > 0;
            const isRecommendedNext = recommendedNext === q.id;

            return (
              <button
                key={q.id}
                type="button"
                onClick={() => onSelectQuestion(q.id)}
                style={{
                  transform: isRecommendedNext && peripheralPulseActive ? 'translateY(-2px)' : 'translateY(0px)',
                  transition: 'transform 600ms cubic-bezier(0.22, 1, 0.36, 1), background-color 200ms ease, opacity 250ms ease'
                }}
                className={`relative flex items-center gap-2 px-3.5 py-2 rounded-xl text-xs font-semibold cursor-pointer ${
                  isActive
                    ? 'bg-[#3D5B4F] text-white shadow-xs ring-2 ring-[#3D5B4F]/20'
                    : isRecommendedNext && peripheralPulseActive
                    ? 'bg-[#FBF7EE] text-[#242927] border border-[#E6C875] shadow-xs'
                    : 'bg-[#F2F0EB] text-[#555C58] opacity-85 hover:opacity-100 hover:bg-[#EAE7E0] hover:text-[#242927]'
                }`}
              >
                <span className={`w-4 h-4 rounded-full text-[10px] flex items-center justify-center font-bold ${
                  isComplete
                    ? (isActive ? 'bg-emerald-300 text-[#183328]' : 'bg-[#3D5B4F] text-white')
                    : (isActive ? 'bg-white/20 text-white' : 'bg-[#E2DFD7] text-[#555C58]')
                }`}>
                  {isComplete ? '✓' : idx + 1}
                </span>
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

        {/* One-Tap Quick Routine Completion for Safety or Plan */}
        <div className="flex items-center gap-2">
          {selectionCounts.risk_factors === 0 && (
            <button
              type="button"
              onClick={() => onQuickCompleteRoutine('risk_factors')}
              className="text-[11px] font-semibold bg-[#EFF6F1] hover:bg-[#E0EFE5] text-[#2F6144] border border-[#CDE4D5] px-2.5 py-1.5 rounded-xl transition-all cursor-pointer flex items-center gap-1"
            >
              <Sparkles className="w-3 h-3" />
              <span>+ No Acute Risk</span>
            </button>
          )}
          {selectionCounts.additional_info === 0 && (
            <button
              type="button"
              onClick={() => onQuickCompleteRoutine('additional_info')}
              className="text-[11px] font-semibold bg-[#F0F5F9] hover:bg-[#E2ECF4] text-[#2E5568] border border-[#CFE0EC] px-2.5 py-1.5 rounded-xl transition-all cursor-pointer flex items-center gap-1"
            >
              <Sparkles className="w-3 h-3" />
              <span>+ Weekly Schedule</span>
            </button>
          )}
        </div>
      </div>

      {/* Active Chapter Banner & Clinical Guidance */}
      <div className="mt-3.5 flex flex-col md:flex-row md:items-center justify-between gap-2">
        <div>
          <div className="flex items-center gap-2">
            <span className="text-[11px] font-bold tracking-wider uppercase text-[#4A6B5D] bg-[#EDF3F0] px-2 py-0.5 rounded border border-[#D5E3DC]">
              Active Section: {activeConfig.label}
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
