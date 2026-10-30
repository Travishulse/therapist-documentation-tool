import React, { useState } from 'react';
import type { ClinicalItem, DocumentationNote } from '../types/documentation';
import { QUESTION_CONFIGS } from './QuestionSelector';
import { Check, X, RotateCcw, Sparkles } from 'lucide-react';

interface Props {
  note: DocumentationNote;
  onRemoveItem: (item: ClinicalItem) => void;
  onClearAll: () => void;
  rawText: string;
  onChangeRawText: (text: string) => void;
  onRegenerateText: () => void;
}

export const ConjoinedOutputBox: React.FC<Props> = ({
  note,
  onRemoveItem,
  onClearAll,
  rawText,
  onChangeRawText,
  onRegenerateText
}) => {
  const [copied, setCopied] = useState(false);

  const totalItemsCount =
    note.session_focus.length +
    note.interventions.length +
    note.risk_factors.length +
    note.additional_info.length;

  const handleCopy = async () => {
    let success = false;
    try {
      if (navigator.clipboard && navigator.clipboard.writeText) {
        await navigator.clipboard.writeText(rawText);
        success = true;
      }
    } catch {
      // Fallback
    }

    if (!success) {
      try {
        const textarea = document.createElement('textarea');
        textarea.value = rawText;
        textarea.style.position = 'fixed';
        textarea.style.opacity = '0';
        document.body.appendChild(textarea);
        textarea.focus();
        textarea.select();
        document.execCommand('copy');
        document.body.removeChild(textarea);
        success = true;
      } catch (err) {
        console.error('Copy fallback failed:', err);
      }
    }

    if (success) {
      setCopied(true);
      setTimeout(() => setCopied(false), 2200);
    }
  };

  const wordCount = rawText.trim() ? rawText.trim().split(/\s+/).length : 0;
  const charCount = rawText.length;

  return (
    <div className="bg-white border border-[#E6E3DC] rounded-2xl p-5 flex flex-col h-full shadow-xs relative">
      {/* Header with Title and Copy to Clipboard Emoji Button */}
      <div className="flex items-center justify-between pb-4 border-b border-[#EFECE6]">
        <div>
          <h3 className="text-base font-bold text-[#242927] flex items-center gap-2">
            Clinical Documentation Note
            <span className="text-xs font-normal text-[#6F7773]">
              ({totalItemsCount} items conjoined)
            </span>
          </h3>
          <p className="text-xs text-[#6F7773] mt-0.5">
            Auto-conjoined responses formatted for your clinical record.
          </p>
        </div>

        {/* Standard Clipboard Emoji Button on top right corner */}
        <div className="flex items-center gap-2">
          <button
            type="button"
            onClick={handleCopy}
            className={`flex items-center gap-2 px-3.5 py-1.5 rounded-xl font-bold text-xs transition-all duration-200 cursor-pointer shadow-xs ${
              copied
                ? 'bg-[#386B52] text-white ring-2 ring-[#386B52]/20'
                : 'bg-[#4A6B5D] hover:bg-[#3D5B4F] text-white active:scale-95'
            }`}
            title="Copy full conjoined response to clipboard"
          >
            <span className="text-base" role="img" aria-label="clipboard">
              📋
            </span>
            <span>{copied ? 'Copied!' : 'Copy to Clipboard'}</span>
            {copied && <Check className="w-3.5 h-3.5 text-white" />}
          </button>
        </div>
      </div>

      {/* Selected Items Tag Chips by Question */}
      <div className="py-3 border-b border-[#EFECE6] space-y-2 max-h-48 overflow-y-auto pr-1 scrollbar-thin">
        {QUESTION_CONFIGS.map((q) => {
          const items = note[q.id];
          if (items.length === 0) return null;

          return (
            <div key={q.id} className="space-y-1">
              <div className="flex items-center justify-between text-[11px] font-semibold text-[#666F6A] uppercase tracking-wider">
                <span>{q.label} ({items.length})</span>
                {q.id === 'interventions' && (
                  <span className={`text-[10px] ${items.length >= 3 && items.length <= 5 ? 'text-[#3D6B54]' : 'text-[#8C5E24]'}`}>
                    Target: 3-5 ({items.length} chosen)
                  </span>
                )}
              </div>

              <div className="flex flex-wrap gap-1.5">
                {items.map((item) => (
                  <span
                    key={item.id}
                    className="inline-flex items-center gap-1.5 bg-[#F4F2EC] text-[#2F3633] border border-[#DFDBD2] text-xs px-2.5 py-1 rounded-lg group hover:border-[#CFCCC2]"
                  >
                    {item.shortModality && (
                      <span className="text-[10px] font-bold text-[#4A6B5D] uppercase">
                        {item.shortModality}:
                      </span>
                    )}
                    <span className="truncate max-w-[200px]" title={item.title}>
                      {item.title}
                    </span>
                    <button
                      type="button"
                      onClick={() => onRemoveItem(item)}
                      className="text-[#7F8984] hover:text-[#B34543] p-0.5 rounded transition-colors cursor-pointer"
                      title="Remove from note"
                    >
                      <X className="w-3 h-3" />
                    </button>
                  </span>
                ))}
              </div>
            </div>
          );
        })}

        {totalItemsCount === 0 && (
          <div className="text-center py-4 text-xs text-[#87908B]">
            Click any answer from the search results on the left to populate this conjoined output box.
          </div>
        )}
      </div>

      {/* Conjoined Editable Output Area */}
      <div className="flex-1 mt-3 flex flex-col min-h-[320px]">
        <div className="flex items-center justify-between mb-1.5">
          <label className="text-xs font-semibold text-[#666F6A] uppercase tracking-wider">
            Conjoined Clinical Narrative
          </label>
          <div className="flex items-center gap-3">
            <button
              type="button"
              onClick={onRegenerateText}
              className="text-[11px] text-[#4A6B5D] hover:text-[#38564A] flex items-center gap-1 cursor-pointer font-medium"
              title="Re-conjoin selected items into standard format"
            >
              <RotateCcw className="w-3 h-3" />
              Re-conjoin
            </button>
            <button
              type="button"
              onClick={onClearAll}
              className="text-[11px] text-[#A84A48] hover:text-[#8E3B3A] cursor-pointer font-medium"
              title="Clear all selected items"
            >
              Clear All
            </button>
          </div>
        </div>

        <textarea
          value={rawText}
          onChange={(e) => onChangeRawText(e.target.value)}
          placeholder="Selected answers will automatically conjoin here into a complete clinical note. You can also edit this text directly..."
          className="w-full flex-1 min-h-[280px] bg-[#FAF9F6] border border-[#DDD9D0] rounded-xl p-4 text-xs md:text-sm text-[#242927] leading-relaxed font-sans focus:outline-none focus:border-[#4A6B5D] focus:ring-1 focus:ring-[#4A6B5D]/20 resize-none font-mono selection:bg-[#D4E3DB]"
        />

        {/* Footer Metrics */}
        <div className="mt-2.5 flex items-center justify-between text-xs text-[#7A837E] px-1">
          <div className="flex items-center gap-3">
            <span>{wordCount} words</span>
            <span>•</span>
            <span>{charCount} characters</span>
          </div>

          <div className="text-[11px] text-[#55615B] flex items-center gap-1">
            <Sparkles className="w-3.5 h-3.5 text-[#4A6B5D]" />
            <span>Ready for EHR paste</span>
          </div>
        </div>
      </div>
    </div>
  );
};
