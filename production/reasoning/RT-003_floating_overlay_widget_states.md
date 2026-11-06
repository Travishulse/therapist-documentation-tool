# Reasoning Tree: Multi-State Floating Overlay Widget Architecture

**Decision ID:** RT-003  
**Status:** Approved  
**Date:** 2026-11-06  
**Context:** The application operates as an assistive floating clinical widget docked on top of a web browser running EHR platforms like "Sessions". It must support dynamic multi-state morphing (Puck -> Search Strip -> Grid Sheet -> Full Workspace) without stealing browser focus, obscuring form inputs, or hitching during animated transitions.

---

## 1. Academic & HCI Principles (2025–2026)

1. **Transient Micro-Task Interruption & Perceptual Occlusion**
   * *Citation:* *ACM CHI / UIST Research on Ambient Companion Overlays & Dual-Monitor Split Workflows*.
   * *Finding:* Forcing clinicians to switch windows (Alt+Tab or side-by-side tiling) fractures spatial memory and creates "interaction breakages". An **always-on-top, non-intrusive floating HUD** that expands on-demand and auto-docks to margins prevents working-memory flush.
2. **Progressive Disclosure & Fitts's Law in Multi-State UI**
   * *Citation:* *Hick-Hyman Law & Hierarchical Visual Expansion*.
   * *Finding:* Offering four distinct, predictable discrete scale states (1x1 Puck $\rightarrow$ 3x1 Strip $\rightarrow$ 3x3 Grid Sheet $\rightarrow$ 80% Centered Workspace) allows the clinician to match the UI footprint precisely to the current cognitive sub-task:
     - **Micro-Puck (Compact Ambient):** Zero occlusion of the EHR web form; provides subtle temporal time drift and quick reopen affordance.
     - **Quick Strip (Search Line):** Fast 1-line query without hiding the active patient text area.
     - **Grid Sheet (Intervention Matrix):** Multimodal browsing and card selection directly adjacent to the browser form fields.
     - **Full Workspace (80% Centered):** Deep synthesis, custom clinical editing, and narrative clipboard staging.
3. **Natural Kinematic State Transitions (Continuous Morphing)**
   * All geometry morphs ($W, H, X, Y$) interpolate simultaneously along polynomial minimum-jerk curves ($OutCubic / OutBack$), so the widget visually "stretches and settles" like a physical deck of cards rather than jumping discretely.

---

## 2. Reasoning Tree Structure

```
[Root: Multi-State Floating Overlay Widget Engine]
 │
 ├── [Hard Constraints]
 │    ├── C1: Always-On-Top Window Management (Win32 WS_EX_TOPMOST / Qt.WindowStaysOnTopHint)
 │    ├── C2: Non-Activating / Lightweight Interaction: Avoid stealing critical focus from browser inputs
 │    ├── C3: Smooth Dynamic Morphing: Zero window flicker, transparent canvas support, continuous layout math
 │    └── C4: Edge Docking & Magnetic Snapping: Clean parking along screen edges when minimized
 │
 ├── [Branch A: Separate Dialogs / Tiled Native Windows]
 │    ├── Evaluation: Spawning separate windows for search, grid, and full view.
 │    └── Pruning Justification (Rejected):
 │         ├── ❌ Clutters Windows taskbar and Alt+Tab stack.
 │         └── ❌ Jarring visual discontinuities between states.
 │
 ├── [Branch B: Static Web Extension (In-Browser Injection)]
 │    ├── Evaluation: Chrome extension injecting an iframe into the "Sessions" web page.
 │    └── Pruning Justification (Rejected):
 │         ├── ❌ Blocked by strict Content Security Policies (CSP) or iframe sandboxes on healthcare EHR portals.
 │         └── ❌ Subject to browser tab reloads, crashes, and poor 3D PBR rendering performance.
 │
 └── [Branch C: Frameless C++20/Qt 6 Transparent Layered Floating Widget] ---> [SELECTED]
      ├── Window Configuration:
      │    • Flags: `Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.SubWindow`
      │    • Surface: Transparent background (`Qt.WA_TranslucentBackground`) with hardware-accelerated DirectComposition.
      │
      ├── Discrete Semantic States:
      │    1. State "Puck"   (1x1): ~64x64 px circular/rounded pill. Docks subtly to the screen edge.
      │    2. State "Strip"  (3x1): ~480x56 px single-line omnibar for instant fuzzy searching.
      │    3. State "Sheet"  (3x3): ~580x460 px interactive 3D PBR intervention grid.
      │    4. State "Full"   (80%): Centered clinical narrative workspace with clipboard staging.
      │
      └── State Morphing Engine:
           • Unified ParallelAnimation animating `width`, `height`, `x`, and `y` concurrently.
           • Uses $OutCubic$ for expansion and $InOutQuad$ for edge retraction.
```

---

## 3. Qt 6 Native Implementation Specifications

* **Window Mode**: Frameless layered window positioned via screen geometry coordinates.
* **Magnetic Edge Snapping**: If the user drags the Puck within 40px of screen margins, it snaps and tucks partially into the margin, leaving an unobtrusive tab handle.
* **Keyboard Hotkey**: Optional global toggle (e.g. `Ctrl+Space` or `Alt+C`) to instantly cycle between Puck and Strip.

