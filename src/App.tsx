import React, { useState, useEffect, useMemo, useCallback } from 'react';
import type { ClinicalItem, QuestionType, DocumentationNote } from './types/documentation';
import { QuestionSelector, QUESTION_CONFIGS } from './components/QuestionSelector';
import { SearchInput } from './components/SearchInput';
import { ResultsList } from './components/ResultsList';
import { ConjoinedOutputBox } from './components/ConjoinedOutputBox';
import { HybridSearchEngine } from './search/hybridEngine';
import { RefreshCw, HeartHandshake } from 'lucide-react';
import clinicalBanksData from '../data/clinical_banks.json';

export const App: React.FC = () => {
  const [dataLoaded, setDataLoaded] = useState(false);
  const [currentQuestion, setCurrentQuestion] = useState<QuestionType>('interventions');
  const [query, setQuery] = useState('');
  const [selectedModality, setSelectedModality] = useState('all');
  const [searchMode, setSearchMode] = useState<'hybrid' | 'lexical' | 'semantic'>('hybrid');

  // Clinical data collections
  const [interventions, setInterventions] = useState<ClinicalItem[]>([]);
  const [sessionFocusItems, setSessionFocusItems] = useState<ClinicalItem[]>([]);
  const [riskFactorsItems, setRiskFactorsItems] = useState<ClinicalItem[]>([]);
  const [additionalInfoItems, setAdditionalInfoItems] = useState<ClinicalItem[]>([]);
  const [modalitiesList, setModalitiesList] = useState<string[]>([]);

  // Selected note items
  const [note, setNote] = useState<DocumentationNote>(() => {
    const saved = localStorage.getItem('therapist_note_draft');
    if (saved) {
      try {
        return JSON.parse(saved);
      } catch (e) {
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

  const [rawText, setRawText] = useState('');
  const [isManualEdit, setIsManualEdit] = useState(false);

  // Load JSON bank data directly inlined for zero-network single file execution
  useEffect(() => {
    const data = clinicalBanksData as any;
    const mappedInterventions: ClinicalItem[] = data.interventions.map((item: any) => ({
          ...item,
          question: 'interventions' as QuestionType
        }));
        const mappedSf: ClinicalItem[] = data.sessionFocus.map((item: any) => ({
          ...item,
          question: 'session_focus' as QuestionType,
          fullText: `${item.title}: ${item.description}`
        }));
        const mappedRf: ClinicalItem[] = data.riskFactors.map((item: any) => ({
          ...item,
          question: 'risk_factors' as QuestionType,
          fullText: `${item.title}: ${item.description}`
        }));
        const mappedAi: ClinicalItem[] = data.additionalInfo.map((item: any) => ({
          ...item,
          question: 'additional_info' as QuestionType,
          fullText: `${item.title}: ${item.description}`
        }));

        setInterventions(mappedInterventions);
        setSessionFocusItems(mappedSf);
        setRiskFactorsItems(mappedRf);
        setAdditionalInfoItems(mappedAi);
        setModalitiesList(data.metadata.modalities || []);
        setDataLoaded(true);
  }, []);

  // Save drafts to localStorage
  useEffect(() => {
    localStorage.setItem('therapist_note_draft', JSON.stringify(note));
  }, [note]);

  // Engines per question
  const currentItems = useMemo(() => {
    switch (currentQuestion) {
      case 'session_focus': return sessionFocusItems;
      case 'interventions': return interventions;
      case 'risk_factors': return riskFactorsItems;
      case 'additional_info': return additionalInfoItems;
    }
  }, [currentQuestion, sessionFocusItems, interventions, riskFactorsItems, additionalInfoItems]);

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
      35
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

  // Conjoin function to assemble standard note
  const generateConjoinedNarrative = useCallback((currentNote: DocumentationNote): string => {
    const sections: string[] = [];

    if (currentNote.session_focus.length > 0) {
      const sfText = currentNote.session_focus
        .map(item => `• ${item.title}: ${item.description}`)
        .join('\n');
      sections.push(`SESSION FOCUS:\n${sfText}`);
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

  // Update conjoined text automatically unless therapist is actively editing custom text
  useEffect(() => {
    if (!isManualEdit) {
      setRawText(generateConjoinedNarrative(note));
    }
  }, [note, isManualEdit, generateConjoinedNarrative]);

  // Click handler to add or toggle item in conjoined output
  const handleSelectItem = (item: ClinicalItem) => {
    setNote(prev => {
      const question = item.question;
      const list = prev[question];
      const exists = list.some(i => i.id === item.id);

      const updatedList = exists
        ? list.filter(i => i.id !== item.id)
        : [...list, item];

      const newNote = {
        ...prev,
        [question]: updatedList
      };

      // Also refresh raw text
      setRawText(generateConjoinedNarrative(newNote));
      setIsManualEdit(false);
      return newNote;
    });
  };

  const handleRemoveItem = (item: ClinicalItem) => {
    handleSelectItem(item);
  };

  const handleClearAll = () => {
    const emptyNote = {
      session_focus: [],
      interventions: [],
      risk_factors: [],
      additional_info: [],
      customEdits: {}
    };
    setNote(emptyNote);
    setRawText('');
    setIsManualEdit(false);
  };

  const handleRegenerateText = () => {
    setIsManualEdit(false);
    setRawText(generateConjoinedNarrative(note));
  };

  const selectionCounts = {
    session_focus: note.session_focus.length,
    interventions: note.interventions.length,
    risk_factors: note.risk_factors.length,
    additional_info: note.additional_info.length
  };

  const activeQuestionConfig = QUESTION_CONFIGS.find(q => q.id === currentQuestion)!;

  return (
    <div className="min-h-screen bg-[#F8F7F4] text-[#242927] flex flex-col font-sans selection:bg-[#D4E3DB] selection:text-[#183328]">
      {/* Top Application Header */}
      <header className="border-b border-[#E8E5DF] bg-white/95 backdrop-blur-md sticky top-0 z-40 px-4 md:px-8 py-3 flex items-center justify-between shadow-xs">
        <div className="flex items-center gap-3">
          <img
            src="/app-icon.png"
            alt="Therapist Documentation Icon"
            className="w-10 h-10 rounded-xl shadow-xs border border-[#E2DED6] object-cover"
          />
          <div>
            <h1 className="text-sm md:text-base font-extrabold tracking-tight text-[#242927] flex items-center gap-2">
              Therapist Documentation Search Engine
              <span className="text-[10px] font-semibold uppercase tracking-wider bg-[#EDF3F0] text-[#345345] border border-[#D4E3DB] px-2 py-0.5 rounded-full">
                Liz Wegner Proprietary Software Collective
              </span>
            </h1>
            <p className="text-[11px] text-[#6B7570]">
              Gentle clinical documentation • 615+ Modality Interventions • Instant in-memory search
            </p>
          </div>
        </div>

        <div className="flex items-center gap-2 text-xs text-[#6F7773]">
          <span className="hidden md:inline-flex items-center gap-1.5 bg-[#F2F0EB] border border-[#E0DDD6] px-2.5 py-1 rounded-lg text-[#4E5652] text-[11px] font-medium">
            <HeartHandshake className="w-3.5 h-3.5 text-[#4A6B5D]" />
            100% In-Browser & Private
          </span>
        </div>
      </header>

      {/* Main Container */}
      <main className="flex-1 max-w-7xl w-full mx-auto p-4 md:p-6 space-y-5">
        {/* Top Question Selector Bar */}
        <QuestionSelector
          currentQuestion={currentQuestion}
          onSelectQuestion={(q) => {
            setCurrentQuestion(q);
            setQuery(''); // Clear query when switching questions for clean search
          }}
          selectionCounts={selectionCounts}
        />

        {/* Two-Column Working Layout */}
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-5 items-start">
          {/* Left Column: Search & Results (7 cols) */}
          <div className="lg:col-span-7 space-y-4">
            <SearchInput
              query={query}
              onChangeQuery={setQuery}
              placeholder={activeQuestionConfig.placeholder}
              selectedModality={selectedModality}
              onSelectModality={setSelectedModality}
              availableModalities={modalitiesList}
              searchMode={searchMode}
              onChangeSearchMode={setSearchMode}
              classification={classification}
              isInterventions={currentQuestion === 'interventions'}
            />

            {dataLoaded ? (
              <ResultsList
                results={searchResults}
                onSelectItem={handleSelectItem}
                selectedItemIds={selectedItemIds}
                query={query}
              />
            ) : (
              <div className="text-center py-12 text-[#7F8884] flex items-center justify-center gap-2">
                <RefreshCw className="w-5 h-5 animate-spin text-[#4A6B5D]" />
                <span>Loading clinical banks...</span>
              </div>
            )}
          </div>

          {/* Right Column: Conjoined Output Box (5 cols) */}
          <div className="lg:col-span-5 sticky top-20">
            <ConjoinedOutputBox
              note={note}
              onRemoveItem={handleRemoveItem}
              onClearAll={handleClearAll}
              rawText={rawText}
              onChangeRawText={(t) => {
                setRawText(t);
                setIsManualEdit(true);
              }}
              onRegenerateText={handleRegenerateText}
            />
          </div>
        </div>
      </main>
    </div>
  );
};

export default App;
