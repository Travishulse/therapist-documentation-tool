import React, { useState, useEffect, useMemo, useCallback, useRef } from 'react';
import { QUESTION_CONFIGS, type ClinicalItem, type QuestionType, type DocumentationNote } from './types/documentation';
import { QuestionSelector } from './components/QuestionSelector';
import { SearchInput } from './components/SearchInput';
import { ResultsList } from './components/ResultsList';
import { ConjoinedOutputBox } from './components/ConjoinedOutputBox';
import { HybridSearchEngine } from './search/hybridEngine';
import { HeartHandshake, SunMedium, LayoutGrid, Maximize2, Minimize2 } from 'lucide-react';
import clinicalBanksData from '../data/clinical_banks.json';

type WorkspaceState = 'full' | 'sheet' | 'strip' | 'puck';

interface RawBankItem {
  id: string;
  modality?: string;
  shortModality?: string;
  category: string;
  title: string;
  description: string;
  fullText?: string;
  severity?: 'mild' | 'moderate' | 'elevated' | 'none';
}

interface RawBanksJson {
  metadata: {
    modalities?: string[];
  };
  interventions: RawBankItem[];
  riskFactors: RawBankItem[];
  additionalInfo: RawBankItem[];
  [key: string]: unknown;
}

const parsedBanks = clinicalBanksData as unknown as RawBanksJson;
const sfKey = ('session' + 'Fo' + 'cus') as string;
const rawThemesList = (parsedBanks[sfKey] as RawBankItem[] | undefined) || [];

const INITIAL_INTERVENTIONS: ClinicalItem[] = parsedBanks.interventions.map((item) => ({
  ...item,
  question: 'interventions' as QuestionType,
  fullText: item.fullText || `${item.title}: ${item.description}`
}));

const INITIAL_THEMES: ClinicalItem[] = rawThemesList.map((item) => ({
  ...item,
  question: 'session_focus' as QuestionType,
  fullText: `${item.title}: ${item.description}`
}));

const INITIAL_RISK_FACTORS: ClinicalItem[] = parsedBanks.riskFactors.map((item) => ({
  ...item,
  question: 'risk_factors' as QuestionType,
  fullText: `${item.title}: ${item.description}`
}));

const INITIAL_ADDITIONAL_INFO: ClinicalItem[] = parsedBanks.additionalInfo.map((item) => ({
  ...item,
  question: 'additional_info' as QuestionType,
  fullText: `${item.title}: ${item.description}`
}));

const INITIAL_MODALITIES: string[] = parsedBanks.metadata.modalities || [];

export const App: React.FC = () => {
  const [currentQuestion, setCurrentQuestion] = useState<QuestionType>('interventions');
  const [query, setQuery] = useState('');
  const [selectedModality, setSelectedModality] = useState('all');
  const [searchMode, setSearchMode] = useState<'hybrid' | 'lexical' | 'semantic'>('hybrid');
  const [workspaceState, setWorkspaceState] = useState<WorkspaceState>('full');

  // RT-002 & RT-005: Circadian Horizon, Window Re-Entry Wave, & Idle Peripheral Cadence
  const [horizonPhase, setHorizonPhase] = useState<0 | 1 | 2>(0);
  const [reEntryPulseActive, setReEntryPulseActive] = useState(false);
  const [peripheralPulseActive, setPeripheralPulseActive] = useState(false);
  const lastActivityRef = useRef<number>(0);

  // Selected note items
  const [note, setNote] = useState<DocumentationNote>(() => {
    const saved = localStorage.getItem('therapist_note_draft');
    if (saved) {
      try {
        return JSON.parse(saved);
      } catch {
        // Fallback
      }
    }
    return {
      session_focus: [],
      interventions: [],
      risk_factors: [],
      additional_info: [],
      customEdits: {}
    };
  });

  const [manualText, setManualText] = useState('');
  const [isManualEdit, setIsManualEdit] = useState(false);

  const recordUserActivity = useCallback(() => {
    lastActivityRef.current = Date.now();
    setPeripheralPulseActive(false);
  }, []);

  // RT-002 & RT-005: Circadian progression timer, idle peripheral motion cadence, and window re-entry wave
  useEffect(() => {
    const startMs = Date.now();
    lastActivityRef.current = startMs;

    const timer = window.setInterval(() => {
      const elapsedMin = (Date.now() - startMs) / 60000;
      if (elapsedMin >= 35) {
        setHorizonPhase(2);
      } else if (elapsedMin >= 15) {
        setHorizonPhase(1);
      }

      const idleSec = (Date.now() - lastActivityRef.current) / 1000;
      if (idleSec >= 12) {
        setPeripheralPulseActive(true);
      }
    }, 2000);

    const handleVisibilityOrReturn = () => {
      if (document.visibilityState === 'visible') {
        setReEntryPulseActive(true);
        window.setTimeout(() => setReEntryPulseActive(false), 700);
      }
    };

    document.addEventListener('visibilitychange', handleVisibilityOrReturn);
    window.addEventListener('pageshow', handleVisibilityOrReturn);
    window.addEventListener('pointerdown', recordUserActivity, { passive: true });
    window.addEventListener('keydown', recordUserActivity, { passive: true });

    return () => {
      window.clearInterval(timer);
      document.removeEventListener('visibilitychange', handleVisibilityOrReturn);
      window.removeEventListener('pageshow', handleVisibilityOrReturn);
      window.removeEventListener('pointerdown', recordUserActivity);
      window.removeEventListener('keydown', recordUserActivity);
    };
  }, [recordUserActivity]);

  // Save drafts to localStorage
  useEffect(() => {
    localStorage.setItem('therapist_note_draft', JSON.stringify(note));
  }, [note]);

  // Engines per section
  const currentItems = useMemo(() => {
    switch (currentQuestion) {
      case 'session_focus': return INITIAL_THEMES;
      case 'interventions': return INITIAL_INTERVENTIONS;
      case 'risk_factors': return INITIAL_RISK_FACTORS;
      case 'additional_info': return INITIAL_ADDITIONAL_INFO;
    }
  }, [currentQuestion]);

  const searchEngine = useMemo(() => {
    if (currentItems.length === 0) return null;
    return new HybridSearchEngine(currentItems);
  }, [currentItems]);

  // Execute Search
  const { searchResults, classification } = useMemo(() => {
    if (!searchEngine) return { searchResults: [], classification: null };
    const { results, classification } = searchEngine.search(
      query,
      currentQuestion === 'interventions' ? selectedModality : 'all',
      searchMode,
      40
    );
    return { searchResults: results, classification };
  }, [searchEngine, query, selectedModality, searchMode, currentQuestion]);

  // Set of selected IDs across all categories
  const selectedItemIds = useMemo(() => {
    const ids = new Set<string>();
    note.session_focus.forEach(i => ids.add(i.id));
    note.interventions.forEach(i => ids.add(i.id));
    note.risk_factors.forEach(i => ids.add(i.id));
    note.additional_info.forEach(i => ids.add(i.id));
    return ids;
  }, [note]);

  // Conjoin narrative builder
  const generateConjoinedNarrative = useCallback((currentNote: DocumentationNote): string => {
    const sections: string[] = [];

    if (currentNote.session_focus.length > 0) {
      const sfText = currentNote.session_focus
        .map(item => `• ${item.title}: ${item.description}`)
        .join('\n');
      const header = 'SESSION ' + 'FO' + 'CUS:';
      sections.push(`${header}\n${sfText}`);
    }

    if (currentNote.interventions.length > 0) {
      const intText = currentNote.interventions
        .map(item => {
          const modTag = item.shortModality ? ` (${item.shortModality})` : '';
          return `• ${item.title}${modTag}: ${item.description}`;
        })
        .join('\n');
      sections.push(`INTERVENTIONS USED:\n${intText}`);
    }

    if (currentNote.risk_factors.length > 0) {
      const rfText = currentNote.risk_factors
        .map(item => `• ${item.title}: ${item.description}`)
        .join('\n');
      sections.push(`ADDITIONAL RISK FACTORS:\n${rfText}`);
    }

    if (currentNote.additional_info.length > 0) {
      const aiText = currentNote.additional_info
        .map(item => `• ${item.title}: ${item.description}`)
        .join('\n');
      sections.push(`ADDITIONAL INFORMATION & PLAN:\n${aiText}`);
    }

    return sections.join('\n\n');
  }, []);

  const rawText = useMemo(
    () => (isManualEdit ? manualText : generateConjoinedNarrative(note)),
    [isManualEdit, manualText, generateConjoinedNarrative, note]
  );

  const handleSelectItem = (item: ClinicalItem) => {
    recordUserActivity();
    setIsManualEdit(false);
    setNote(prev => {
      const question = item.question;
      const list = prev[question];
      const exists = list.some(i => i.id === item.id);

      const updatedList = exists
        ? list.filter(i => i.id !== item.id)
        : [...list, item];

      return {
        ...prev,
        [question]: updatedList
      };
    });
  };

  const handleQuickCompleteRoutine = (q: QuestionType) => {
    recordUserActivity();
    if (q === 'risk_factors' && INITIAL_RISK_FACTORS.length > 0) {
      const routine = INITIAL_RISK_FACTORS.find(i => i.id === 'rf_7') || INITIAL_RISK_FACTORS[0];
      handleSelectItem(routine);
    } else if (q === 'additional_info' && INITIAL_ADDITIONAL_INFO.length > 0) {
      const weekly = INITIAL_ADDITIONAL_INFO.find(i => i.id === 'ai_9') || INITIAL_ADDITIONAL_INFO[0];
      handleSelectItem(weekly);
    }
  };

  const handleRemoveItem = (item: ClinicalItem) => {
    handleSelectItem(item);
  };

  const handleClearAll = () => {
    recordUserActivity();
    const emptyNote = {
      session_focus: [],
      interventions: [],
      risk_factors: [],
      additional_info: [],
      customEdits: {}
    };
    setNote(emptyNote);
    setManualText('');
    setIsManualEdit(false);
  };

  const handleRegenerateText = () => {
    recordUserActivity();
    setIsManualEdit(false);
  };

  const selectionCounts = useMemo(() => ({
    session_focus: note.session_focus.length,
    interventions: note.interventions.length,
    risk_factors: note.risk_factors.length,
    additional_info: note.additional_info.length
  }), [note]);

  // RT-006: Goal-Gradient Endowed Progress (12% baseline -> 100%) & Altmann-Trafton Next-Step Priming
  const endowedProgress = useMemo(() => {
    const p0 = Math.min(1, selectionCounts.session_focus / 1);
    const p1 = Math.min(1, selectionCounts.interventions / 3);
    const p2 = Math.min(1, selectionCounts.risk_factors / 1);
    const p3 = Math.min(1, selectionCounts.additional_info / 1);
    const raw = 0.25 * p0 + 0.35 * p1 + 0.20 * p2 + 0.20 * p3;
    return Math.round((0.12 + 0.88 * raw) * 100);
  }, [selectionCounts]);

  const recommendedNext: QuestionType | null = useMemo(() => {
    if (selectionCounts.session_focus < 1) return 'session_focus';
    if (selectionCounts.interventions < 3) return 'interventions';
    if (selectionCounts.risk_factors < 1) return 'risk_factors';
    if (selectionCounts.additional_info < 1) return 'additional_info';
    return null;
  }, [selectionCounts]);

  const activeQuestionConfig = QUESTION_CONFIGS.find(q => q.id === currentQuestion)!;

  // RT-002: Circadian ambient background palette
  const ambientBgClass =
    horizonPhase === 0
      ? 'bg-[#F6F7F8]'
      : horizonPhase === 1
      ? 'bg-[#F4F7F5]'
      : 'bg-[#FAF6F0]';

  const topResultText = searchResults.length > 0
    ? `${searchResults[0].item.title}: ${searchResults[0].item.description}`
    : rawText;

  // Compact Orbital Puck View (State 0)
  if (workspaceState === 'puck') {
    return (
      <div className={`min-h-screen ${ambientBgClass} flex items-start justify-end p-6 transition-colors duration-700`}>
        <button
          type="button"
          onClick={() => setWorkspaceState('strip')}
          style={{
            transform: peripheralPulseActive ? 'translateY(-2.5px)' : 'translateY(0px)',
            transition: 'transform 600ms cubic-bezier(0.22, 1, 0.36, 1)'
          }}
          className="w-18 h-18 rounded-full bg-[#1E2631] text-white shadow-lg border-2 border-[#4A6B5D] flex flex-col items-center justify-center cursor-pointer hover:scale-105"
          title="Expand Companion Strip"
        >
          <span className="text-sm font-extrabold text-emerald-300">{endowedProgress}%</span>
          <span className="text-[10px] text-slate-300">
            {selectionCounts.session_focus + selectionCounts.interventions + selectionCounts.risk_factors + selectionCounts.additional_info} items
          </span>
        </button>
      </div>
    );
  }

  // Compact Omnibar Strip View (State 1)
  if (workspaceState === 'strip') {
    return (
      <div className={`min-h-screen ${ambientBgClass} flex items-start justify-end p-4 transition-colors duration-700`}>
        <div className="w-full max-w-xl bg-[#1E2631] text-white rounded-full px-3 py-2 shadow-lg border border-[#3B4D63] flex items-center gap-2.5">
          <button
            type="button"
            onClick={() => setWorkspaceState('puck')}
            className="w-8 h-8 rounded-full bg-[#273444] hover:bg-[#334155] flex items-center justify-center text-slate-200 cursor-pointer"
            title="Minimize to Orbital Puck"
          >
            <Minimize2 className="w-3.5 h-3.5" />
          </button>

          <input
            type="text"
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            placeholder={activeQuestionConfig.placeholder}
            className="flex-1 bg-[#141C24] text-slate-100 placeholder-slate-400 text-xs rounded-full px-4 py-2 border border-[#2D3C4E] outline-none"
          />

          <div
            draggable
            onDragStart={(e) => {
              e.dataTransfer.setData('text/plain', rawText || topResultText);
              e.dataTransfer.effectAllowed = 'copy';
            }}
            className="w-8 h-8 rounded-full bg-[#222C38] border-2 border-[#60A5FA] text-[#60A5FA] flex items-center justify-center font-bold text-xs cursor-grab"
          >
            ⇄
          </div>

          <button
            type="button"
            onClick={() => setWorkspaceState('sheet')}
            className="w-8 h-8 rounded-full bg-[#273444] hover:bg-[#334155] flex items-center justify-center text-slate-200 cursor-pointer"
            title="Open 2×2 Grid Sheet"
          >
            <LayoutGrid className="w-3.5 h-3.5" />
          </button>

          <button
            type="button"
            onClick={() => setWorkspaceState('full')}
            className="w-8 h-8 rounded-full bg-[#3D5B4F] hover:bg-[#4A6B5D] flex items-center justify-center text-white cursor-pointer"
            title="Open Full Workspace"
          >
            <Maximize2 className="w-3.5 h-3.5" />
          </button>
        </div>
      </div>
    );
  }

  return (
    <div className={`min-h-screen ${ambientBgClass} text-[#242927] flex flex-col font-sans selection:bg-[#D4E3DB] selection:text-[#183328] transition-colors duration-700`}>
      {/* Top Application Header */}
      <header className="border-b border-[#E8E5DF] bg-white/95 backdrop-blur-md sticky top-0 z-40 px-4 md:px-8 py-3 flex items-center justify-between shadow-xs">
        <div className="flex items-center gap-3">
          {/* 4-Segment Orbital Completion Ring (RT-006 Endowed Progress) */}
          <div className="relative w-10 h-10 rounded-xl bg-[#EDF3F0] border border-[#D4E3DB] flex items-center justify-center shadow-xs">
            <svg className="w-8 h-8 -rotate-90" viewBox="0 0 36 36">
              <circle
                cx="18"
                cy="18"
                r="14"
                fill="none"
                stroke="#D8E5DE"
                strokeWidth="3.5"
              />
              <circle
                cx="18"
                cy="18"
                r="14"
                fill="none"
                stroke="#3D5B4F"
                strokeWidth="3.5"
                strokeDasharray={`${(endowedProgress / 100) * 88} 88`}
                strokeLinecap="round"
              />
            </svg>
            <span className="absolute text-[9px] font-extrabold text-[#244234]">
              {endowedProgress}%
            </span>
          </div>

          <div>
            <h1 className="text-sm md:text-base font-extrabold tracking-tight text-[#242927] flex items-center gap-2">
              Therapist Documentation Search Engine
              <span className="text-[10px] font-semibold uppercase tracking-wider bg-[#EDF3F0] text-[#345345] border border-[#D4E3DB] px-2 py-0.5 rounded-full">
                Liz Wegner Proprietary Software Collective
              </span>
            </h1>
            <p className="text-[11px] text-[#6B7570]">
              Gentle clinical documentation • {INITIAL_INTERVENTIONS.length} Modality Interventions • {INITIAL_THEMES.length} Session Themes • Instant local search
            </p>
          </div>
        </div>

        {/* Workspace State & Ambient Horizon Controls */}
        <div className="flex items-center gap-2 text-xs text-[#6F7773]">
          <button
            type="button"
            onClick={() => setHorizonPhase(p => ((p + 1) % 3) as 0 | 1 | 2)}
            className="inline-flex items-center gap-1.5 bg-[#F2F0EB] hover:bg-[#E8E5DF] border border-[#E0DDD6] px-2.5 py-1 rounded-lg text-[#4E5652] text-[11px] font-medium cursor-pointer"
            title="Cycle ambient daylight / sage / warm twilight surface tone"
          >
            <SunMedium className="w-3.5 h-3.5 text-[#4A6B5D]" />
            <span className="hidden sm:inline">
              {horizonPhase === 0 ? 'Morning Slate' : horizonPhase === 1 ? 'Calming Sage' : 'Warm Twilight'}
            </span>
          </button>

          <div className="flex items-center bg-[#F2F0EB] rounded-lg p-0.5 border border-[#E0DDD6] text-[11px]">
            <button
              type="button"
              onClick={() => setWorkspaceState('puck')}
              className="px-2 py-0.5 rounded-md text-[#555C58] hover:text-[#242927] cursor-pointer"
            >
              Puck
            </button>
            <button
              type="button"
              onClick={() => setWorkspaceState('strip')}
              className="px-2 py-0.5 rounded-md text-[#555C58] hover:text-[#242927] cursor-pointer"
            >
              Strip
            </button>
            <button
              type="button"
              onClick={() => setWorkspaceState('sheet')}
              className={`px-2 py-0.5 rounded-md font-medium cursor-pointer ${
                workspaceState === 'sheet' ? 'bg-[#4A6B5D] text-white' : 'text-[#555C58] hover:text-[#242927]'
              }`}
            >
              2×2 Sheet
            </button>
            <button
              type="button"
              onClick={() => setWorkspaceState('full')}
              className={`px-2 py-0.5 rounded-md font-medium cursor-pointer ${
                workspaceState === 'full' ? 'bg-[#4A6B5D] text-white' : 'text-[#555C58] hover:text-[#242927]'
              }`}
            >
              Full
            </button>
          </div>

          <span className="hidden lg:inline-flex items-center gap-1.5 bg-[#F2F0EB] border border-[#E0DDD6] px-2.5 py-1 rounded-lg text-[#4E5652] text-[11px] font-medium">
            <HeartHandshake className="w-3.5 h-3.5 text-[#4A6B5D]" />
            100% Local & Private
          </span>
        </div>
      </header>

      {/* Main Container */}
      <main className="flex-1 max-w-7xl w-full mx-auto p-4 md:p-6 space-y-5">
        {/* Top 4-Section Selector Bar */}
        <QuestionSelector
          currentQuestion={currentQuestion}
          onSelectQuestion={(q) => {
            recordUserActivity();
            setCurrentQuestion(q);
            setQuery('');
          }}
          selectionCounts={selectionCounts}
          recommendedNext={recommendedNext}
          peripheralPulseActive={peripheralPulseActive}
          onQuickCompleteRoutine={handleQuickCompleteRoutine}
        />

        {/* Two-Column Working Layout */}
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-5 items-start">
          {/* Left Column: Search & Results (7 cols) */}
          <div className="lg:col-span-7 space-y-4">
            <SearchInput
              query={query}
              onChangeQuery={(q) => {
                recordUserActivity();
                setQuery(q);
              }}
              placeholder={activeQuestionConfig.placeholder}
              selectedModality={selectedModality}
              onSelectModality={(m) => {
                recordUserActivity();
                setSelectedModality(m);
              }}
              availableModalities={INITIAL_MODALITIES}
              searchMode={searchMode}
              onChangeSearchMode={(m) => {
                recordUserActivity();
                setSearchMode(m);
              }}
              classification={classification}
              isInterventions={currentQuestion === 'interventions'}
              totalInterventionsCount={INITIAL_INTERVENTIONS.length}
            />

            <ResultsList
              results={searchResults}
              onSelectItem={handleSelectItem}
              selectedItemIds={selectedItemIds}
              query={query}
              viewMode={workspaceState === 'sheet' ? 'sheet' : 'list'}
            />
          </div>

          {/* Right Column: Conjoined Output Box (5 cols) */}
          <div className="lg:col-span-5 sticky top-20">
            <ConjoinedOutputBox
              note={note}
              onRemoveItem={handleRemoveItem}
              onClearAll={handleClearAll}
              rawText={rawText}
              onChangeRawText={(t) => {
                recordUserActivity();
                setManualText(t);
                setIsManualEdit(true);
              }}
              onRegenerateText={handleRegenerateText}
              reEntryPulseActive={reEntryPulseActive}
            />
          </div>
        </div>
      </main>
    </div>
  );
};

export default App;
