# Therapist Documentation Companion

Client-side clinical note builder, floating overlay companion, and hybrid intervention search engine for **Liz Wegner Proprietary Software Collective**. Indexes **1,308 clinical interventions across 20 therapeutic modalities**, **160 session themes**, **12 clinical safety indicators**, and **12 care-plan statements**—running 100% locally with zero network requests.

## Safe Download & Zero-Alert Launch Options

1. **Open in Browser (Zero Download Prompts)**:
   Visit `https://travishulse.github.io/therapist-documentation-tool/` on Windows, macOS, iPadOS, or ChromeOS.
2. **Offline Single-File HTML (`release/Therapist-Documentation-Tool.html`)**:
   Download `Therapist-Documentation-Tool.html` and double-click it to open in Chrome, Edge, Safari, or Firefox. Runs 100% offline under a strict `connect-src 'none'` Content Security Policy with zero SmartScreen or executable warnings.
3. **Native C++20 / Qt 6 Floating Companion (`production/build/appTherapistDocumentationTool_Production.exe`)**:
   Always-on-top native Windows companion with 4 kinematic states (`Puck`, `Quick Strip`, `2×2 Sheet`, and `Full Studio`), embedded Win32 `asInvoker` security manifest, and native OS drag-and-drop into any EHR browser field.

## Clinical Workflow

1. Navigate across the 4 clinical chapters at the top: **Session Themes** (`1–3`), **Interventions** (`3–5`), **Safety & Risk** (`0–3`), and **Plan & Next Steps** (`0–3`).
2. Search by keyword, presenting symptom, or therapeutic modality (`CBT`, `ACT`, `DBT`, `CPT`, `Gottman Method`, `Family Systems`, `SFBT`, `CFT`, `ERP`, `Relational`, `Psychodynamic`, `EFT`, `Attachment-Based`, `Existential`, `Somatic`, `Person-Centered`, `Motivational Interviewing`, `CBT for Couples`, `Discernment Counseling`).
3. Click the interior of any clinical card to append or remove it from the conjoined clinical narrative, or grab the raised satin border of any card (or the `⇄` handle) to drag and drop its clinical text directly into your EHR browser form.
4. Edit the conjoined narrative directly if needed, then click **📋 Copy to Clipboard** or **Save .txt**.

## Build & Verify From Source

```bash
npm install
npm run lint
npm run build
```
