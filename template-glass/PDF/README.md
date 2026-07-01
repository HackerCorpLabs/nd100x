# PDF manuals for the Glass UI Help menu

Place the manual PDFs here. They are served as-is and rendered by the
browser's native PDF viewer inside a draggable glass window (Help menu).

Expected files (exact names, referenced from index.html):

- `ND-30003-7-NO Håndbok for driftsansvarlige.pdf`   -> Help > Håndbok
- `ND-30.003.06A_SINTRAN_III_System_Supervisor.pdf`  -> Help > System Supervisor

The `make wasm-glass` build copies this whole folder to
`build_wasm_glass/bin/PDF/`. To add another manual: drop the PDF here,
then add a window + Help menu entry in `template-glass/index.html` and
wire it in `template-glass/js/toolbar.js` (see `openPdfWindow`).
