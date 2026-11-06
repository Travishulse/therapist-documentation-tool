# Reasoning Tree: Technology Stack Selection (Production Partition)

**Decision ID:** RT-001  
**Status:** Approved  
**Date:** 2026-11-06  
**Context:** Rewriting the application from a prototype/web baseline into a zero-latency, high-fidelity native application with real-time dynamic layout transitions, physically-based materials, and natural light reflections.

---

```
[Root: Rewrite Application for Real-Time Animated Photorealistic UI]
 │
 ├── [Hard Constraints]
 │    ├── C1: Non-deterministic end-user hardware (Must run smoothly on laptop iGPU / Intel Iris Xe / AMD Radeon)
 │    ├── C2: Cold-launch & execution latency: Near-zero startup, deterministic frame delivery (60-120 FPS pinned)
 │    ├── C3: Per-item dynamic animated transitions (FLIP / physics-based element choreography on state change)
 │    ├── C4: Photorealistic visuals: Natural environmental lighting (IBL/HDRI), PBR materials, specular reflections
 │    └── C5: High LLM code generation success rate (minimal syntax friction, borrow-checker deadlocks, or manual lifetime bugs)
 │
 ├── [Branch A: Rust (egui / iced / wgpu / Bevy)]
 │    ├── Evaluation:
 │    │    ├── Performance: High native speed, zero GC overhead.
 │    │    ├── iGPU Efficiency: Good with wgpu.
 │    ├── Pruning Justification (Rejected):
 │    │    ├── ❌ Borrow-checker & lifetime conflicts with complex retained UI trees and dynamic animation interpolators.
 │    │    ├── ❌ Immature GUI ecosystem: egui lacks declarative per-item choreography; iced lacks high-end 3D PBR shader pipelines.
 │    │    └── ❌ LLM Generation friction: Substantial rate of borrow errors when writing dynamic UI graphs.
 │
 ├── [Branch B: Heavy Game Engine (Unreal Engine 5 / Unity HDRP)]
 │    ├── Evaluation:
 │    │    ├── Visual Realism: Photorealistic (Lumen, Nanite, Raytracing).
 │    ├── Pruning Justification (Rejected):
 │    │    ├── ❌ Fails C1: Extreme thermal load, high battery drain, and unplayable stutter on laptop integrated GPUs.
 │    │    ├── ❌ Fails C2: Heavy binary footprint, slow cold launch (several seconds), massive asset pipelines.
 │    │    └── ❌ Poor desktop application UI ergonomics (window management, form controls, high overhead).
 │
 ├── [Branch C: Managed Native (.NET 9 Native AOT + Avalonia / Skia3D)]
 │    ├── Evaluation:
 │    │    ├── LLM Generation: High.
 │    │    ├── UI Ergonomics: Excellent declarative XAML.
 │    ├── Pruning Justification (Rejected):
 │    │    ├── ❌ PBR 3D integration with dynamic lighting and reflections requires custom OpenGL/Vulkan interop layers.
 │    │    └── ⚠️ Native AOT reduces startup delay, but GPU scene graph lacks integrated real-time lighting pipelines out of the box.
 │
 └── [Branch D: C++20 + Qt 6 (Qt Quick & Qt Quick 3D)] ---> [SELECTED]
      ├── Validation against Constraints:
      │    ├── C1 (iGPU Efficiency): Qt RHI (Rendering Hardware Interface) targets Direct3D 11/12 on Windows with batched draw calls, forward PBR, and split-sum environment reflections. Runs cool (<5% CPU) on low-wattage laptops.
      │    ├── C2 (Latency): Native compiled C++20 backend with instant process launch and direct DWM/DirectComposition flip presentation.
      │    ├── C3 (Per-item Animations): Built-in declarative `Transition`, `Behavior`, and `LayoutItemProxy` with hardware-accelerated property interpolators.
      │    ├── C4 (Visual Realism): Native `PrincipledMaterial` (metalness, roughness, clearcoat), `lightProbe` (HDRI/IBL), and Screen-Space Reflections (SSR).
      │    └── C5 (LLM Reliability): Decades of established Qt/QML training data; clean separation of C++ logic and declarative QML visuals minimizes synthesis errors.
```

---

## Decision Matrix Summary

| Criterion | Rust (egui/wgpu) | Unreal Engine 5 | .NET Avalonia | **C++20 / Qt 6 Quick 3D** |
| :--- | :--- | :--- | :--- | :--- |
| **iGPU Compatibility** | Good | Poor (Hitching/Throttling) | Very Good | **Superior (Engineered for Embedded/iGPU)** |
| **PBR Natural Lighting & Reflection** | Requires custom shaders | Photorealistic | Minimal (Requires FFI) | **Built-in (PrincipledMaterial, IBL, SSR)** |
| **Per-Item Choreographed Animation** | Manual layout math | High complexity | Moderate | **Native Declarative Transitions (`add`, `displaced`)** |
| **Cold Startup & Run Latency** | Zero GC / Instant | Heavy (>3-10s) | Sub-second (Native AOT) | **Instant (<100ms Native)** |
| **LLM Generation Success** | Moderate / Borrow friction | Moderate | High | **Very High** |

---

## Architectural Implementation Plan for `production/`

1. **Core Engine (`production/src/core/`)**:
   - Modern C++20 data processing, model abstractions, and asynchronous task management.
2. **Visual Presentation Layer (`production/src/ui/`)**:
   - QML / Qt Quick 3D scene graphs with PBR physical materials, HDRI environment reflections, and directional key lighting.
3. **Choreography System (`production/src/ui/transitions/`)**:
   - Declarative item-level displacement, spring easing, and state-driven morphing.
4. **Verification & Profiling (`production/tests/`)**:
   - Frame-time telemetry validating continuous 60/120 FPS targets under integrated graphics load.

