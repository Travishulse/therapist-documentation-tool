# Reasoning Tree: Zero-Alert Safe Download & Endpoint Security Hardening

**Decision ID:** RT-008  
**Status:** Approved  
**Date:** 2026-11-06  
**Context:** Ensuring the production product maintains strict security invariants while minimizing false-positive browser download blocks, email gateway rejections, and OS security warnings (Microsoft Defender SmartScreen, Google Safe Browsing, Mark-of-the-Web).

---

## 1. Security & Delivery Threat/Heuristic Analysis

1. **Why Clinical Tools Trigger False-Positive Download Warnings**
   * **Email Gateways (Google Workspace SMTP 552 5.7.0)**: Automatically block `.exe`, `.bat`, `.ps1`, and `.zip` archives containing executables or obfuscated scripts.
   * **Browser Safe Browsing (Chromium / Edge)**: Flags raw unsigned `.exe` downloads or HTML files containing external scripts, `eval()`, or mixed-content network requests.
   * **Windows Defender / SmartScreen PE Heuristics**: Penalizes native Windows executables that:
     - Lack an embedded Win32 UAC Application Manifest (`RT_MANIFEST`) explicitly declaring `<requestedExecutionLevel level="asInvoker" uiAccess="false"/>`.
     - Lack a complete `VERSIONINFO` resource block (`CompanyName`, `FileDescription`, `ProductName`, `FileVersion`, `OriginalFilename`, `LegalCopyright`).
     - Lack modern exploit-mitigation PE header flags (`IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE` / ASLR, `IMAGE_DLLCHARACTERISTICS_NX_COMPAT` / DEP, `IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA`).
     - Import suspicious Win32 APIs (`SetWindowsHookEx`, `SendInput`, `keybd_event`, `VirtualAllocEx`, `WriteProcessMemory`) or use binary packers (UPX).

---

## 2. Reasoning Tree Structure

```
[Root: Maximize Security & Minimize Download / Execution Safety Alerts]
 │
 ├── [Hard Constraints]
 │    ├── C1: Never weaken or bypass OS, browser, or EHR security sandboxes
 │    ├── C2: Zero network telemetry, zero external script loads, 100% local clinical privacy
 │    ├── C3: Zero invasive Win32 APIs (no global hooks, no synthetic keystroke injection, no memory patching)
 │    └── C4: Dual-format safe delivery (Zero-Alert Single-File HTML + Hardened Native PE Bundle)
 │
 ├── [Branch A: Self-Extracting .EXE Installer or UPX-Packed Binary]
 │    ├── Evaluation: Packing executable assets into a compressed self-extractor.
 │    └── Pruning Justification (Rejected):
 │         └── ❌ High-entropy packed sections trigger immediate heuristic malware alerts in Windows Defender.
 │
 ├── [Branch B: PowerShell / Batch Downloader Wrapper Sent as Attachment]
 │    ├── Evaluation: Sending `.bat` or `.ps1` scripts to end-users.
 │    └── Pruning Justification (Rejected):
 │         └── ❌ Blocked by Google Workspace email filters and flagged by Windows Mark-of-the-Web (MotW).
 │
 └── [Branch C: Hardened Dual-Artifact Architecture + Strict CSP + Clean PE Manifest] ---> [SELECTED]
      ├── Tier 1: Zero-Alert Single-File HTML Deliverable (`release/Therapist-Documentation-Tool.html`)
      │    • Strict offline Content Security Policy (`default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; img-src data:; connect-src 'none'`).
      │    • Zero external URLs, zero `eval()`, zero `new Function()`, clean UTF-8 bundle.
      │    • Downloads and opens in any browser (Chrome, Edge, Safari, Firefox) with **zero SmartScreen alerts**
      │      and **zero executable warnings**.
      │
      ├── Tier 2: Hardened Native C++20/Qt6 Executable (`production/`)
      │    • Embedded Win32 Resource (`app.rc`) with complete `VERSIONINFO` metadata and `RT_MANIFEST`
      │      declaring `level="asInvoker" uiAccess="false"` and Windows 10/11 compatibility GUIDs.
      │    • Compiled with ASLR (`-Wl,--dynamicbase`), DEP/NX (`-Wl,--nxcompat`), and 64-bit High-Entropy VA
      │      (`-Wl,--high-entropy-va`).
      │    • Uses only standard OS `QDrag` (`text/plain`) and `QClipboard`—zero hooks or input spoofing.
      │
      └── Tier 3: Cryptographic Verification (`release/SHA256SUMS.txt`)
           • Publishes deterministic SHA-256 hashes for all release artifacts to guarantee supply-chain integrity.
```

