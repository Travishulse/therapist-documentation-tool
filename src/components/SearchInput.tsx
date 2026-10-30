import React from 'react';
import { Search, X, SlidersHorizontal, BrainCircuit } from 'lucide-react';
import type { ModalityClassification } from '../search/hybridEngine';

interface Props {
  query: string;
  onChangeQuery: (q: string) => void;
  placeholder: string;
  selectedModality: string;
  onSelectModality: (m: string) => void;
  availableModalities: string[];
  searchMode: 'hybrid' | 'lexical' | 'semantic';
  onChangeSearchMode: (m: 'hybrid' | 'lexical' | 'semantic') => void;
  classification: ModalityClassification | null;
  isInterventions: boolean;
}

export const SearchInput: React.FC<Props> = ({
  query,
  onChangeQuery,
  placeholder,
  selectedModality,
  onSelectModality,
  availableModalities,
  searchMode,
  onChangeSearchMode,
  classification,
  isInterventions
}) => {
  return (
    <div className="space-y-3">
      {/* Search Bar Input */}
      <div className="relative">
        <div className="absolute inset-y-0 left-0 pl-3.5 flex items-center pointer-events-none text-[#7D8882]">
          <Search className="w-4 h-4 text-[#4A6B5D]" />
        </div>
        
        <input
          type="text"
          value={query}
          onChange={(e) => onChangeQuery(e.target.value)}
          placeholder={placeholder}
          autoFocus
          className="w-full bg-white border border-[#DDD9D0] rounded-xl pl-10 pr-28 py-3 text-sm text-[#242927] placeholder-[#8F9893] focus:outline-none focus:border-[#4A6B5D] focus:ring-2 focus:ring-[#4A6B5D]/15 transition-all shadow-xs"
        />

        <div className="absolute inset-y-0 right-0 pr-2 flex items-center gap-1.5">
          {query && (
            <button
              onClick={() => onChangeQuery('')}
              className="p-1 rounded-lg hover:bg-[#F0EEE9] text-[#7B8580] hover:text-[#242927] transition-colors cursor-pointer"
              title="Clear search"
            >
              <X className="w-4 h-4" />
            </button>
          )}

          {/* Mode Switcher pill */}
          <div className="flex items-center bg-[#F2F0EB] rounded-lg p-0.5 border border-[#E2DFD8] text-[11px]">
            <button
              type="button"
              onClick={() => onChangeSearchMode('hybrid')}
              className={`px-2 py-0.5 rounded-md font-medium transition-all ${
                searchMode === 'hybrid'
                  ? 'bg-[#4A6B5D] text-white shadow-xs'
                  : 'text-[#616B66] hover:text-[#242927]'
              }`}
              title="Hybrid: Lexical BM25 + Vector Semantic Reranking"
            >
              Hybrid
            </button>
            <button
              type="button"
              onClick={() => onChangeSearchMode('lexical')}
              className={`px-2 py-0.5 rounded-md font-medium transition-all ${
                searchMode === 'lexical'
                  ? 'bg-[#4A6B5D] text-white shadow-xs'
                  : 'text-[#616B66] hover:text-[#242927]'
              }`}
              title="Lexical: Fast Exact & Fuzzy BM25 Inverted Index"
            >
              BM25
            </button>
            <button
              type="button"
              onClick={() => onChangeSearchMode('semantic')}
              className={`px-2 py-0.5 rounded-md font-medium transition-all ${
                searchMode === 'semantic'
                  ? 'bg-[#4A6B5D] text-white shadow-xs'
                  : 'text-[#616B66] hover:text-[#242927]'
              }`}
              title="Semantic: Cosine Vector Projections"
            >
              Vector
            </button>
          </div>
        </div>
      </div>

      {/* Auto-detected Modality Cue Banner */}
      {classification && classification.confidence >= 0.5 && (
        <div className="flex items-center justify-between bg-[#EDF4F0] border border-[#D0E2D8] rounded-xl px-3.5 py-2 text-xs text-[#305244] animate-fadeIn">
          <div className="flex items-center gap-2">
            <BrainCircuit className="w-4 h-4 text-[#4A6B5D] shrink-0" />
            <span>
              Intent Classifier detected: <strong className="text-[#20382E] font-semibold">{classification.topModality}</strong>
              <span className="text-[#516E60] ml-1">
                ({Math.round(classification.confidence * 100)}% match on "{classification.detectedCues.join(', ')}")
              </span>
            </span>
          </div>
          {selectedModality !== classification.topModality && (
            <button
              onClick={() => onSelectModality(classification.topModality)}
              className="text-[11px] font-semibold text-[#2C4A3D] hover:text-white bg-[#D6E6DE] hover:bg-[#4A6B5D] px-2.5 py-0.5 rounded-md transition-colors cursor-pointer"
            >
              Filter to this modality
            </button>
          )}
        </div>
      )}

      {/* Modality Filter Pills (Visible for Interventions) */}
      {isInterventions && (
        <div className="flex items-center gap-1.5 overflow-x-auto pb-1 scrollbar-thin">
          <span className="text-[11px] uppercase tracking-wider text-[#79837E] font-semibold flex items-center gap-1 mr-1 shrink-0">
            <SlidersHorizontal className="w-3 h-3" /> Modality:
          </span>

          <button
            onClick={() => onSelectModality('all')}
            className={`px-2.5 py-1 rounded-lg text-xs font-medium whitespace-nowrap transition-colors cursor-pointer shrink-0 ${
              selectedModality === 'all'
                ? 'bg-[#4A6B5D] text-white shadow-xs'
                : 'bg-[#F2F0EB] text-[#555E59] hover:bg-[#E7E4DC] hover:text-[#242927]'
            }`}
          >
            All Modalities (615)
          </button>

          {availableModalities.map((mod) => {
            const isSelected = selectedModality === mod;
            return (
              <button
                key={mod}
                onClick={() => onSelectModality(mod)}
                className={`px-2.5 py-1 rounded-lg text-xs font-medium whitespace-nowrap transition-colors cursor-pointer shrink-0 ${
                  isSelected
                    ? 'bg-[#4A6B5D] text-white shadow-xs'
                    : 'bg-[#F2F0EB] text-[#555E59] hover:bg-[#E7E4DC] hover:text-[#242927]'
                }`}
              >
                {mod}
              </button>
            );
          })}
        </div>
      )}
    </div>
  );
};
