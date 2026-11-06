# Production Engineering Guidelines & Architecture

All code and architecture within the `production/` partition adhere to strict production-grade reliability, performance, privacy, and transparency standards.

## Mandatory Reasoning Tree Requirement
**Every significant technical decision, architectural pattern, library choice, and non-trivial optimization records a Reasoning Tree.**

### Active Reasoning Trees (`production/reasoning/`):
1. **[RT-001: Technology Stack Selection](file:///c:/Users/travi/.gemini/antigravity-ide/scratch/therapist-documentation-tool/production/reasoning/RT-001_stack_selection.md)** — Native C++20 + Qt 6 (Qt Quick / RHI) selection for integrated GPU efficiency and deterministic 60–120 FPS presentation.
2. **[RT-002: Temporal & Re-Entry Visual Scaffolding](file:///c:/Users/travi/.gemini/antigravity-ide/scratch/therapist-documentation-tool/production/reasoning/RT-002_adhd_executive_function_cues.md)** — Circadian chromatic horizon drift (Morning Slate $\rightarrow$ Calming Sage $\rightarrow$ Warm Amber Twilight), window re-entry minimum-jerk luminance wave, and tactile completion settle animations.
3. **[RT-003: Multi-State Floating Overlay Widget Architecture](file:///c:/Users/travi/.gemini/antigravity-ide/scratch/therapist-documentation-tool/production/reasoning/RT-003_floating_overlay_widget_states.md)** — Continuous kinematic morphing across 4 discrete semantic states: `Puck (1x1)`, `Strip (3x1)`, `Sheet (2x2 Matrix)`, and `Full Workspace (82%)` with magnetic edge snapping.
4. **[RT-004: Universal OS Drag-and-Drop & Spatial Affordances](file:///c:/Users/travi/.gemini/antigravity-ide/scratch/therapist-documentation-tool/production/reasoning/RT-004_drag_and_drop_spatial_affordances.md)** — Raised satin perimeter borders for standard OS `QDrag` (`text/plain`) payloads into any web browser input field vs. 4 corner anchors for window relocation.
5. **[RT-005: Peripheral Motion & Minimum-Jerk Kinematic Cadence](file:///c:/Users/travi/.gemini/antigravity-ide/scratch/therapist-documentation-tool/production/reasoning/RT-005_peripheral_motion_and_kinematic_cadence.md)** — Magnocellular peripheral buoyancy cues after idle intervals (Bartram, Ware, & Calvert, 2003), minimum-jerk splines (Flash & Hogan, 1985), and next-step spatial priming (Altmann & Trafton, 2002).
6. **[RT-006: Cognitive Load & Working Memory Scaffolding](file:///c:/Users/travi/.gemini/antigravity-ide/scratch/therapist-documentation-tool/production/reasoning/RT-006_cognitive_load_and_working_memory_scaffolding.md)** — $4 \pm 1$ working memory chunking (Cowan, 2001), extraneous load reduction (Sweller, 1988), and 4-segment orbital Endowed Progress (Hull, 1932; Kivetz et al., 2006; Zeigarnik, 1927).
7. **[RT-007: Native C++20 Hybrid Search & Clinical Engine](file:///c:/Users/travi/.gemini/antigravity-ide/scratch/therapist-documentation-tool/production/reasoning/RT-007_hybrid_search_and_clinical_engine_architecture.md)** — Sub-millisecond Okapi BM25 + TF-IDF Cosine Vector Reciprocal Rank Fusion over 1,308 interventions across 20 modalities and 160 session themes.
8. **[RT-008: Zero-Alert Safe Download & Endpoint Security Hardening](file:///c:/Users/travi/.gemini/antigravity-ide/scratch/therapist-documentation-tool/production/reasoning/RT-008_zero_alert_safe_download_and_security_hardening.md)** — Embedded Win32 `asInvoker` UAC manifest, PE `VERSIONINFO` metadata, ASLR/DEP/High-Entropy VA linker hardening, strict offline CSP single-file HTML bundle, and SHA-256 verification.

## Building & Running the Native C++20 / Qt 6 Application

```powershell
$env:PATH = "C:\msys64\mingw64\bin;C:\Qt\6.6.0\mingw_64\bin;C:\Program Files\CMake\bin;" + $env:PATH
cmake -B production/build -S production -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Qt\6.6.0\mingw_64"
cmake --build production/build --parallel
.\production\build\tst_production_suite.exe
.\production\build\appTherapistDocumentationTool_Production.exe
```
