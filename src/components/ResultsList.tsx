import React, { useState } from 'react';
import type { SearchResult, ClinicalItem } from '../types/documentation';
import { Plus, Check, ShieldAlert, Sparkles, BookOpen, ChevronLeft, ChevronRight } from 'lucide-react';

interface Props {
  results: SearchResult[];
  onSelectItem: (item: ClinicalItem) => void;
  selectedItemIds: Set<string>;
  query: string;
  viewMode: 'list' | 'sheet';
}

export const ResultsList: React.FC<Props> = ({
  results,
  onSelectItem,
  selectedItemIds,
  query,
  viewMode
}) => {
  const [sheetPage, setSheetPage] = useState(0);
  const [lastSettledId, setLastSettledId] = useState<string | null>(null);

  if (results.length === 0) {
    return (
      <div className="bg-white border border-[#E6E3DC] rounded-2xl p-8 text-center shadow-xs">
        <BookOpen className="w-8 h-8 text-[#A0A8A3] mx-auto mb-2" />
        <h3 className="text-sm font-semibold text-[#242927]">No matching statements found</h3>
        <p className="text-xs text-[#6F7773] mt-1 max-w-sm mx-auto">
          Try adjusting your search terms or clearing the modality filter to explore more clinical options.
        </p>
      </div>
    );
  }

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

  const handleCardSelect = (item: ClinicalItem) => {
    setLastSettledId(item.id);
    setTimeout(() => setLastSettledId(null), 320);
    onSelectItem(item);
  };

  const formatDragPayload = (item: ClinicalItem) => {
    const modTag = item.shortModality ? ` (${item.shortModality})` : '';
    return `${item.title}${modTag}: ${item.description}`;
  };

  // Cowan (2001) 2x2 Matrix Mode (4 cards per page)
  if (viewMode === 'sheet') {
    const totalPages = Math.max(1, Math.ceil(results.length / 4));
    const safePage = Math.min(sheetPage, totalPages - 1);
    const pageSlice = results.slice(safePage * 4, safePage * 4 + 4);

    return (
      <div className="space-y-3">
        <div className="flex items-center justify-between text-xs text-[#6F7773] px-1">
          <span>2×2 Clinical Matrix ({results.length} matching statements)</span>
          <div className="flex items-center gap-1.5">
            <button
              type="button"
              onClick={() => setSheetPage(p => (p > 0 ? p - 1 : totalPages - 1))}
              className="p-1 rounded-lg bg-white border border-[#E0DDD6] hover:bg-[#F2F0EB] cursor-pointer"
            >
              <ChevronLeft className="w-3.5 h-3.5" />
            </button>
            <span className="text-[11px] font-semibold text-[#4A6B5D] px-2">
              {safePage + 1} / {totalPages}
            </span>
            <button
              type="button"
              onClick={() => setSheetPage(p => (p + 1 < totalPages ? p + 1 : 0))}
              className="p-1 rounded-lg bg-white border border-[#E0DDD6] hover:bg-[#F2F0EB] cursor-pointer"
            >
              <ChevronRight className="w-3.5 h-3.5" />
            </button>
          </div>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
          {pageSlice.map(({ item, matchedTerms }) => {
            const isSelected = selectedItemIds.has(item.id);
            const isSettling = lastSettledId === item.id;

            return (
              <div
                key={item.id}
                draggable
                onDragStart={(e) => {
                  e.dataTransfer.setData('text/plain', formatDragPayload(item));
                  e.dataTransfer.effectAllowed = 'copy';
                }}
                onClick={() => handleCardSelect(item)}
                style={{
                  transform: isSettling ? 'scale(0.975)' : 'scale(1)',
                  transition: 'transform 220ms cubic-bezier(0.22, 1, 0.36, 1), border-color 180ms ease, box-shadow 180ms ease',
                  boxShadow: isSelected
                    ? '-5px 6px 14px rgba(24, 51, 40, 0.14), -2px 2px 5px rgba(24, 51, 40, 0.10), inset -1px 1px 0 rgba(255, 255, 255, 0.85)'
                    : '-4px 5px 12px rgba(15, 23, 34, 0.08), -1.5px 2px 4px rgba(15, 23, 34, 0.06), inset -1px 1px 0 rgba(255, 255, 255, 0.9)',
                  backgroundImage: isSelected
                    ? 'linear-gradient(225deg, rgba(255, 255, 255, 0.65) 0%, rgba(239, 245, 242, 0.96) 55%)'
                    : 'linear-gradient(225deg, rgba(255, 255, 255, 0.98) 0%, rgba(250, 249, 246, 0.94) 65%)'
                }}
                className={`group relative rounded-xl border-2 p-4 cursor-pointer text-left flex flex-col justify-between min-h-[158px] overflow-hidden ${
                  isSelected
                    ? 'bg-[#EFF5F2] border-t-[#6B9B86] border-r-[#6B9B86] border-b-[#3B594C] border-l-[#3B594C]'
                    : 'bg-white border-t-[#EAE7E0] border-r-[#EAE7E0] border-b-[#CFCAC0] border-l-[#CFCAC0] hover:border-t-[#8EB0A0] hover:border-r-[#8EB0A0]'
                }`}
              >
                <div className="space-y-1.5 min-w-0">
                  <div className="flex items-center justify-between gap-2 min-w-0">
                    <span className="bg-[#EDF3F0] text-[#345345] border border-[#D4E3DB] text-[10px] font-bold px-2 py-0.5 rounded truncate max-w-[180px]">
                      {item.shortModality || item.category}
                    </span>
                    <span className={`w-6 h-6 rounded-full flex items-center justify-center text-xs font-bold ${
                      isSelected ? 'bg-[#3D5B4F] text-white' : 'bg-[#F2F0EB] text-[#555E59]'
                    }`}>
                      {isSelected ? <Check className="w-3.5 h-3.5" /> : <Plus className="w-3.5 h-3.5" />}
                    </span>
                  </div>
                  <h4 className="text-sm font-bold text-[#242927] line-clamp-1">
                    {highlightText(item.title, matchedTerms)}
                  </h4>
                  <p className="text-xs text-[#505854] leading-relaxed line-clamp-3">
                    {highlightText(item.description, matchedTerms)}
                  </p>
                </div>
              </div>
            );
          })}
        </div>
      </div>
    );
  }

  return (
    <div className="space-y-2.5 max-h-[580px] overflow-y-auto pr-1 scrollbar-thin">
      <div className="flex items-center justify-between text-xs text-[#6F7773] px-1">
        <span>Showing {results.length} clinical matching statements</span>
        {query && <span className="text-[#87908B] text-[11px]">Ranked by hybrid relevance</span>}
      </div>

      {results.map(({ item, score, matchedTerms }) => {
        const isSelected = selectedItemIds.has(item.id);
        const isSettling = lastSettledId === item.id;

        return (
          <div
            key={item.id}
            draggable
            onDragStart={(e) => {
              e.dataTransfer.setData('text/plain', formatDragPayload(item));
              e.dataTransfer.effectAllowed = 'copy';
            }}
            onClick={() => handleCardSelect(item)}
            style={{
              transform: isSettling ? 'scale(0.98)' : 'scale(1)',
              transition: 'transform 220ms cubic-bezier(0.22, 1, 0.36, 1), border-color 180ms ease, box-shadow 180ms ease',
              boxShadow: isSelected
                ? '-5px 6px 14px rgba(24, 51, 40, 0.13), -2px 2px 5px rgba(24, 51, 40, 0.09), inset -1px 1px 0 rgba(255, 255, 255, 0.85)'
                : '-4px 5px 12px rgba(15, 23, 34, 0.08), -1.5px 2px 4px rgba(15, 23, 34, 0.05), inset -1px 1px 0 rgba(255, 255, 255, 0.9)',
              backgroundImage: isSelected
                ? 'linear-gradient(225deg, rgba(255, 255, 255, 0.65) 0%, rgba(242, 246, 244, 0.96) 55%)'
                : 'linear-gradient(225deg, rgba(255, 255, 255, 0.98) 0%, rgba(250, 249, 246, 0.94) 65%)'
            }}
            className={`group relative rounded-xl border-2 p-4 cursor-pointer text-left overflow-hidden ${
              isSelected
                ? 'bg-[#F2F6F4] border-t-[#6B9B86] border-r-[#6B9B86] border-b-[#466B5B] border-l-[#466B5B]'
                : 'bg-white border-t-[#EAE7E0] border-r-[#EAE7E0] border-b-[#CFCAC0] border-l-[#CFCAC0] hover:border-t-[#8EB0A0] hover:border-r-[#8EB0A0]'
            }`}
          >
            <div className="flex items-start justify-between gap-3">
              <div className="space-y-1.5 flex-1 min-w-0">
                <div className="flex flex-wrap items-center gap-1.5 min-w-0">
                  {item.modality && (
                    <span className="bg-[#EDF3F0] text-[#345345] border border-[#D4E3DB] text-[10px] font-bold px-2 py-0.5 rounded truncate max-w-[140px]">
                      {item.shortModality || item.modality}
                    </span>
                  )}
                  {item.category && (
                    <span className="bg-[#F4F2EC] text-[#5D6661] text-[10px] font-medium px-2 py-0.5 rounded truncate max-w-[200px]">
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

                <h4 className="text-sm font-bold text-[#242927] group-hover:text-[#325244] transition-colors">
                  {highlightText(item.title, matchedTerms)}
                </h4>

                <p className="text-xs text-[#505854] leading-relaxed">
                  {highlightText(item.description, matchedTerms)}
                </p>
              </div>

              <button
                type="button"
                onClick={(e) => {
                  e.stopPropagation();
                  handleCardSelect(item);
                }}
                className={`shrink-0 flex items-center justify-center p-2 rounded-xl text-xs font-semibold transition-all cursor-pointer ${
                  isSelected
                    ? 'bg-[#4A6B5D] text-white shadow-xs'
                    : 'bg-[#F4F2EC] text-[#555E59] hover:bg-[#4A6B5D] hover:text-white border border-[#E2DFD8]'
                }`}
                title={isSelected ? 'Remove from note' : 'Add to conjoined clinical note'}
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
