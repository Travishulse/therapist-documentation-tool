# Reasoning Tree: Peripheral Motion & Minimum-Jerk Kinematic Cadence

**Decision ID:** RT-005  
**Status:** Approved  
**Date:** 2026-11-06  
**Context:** Sustaining clinician task momentum and guiding ocular re-orientation back to incomplete clinical note sections after idle pauses or EHR context switches—strictly using subtle biomechanical movement and peripheral visual physics, with zero explicit text references to attention, focus, ADHD, or executive function.

---

## 1. University Research Evidence

1. **Pre-Attentive Peripheral Motion Detection (Magnocellular Pathway Activation)**
   * *Citation:* **Bartram, L., Ware, C., & Calvert, T. (2003)**. *Moticons: detection, distraction and task*. **Simon Fraser University & University of New Hampshire**, *International Journal of Human-Computer Studies*, 58(5), 515–545.
   * *Finding:* The human retina's magnocellular pathway in peripheral vision is highly sensitive to low-frequency, small-amplitude smooth motion (< 4px excursion, 0.5–1.2 Hz sinusoidal easing) while remaining insensitive to static color/text changes outside the fovea. Crucially, smooth sinusoidal micro-motions below 4px amplitude reliably attract peripheral awareness back to a pending task node without inducing foveal distraction, startle reflexes, or irritation.

2. **Minimum-Jerk Trajectory Optimization in Human Oculomotor & Motor Control**
   * *Citation:* **Flash, T., & Hogan, N. (1985)**. *The coordination of arm movements: An experimentally confirmed mathematical model*. **MIT Artificial Intelligence Laboratory & Department of Psychology**, *The Journal of Neuroscience*, 5(7), 1688–1703.
   * *Finding:* Biological reaching and eye-tracking movements minimize the time-integrated squared jerk:
     $$J = \frac{1}{2} \int_{0}^{T} \left( \left(\frac{d^3x}{dt^3}\right)^2 + \left(\frac{d^3y}{dt^3}\right)^2 \right) dt$$
     producing symmetric, bell-shaped velocity profiles ($x(\tau) = x_0 + (x_f - x_0)(10\tau^3 - 15\tau^4 + 6\tau^5)$). UI elements that morph and settle along minimum-jerk curves match the central nervous system's internal forward kinematic model, eliminating visual-vestibular friction and lowering perceptual fatigue during repetitive charting.

3. **Activation-Based Goal Memory & Associative Spatial Priming**
   * *Citation:* **Altmann, E. M., & Trafton, J. G. (2002)**. *Memory for goals: An activation-based model*. **Michigan State University & Naval Research Laboratory**, *Cognitive Science*, 26(1), 39–83.
   * *Finding:* When a clinician pauses or switches to an EHR window to verify patient demographics, the activation level $A(t) = B_i + \sum W_j S_{ji}$ of the suspended note-writing goal decays exponentially ($B_i = \ln(\sum t_k^{-d})$). Providing a localized visual priming cue directly on the next incomplete clinical section immediately before and after idle intervals (> 15 seconds) restores goal activation without requiring conscious memory reconstruction.

---

## 2. Reasoning Tree Structure

```
[Root: Sustain Note-Finishing Momentum via Subtle Kinematic Movement]
 │
 ├── [Hard Constraints]
 │    ├── C1: Zero Explicit Nomenclature: Never display words "ADHD", "Attention", "Focus", or "Function"
 │    ├── C2: Sub-Startle Amplitude: Motion excursions must remain <= 3.5px with continuous C2 derivatives
 │    ├── C3: Idle-Triggered Priming: Activate only when interaction cadence pauses (>12-18s) or on window re-entry
 │    └── C4: Zero Input Blocking: Never use modal dialogs, audio chimes, or cursor hijacking
 │
 ├── [Branch A: Textual Reminders / Pop-up Nudges ("Don't forget to finish your note!")]
 │    ├── Evaluation: Explicit banner or toast notification after idle timer.
 │    └── Pruning Justification (Rejected):
 │         ├── ❌ Violates C1 and induces demand-avoidance / reactance.
 │         └── ❌ Adds extraneous reading load (Sweller, 1988).
 │
 ├── [Branch B: High-Frequency Blinking / Bouncing Icons]
 │    ├── Evaluation: Rapid step-function blinking or high-amplitude bounce.
 │    └── Pruning Justification (Rejected):
 │         └── ❌ Bartram et al. (2003) proved high-frequency/step-edge motion triggers peripheral irritation and sensory fatigue.
 │
 └── [Branch C: Minimum-Jerk Kinematic Cadence + Magnocellular Peripheral Breathing] ---> [SELECTED]
      ├── Mechanism 1: Idle Cadence Peripheral Breathing (Bartram et al., 2003)
      │    └── After 12 seconds of user inactivity while a note is incomplete, the next recommended category pill
      │        and the Puck/Strip perimeter emit a 2.5px sinusoidal vertical buoyancy drift + soft specular rim wave
      │        at 0.45 Hz. Instantly ceases the millisecond the user moves the pointer or types.
      │
      ├── Mechanism 2: Minimum-Jerk State & Card Choreography (Flash & Hogan, 1985)
      │    └── All state transitions (Puck <-> Strip <-> Sheet <-> Full) and card insertions follow 5th-order
      │        minimum-jerk / cubic-bezier splines, with staggered 35ms per-card entrance choreography.
      │
      └── Mechanism 3: Next-Step Spatial Locus Priming (Altmann & Trafton, 2002)
           └── A warm luminous indicator bead smoothly glides along the category rail to rest beneath the next
               unfulfilled clinical section (Themes -> Interventions -> Safety -> Plan), acting as an externalized
               associative memory pointer.
```

---

## 3. Verification Criteria
- **Zero Forbidden Strings**: Automated test verifies 0 occurrences of `ADHD`, `ATTENTION`, `FOCUS`, `FUNCTION` in all user-facing strings and UI components.
- **Kinematic Continuity**: All transitions execute via hardware-accelerated interpolators at 60+ FPS with zero step discontinuities.
- **Idle Cadence Responsiveness**: Idle timer resets immediately (< 16ms) upon keystroke, click, or drag gesture.

