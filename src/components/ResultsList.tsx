import React from 'react';
import type { SearchResult, ClinicalItem } from '../types/documentation';
import { Plus, Check, ShieldAlert, Sparkles, BookOpen } from 'lucide-react';

interface Props {
  results: SearchResult[];
  onSelectItem: (item: ClinicalItem) => void;
  selectedItemIds: Set<string>;
  query: string;
}

export const ResultsList: React.FC<Props> = ({
  results,
  onSelectItem,
  selectedItemIds,
  query
}) => {
  if (results.length === 0) {
    return (
      <div className="bg-white border border-[#E6E3DC] rounded-2xl p-8 text-center shadow-xs">
        <BookOpen className="w-8 h-8 text-[#A0A8A3] mx-auto mb-2" />
        <h3 className="text-sm font-semibold text-[#242927]">No matching answers found</h3>
        <p className="text-xs text-[#6F7773] mt-1 max-w-sm mx-auto">
          Try adjusting your search terms or clearing the modality filter to explore more clinical options.
        </p>
      </div>
    );
  }

  // Highlight matched terms in text
  const highlightText = (text: string, terms: string[]) => {
    if (!query.trim() || terms.length === 0) return text;
    const regex = new RegExp(`(${terms.map(t => t.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')).join('|')})`, 'gi');
    const parts = text.split(regex);

    return parts.map((part, i) =>
      regex.test(part) ? (
        <mark key={i} className="bg-[#E4ECE7] text-[#244234] px-0.5 rounded font-medium">
          {part}
        </mark>
      ) : (
        part
      )
    );
  };

  const getSeverityBadge = (severity?: string) => {
    switch (severity) {
      case 'elevated':
        return <span className="bg-[#FDF2F2] text-[#9E3634] border border-[#F5CACA] text-[10px] font-bold px-2 py-0.5 rounded-full flex items-center gap-1"><ShieldAlert className="w-3 h-3 text-[#A83D3B]" /> Elevated Risk</span>;
      case 'moderate':
        return <span className="bg-[#FFF8ED] text-[#8C5E24] border border-[#F7E3C6] text-[10px] font-bold px-2 py-0.5 rounded-full">Moderate Risk</span>;
      case 'mild':
        return <span className="bg-[#F0F6F9] text-[#3E657A] border border-[#D5E5EE] text-[10px] font-bold px-2 py-0.5 rounded-full">Monitored</span>;
      case 'none':
        return <span className="bg-[#EFF6F1] text-[#2F6144] border border-[#D1E6D8] text-[10px] font-bold px-2 py-0.5 rounded-full">No Acute Risk</span>;
      default:
        return null;
    }
  };

  return (
    <div className="space-y-2.5 max-h-[580px] overflow-y-auto pr-1 scrollbar-thin">
      <div className="flex items-center justify-between text-xs text-[#6F7773] px-1">
        <span>Showing {results.length} clinical matching responses</span>
        {query && <span className="text-[#87908B] text-[11px]">Ranked by relevance</span>}
      </div>

      {results.map(({ item, score, matchedTerms }) => {
        const isSelected = selectedItemIds.has(item.id);

        return (
          <div
            key={item.id}
            onClick={() => onSelectItem(item)}
            className={`group relative rounded-xl border p-4 transition-all duration-150 cursor-pointer text-left ${
              isSelected
                ? 'bg-[#F2F6F4] border-[#6D9682] ring-1 ring-[#6D9682]/30 shadow-xs'
                : 'bg-white border-[#E6E3DC] hover:border-[#D0CCC3] hover:bg-[#FAF9F6] shadow-xs'
            }`}
          >
            <div className="flex items-start justify-between gap-3">
              <div className="space-y-1.5 flex-1 min-w-0">
                {/* Modality / Category metadata */}
                <div className="flex flex-wrap items-center gap-1.5">
                  {item.modality && (
                    <span className="bg-[#EDF3F0] text-[#345345] border border-[#D4E3DB] text-[10px] font-bold px-2 py-0.5 rounded">
                      {item.shortModality || item.modality}
                    </span>
                  )}
                  {item.category && (
                    <span className="bg-[#F4F2EC] text-[#5D6661] text-[10px] font-medium px-2 py-0.5 rounded">
                      {item.category}
                    </span>
                  )}
                  {getSeverityBadge(item.severity)}
                  {score > 0.8 && query && (
                    <span className="text-[10px] text-[#3D6B54] flex items-center gap-0.5 font-medium ml-auto">
                      <Sparkles className="w-3 h-3" /> Top match
                    </span>
                  )}
                </div>

                {/* Title */}
                <h4 className="text-sm font-bold text-[#242927] group-hover:text-[#325244] transition-colors">
                  {highlightText(item.title, matchedTerms)}
                </h4>

                {/* Description */}
                <p className="text-xs text-[#505854] leading-relaxed">
                  {highlightText(item.description, matchedTerms)}
                </p>
              </div>

              {/* Add / Added Button */}
              <button
                type="button"
                onClick={(e) => {
                  e.stopPropagation();
                  onSelectItem(item);
                }}
                className={`shrink-0 flex items-center justify-center p-2 rounded-xl text-xs font-semibold transition-all cursor-pointer ${
                  isSelected
                    ? 'bg-[#4A6B5D] text-white shadow-xs'
                    : 'bg-[#F4F2EC] text-[#555E59] hover:bg-[#4A6B5D] hover:text-white border border-[#E2DFD8]'
                }`}
                title={isSelected ? 'Remove from note' : 'Click to add to conjoined output box'}
              >
                {isSelected ? (
                  <Check className="w-4 h-4" />
                ) : (
                  <Plus className="w-4 h-4" />
                )}
              </button>
            </div>
          </div>
        );
      })}
    </div>
  );
};
