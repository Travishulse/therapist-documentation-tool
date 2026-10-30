# Therapist Documentation Search Engine

Client-side clinical note builder and intervention search engine for Liz Wegner Proprietary Software Collective. All 615 clinical interventions across 12 therapeutic modalities run locally inside a single self-contained HTML file with zero network requests.

## Open for testing

Choose either option:

1. **Open in browser.** Visit `https://travishulse.github.io/therapist-documentation-tool/` on Windows, macOS, iPadOS, or ChromeOS.
2. **Download offline single-file HTML.** Download `Therapist-Documentation-Tool.html` from the latest GitHub Release and double-click it on your computer. No installation, extraction, or internet connection is required.

## Clinical workflow

1. Switch across **Session Focus**, **Interventions**, **Risk Factors**, and **Additional Info** tabs at the top.
2. Search by keyword, symptom, or therapeutic modality (`CBT`, `ACT`, `DBT`, `CPT`, `Gottman Method`, `Family Systems`, `Trauma-Focused`, `Psychodynamic`, `Person-Centered`, `Motivational Interviewing`, `CBT for Couples`, `Discernment Counseling`).
3. Click any clinical card to append or remove it from the conjoined clinical narrative on the right.
4. Edit the narrative text directly if needed, then click **Copy to Clipboard** to paste into your EHR or **Save .txt** to save a local plain-text file.

## Build from source

Requires Node.js 22 or newer.

```bash
npm install
npm run lint
npm run build
```

`npm run build` compiles TypeScript, compacts `data/clinical_banks.json`, bundles CSS and classic IIFE JavaScript into a single self-contained `dist/index.html`, writes `release/Therapist-Documentation-Tool.html` and `release/Therapist-Documentation-Tool.zip`, and verifies offline Content Security Policy, V8 classic-script syntax, and Google Workspace attachment compliance.
