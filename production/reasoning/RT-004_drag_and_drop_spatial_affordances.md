# Reasoning Tree: Universal OS Drag-and-Drop & Spatial Edge-Grab Architecture

**Decision ID:** RT-004  
**Status:** Approved  
**Date:** 2026-11-06  
**Context:** Implementing universal, browser-standard drag-and-drop from the floating companion into web browser input fields without overriding browser security features or injecting custom scripts into third-party websites ("Sessions"). Spatial grab partitioning must distinguish between window relocation and card drag without explanatory text.

---

## 1. Ergonomic & Spatial Affordance Principles

1. **Self-Evident Physical Affordance (Zero Explanatory Text)**
   * In tactile physical design, users inherently understand that **boundaries/raised edges** belong to individual objects, while **outer corners/margins** belong to the container.
   * **Visual Edge Treatment:** Each card features a distinct tactile physical boundary (2px raised satin border with specular highlight). Grabbing this raised perimeter initiates an OS-level `QDrag` payload containing the clinical text.
   * **Corner Margin Treatment:** The 4 corners of the companion window have distinct structural anchor styling; grabbing them initiates window movement (`Window.startSystemMove()`).

2. **Universal Browser OLE Drag-and-Drop Compliance**
   * Standard web browsers (Chrome, Edge, Firefox, Brave) natively intercept Windows OLE / MIME drag-and-drop messages through their standard DOM event pipeline (`dragenter`, `dragover`, `drop`).
   * By providing **standard, clean `text/plain` MIME payloads** (without proprietary non-standard hooks), any standard browser `<input>`, `<textarea>`, or contenteditable block accepts the drop, natively inserting it at the mouse position or active cursor position according to the browser's own default behavior.
   * Respects all platform security sandboxes and browser protections—no injection, no synthetic keypress spoofing, no privilege override.

---

## 2. Reasoning Tree Structure

```
[Root: Universal Native-to-Browser Drag-and-Drop with Zero-Text Spatial Grabs]
 │
 ├── [Hard Constraints]
 │    ├── C1: Universal Standard Compliance: Standard MIME text/plain via OS drag system
 │    ├── C2: Non-invasive: Zero script injection, zero security override on the target website
 │    ├── C3: Pure visual language: Zero explanatory helper text or labels explaining drag mechanics
 │    └── C4: Spatial separation: Clear edge grab for cards vs. corner grab for window movement
 │
 ├── [Branch A: Custom Web-Extension or Injected Scripts]
 │    ├── Evaluation: Bridge between native app and browser via WebSocket or content-script injection.
 │    └── Pruning Justification (Rejected):
 │         ├── ❌ Violates prompt requirement: must not override or hook into website internals.
 │         └── ❌ Breaks enterprise EHR sandboxes and requires user browser permissions.
 │
 ├── [Branch B: Clipboard Auto-Paste Simulation]
 │    ├── Evaluation: Copying to clipboard and simulating Ctrl+V on mouse release.
 │    └── Pruning Justification (Rejected):
 │         ├── ❌ Unreliable across focus changes and overrides existing user clipboard contents.
 │
 └── [Branch C: Standard OS Native Drag Payload with Spatial Perimeter Hit-Testing] ---> [SELECTED]
      ├── Windows OLE / Qt QDrag:
      │    • Uses standard `Qt.Drag.mimeData: { "text/plain": clinicalText }`
      │    • The browser's native text-editing engine handles the insertion at cursor/drop point.
      │
      ├── Spatial Hit-Testing:
      │    • Card Outer Edges: 6px tactile perimeter boundary triggers `card.Drag.start()`.
      │    • Companion Corners: 18x18 px corner anchors trigger `Window.startSystemMove()`.
      │    • Visual Language: Subtle satin border glint on cards; understated corner anchors on container.
```

