# Reasoning Tree: Cognitive Load & Working Memory Scaffolding

**Decision ID:** RT-006  
**Status:** Approved  
**Date:** 2026-11-06  
**Context:** Structuring 615+ clinical interventions and multi-section progress notes to minimize mental load and accelerate note completion using empirical cognitive psychology models—without displaying any clinical or diagnostic labels about executive functioning or attention in the interface.

---

## 1. University Research Evidence

1. **The $4 \pm 1$ Working Memory Capacity Limit**
   * *Citation:* **Cowan, N. (2001)**. *The magical number 4 in short-term memory: A reconsideration of mental storage capacity*. **University of Missouri**, *Behavioral and Brain Sciences*, 24(1), 87–114.
   * *Finding:* Active human working memory can simultaneously maintain only $4 \pm 1$ discrete chunks of information without rehearsal degradation. Interfaces presenting more than 4 simultaneous ungrouped decision nodes trigger choice overload and decision paralysis. By partitioning the documentation workflow into **4 discrete chapters** (`Themes`, `Interventions`, `Safety`, `Plan`) and presenting a **$2 \times 2$ (4-card) primary matrix** in compact Sheet view with effortless pagination, working memory load stays strictly within Cowan's boundary.

2. **Extraneous vs. Germane Cognitive Load Reduction**
   * *Citation:* **Sweller, J. (1988)**. *Cognitive load during problem solving: Effects on learning*. **University of New South Wales**, *Cognitive Science*, 12(2), 257–285.
   * *Finding:* Total mental load is the sum of intrinsic task complexity and extraneous interface friction. Eliminating redundant visual chrome, pre-synthesizing structured clinical prose automatically as cards are tapped, and dimming non-active peripheral controls by 25% contrast frees prefrontal capacity for clinical judgment.

3. **The Goal-Gradient Hypothesis & Endowed Progress Acceleration**
   * *Citation:* **Hull, C. L. (1932)**, **Yale University**, *The goal-gradient hypothesis and maze learning*, *Psychological Review*, 39(1), 25–43; and **Kivetz, R., Urminsky, O., & Zheng, Y. (2006)**, **Columbia University & University of Chicago**, *The Goal-Gradient Hypothesis Resurrected: Purchase Acceleration, Illusionary Goal Progress, and Customer Retention*, *Journal of Marketing Research*, 43(1), 39–58.
   * *Finding:* Task completion velocity increases non-linearly as perceived proximity to goal completion narrows. Furthermore, providing **Endowed Progress** (initializing a visual progress arc at 12% baseline upon session start rather than 0%) significantly lowers task-initiation inertia and increases completion rates.

4. **The Zeigarnik Open-Loop Tension Principle**
   * *Citation:* **Zeigarnik, B. (1927)**. *Das Behalten erledigter und unerledigter Handlungen (On finished and unfinished tasks)*. **Humboldt University of Berlin**, *Psychologische Forschung*, 9, 1–85.
   * *Finding:* Unfinished sub-tasks maintain involuntary retrieval tension until resolved by a concrete closure gesture. A segmented 4-arc orbital ring that visibly locks each arc into a warm emerald-gold state as its clinical section meets recommended item counts provides deterministic closure per section.

---

## 2. Reasoning Tree Structure

```
[Root: Minimize Mental Load & Accelerate Note Completion]
 │
 ├── [Hard Constraints]
 │    ├── C1: Strictly <= 4 top-level workflow chunks visible simultaneously (Cowan, 2001)
 │    ├── C2: Endowed Progress baseline (>0% visual start) to eliminate blank-page paralysis (Kivetz et al., 2006)
 │    ├── C3: Auto-Advance Option / Guided Flow across the 4 clinical chapters
 │    └── C4: Zero Direct Text of "ADHD", "Attention", "Focus", or "Function"
 │
 ├── [Branch A: Flat Unstructured List of All 615+ Items]
 │    ├── Evaluation: Scrolling through hundreds of dense text rows at once.
 │    └── Pruning Justification (Rejected):
 │         └── ❌ Violates Cowan's 4-chunk limit; induces scanning exhaustion and decision stall.
 │
 ├── [Branch B: Multi-Page Wizard Blocking Free Navigation]
 │    ├── Evaluation: Strict modal wizard requiring "Next/Back" clicks before viewing other sections.
 │    └── Pruning Justification (Rejected):
 │         └── ❌ Restricts non-linear clinical charting when a therapist recalls a detail out of order.
 │
 └── [Branch C: 4-Chapter Segmented Orbital Progress + 2x2 Tactile Card Chunking] ---> [SELECTED]
      ├── Sub-Mechanism 1: 4-Segment Orbital Completion Ring (Zeigarnik + Goal-Gradient)
      │    └── Displays 4 curved arc segments corresponding to the 4 clinical sections:
      │         1. Themes (1–3 items)
      │         2. Interventions (3–5 items)
      │         3. Safety (1+ item)
      │         4. Plan (1+ item)
      │    └── Initializes with a 12% warm baseline glow (Endowed Progress) and accelerates chromatic brilliance
      │        as sections fill, culminating in a soft emerald-gold completion seal when ready to copy/drag.
      │
      ├── Sub-Mechanism 2: Cowan 2x2 Spatial Card Chunking (Sheet State)
      │    └── Presents 4 high-affordance tactile cards at a time in the compact Sheet overlay with smooth
      │        wheel/pager cycling, keeping visual density calm and scannable in a single glance.
      │
      └── Sub-Mechanism 3: One-Tap Quick-Complete Defaults
           └── Includes instant 1-tap baseline screening statements (e.g., routine safety screening, recurring weekly
               schedule) so clinicians can close routine sections in a single tactile click.
```

