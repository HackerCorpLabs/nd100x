# Test Plan: Printer PDF Generation, Preview & Download

## Prerequisites

```bash
make wasm-glass
cd build_wasm_glass/bin
python3 -m http.server 8000
```

Open `http://localhost:8000` in Chrome/Firefox.

---

## 1. Window & Menu Structure

### 1.1 View Menu
- [ ] View menu shows **"Printer"** (not "Line Printer")
- [ ] No "Printer Settings" menu item exists
- [ ] Clicking "Printer" opens the printer window

### 1.2 Printer Window Layout
- [ ] Window title says **"Printer"**
- [ ] Header contains a **driver dropdown** (Text / ESC/P)
- [ ] **Three tabs** visible: Output, Jobs, Preview
- [ ] Output tab is active by default
- [ ] Jobs tab shows badge with "0"
- [ ] Active indicator area (right side of tab bar) is initially empty
- [ ] Window is draggable and resizable
- [ ] Close button works
- [ ] Window appears in taskbar when open

### 1.3 Config Window — Printer Section
- [ ] Open Config (cogwheel in taskbar)
- [ ] Scroll down — **"Printer"** section exists after Network
- [ ] Driver dropdown shows: "Text — Plain ASCII line printer" and "ESC/P — Epson ESC/P interpreter"
- [ ] "Flush Current Job" button is present
- [ ] Info text about 5s timeout and /prints/ is shown

---

## 2. Text Output (Output Tab)

### Setup
Power on → Boot SMD → wait for SINTRAN prompt.

### 2.1 Generate printer output
Type at SINTRAN console:
```
LIST-FILES ,,,
```
or any command that produces printer output (e.g. `@WRITE-FILE file-name LINEPRINTER` if a file exists).

Alternatively, if no easy SINTRAN printer command is available, any method that sends characters to I/O 0430 will work.

### 2.2 Verify Output tab
- [ ] Text appears in the Output tab `<pre>` element
- [ ] Text auto-scrolls to bottom as new output arrives
- [ ] Active indicator shows **"Printing..."** while output is streaming
- [ ] Long output (>2000 lines) gets trimmed from the top

---

## 3. Job Auto-Completion & Jobs Tab

### 3.1 Wait for timeout
After printer output stops, wait **~6 seconds** (5s timeout + 1s poll interval).

- [ ] Active indicator changes to **"Job 1 completed"**
- [ ] Jobs tab badge updates to **"1"**

### 3.2 Jobs tab content
Click the **Jobs** tab.

- [ ] Table shows one row with columns: Job#, Started, Ended, Lines, Bytes
- [ ] Job# is **1**
- [ ] Started/Ended show reasonable times (not "–" or epoch 0)
- [ ] Lines and Bytes are non-zero
- [ ] **Preview** and **Download** buttons are present in the row

### 3.3 Multiple jobs
Generate more printer output, wait for timeout again.

- [ ] Second job appears in Jobs tab
- [ ] Badge updates to **"2"**
- [ ] Job# is **2**

---

## 4. PDF Preview

Click **Preview** button on any job row.

- [ ] Tab switches to Preview tab automatically
- [ ] PDF renders in the iframe (browser's built-in PDF viewer)
- [ ] PDF contains the text that was printed (monospaced Courier font)
- [ ] If output spanned multiple pages (66+ lines), PDF has multiple pages

---

## 5. PDF Download

Click **Download** button on any job row.

- [ ] Browser downloads a file named **print-N.pdf** (where N = job number)
- [ ] Downloaded PDF opens correctly in a PDF viewer
- [ ] Content matches what was shown in Preview

---

## 6. Manual Flush

### 6.1 From Config window
Generate some printer output (so a job is active), then immediately:

- [ ] Open Config window
- [ ] Click **"Flush Current Job"**
- [ ] Job completes immediately (appears in Jobs tab without waiting 5s)

---

## 7. Driver Selection

### 7.1 Inline dropdown (printer window header)
- [ ] Change driver to **ESC/P** in printer window header dropdown
- [ ] Open Config window — Config dropdown also shows **ESC/P**

### 7.2 Config dropdown
- [ ] Change driver to **Text** in Config window
- [ ] Printer window header dropdown also shows **Text**

### 7.3 Persistence
- [ ] Set driver to **ESC/P**
- [ ] Reload page (F5)
- [ ] Both dropdowns show **ESC/P** on reload
- [ ] Power on the emulator
- [ ] Generate printer output — verify job completes (ESC/P driver active on C side)

### 7.4 Persistence across sessions
- [ ] Set driver to **Text**
- [ ] Close browser tab, reopen `http://localhost:8000`
- [ ] Both dropdowns show **Text**

---

## 8. Worker Mode

### 8.1 Switch to Worker mode
- [ ] Open Config → enable **"Background execution (Web Worker)"**
- [ ] Page reloads

### 8.2 Repeat core tests in Worker mode
- [ ] Power on → Boot SMD
- [ ] Generate printer output → text appears in Output tab
- [ ] Wait for timeout → job appears in Jobs tab with correct metadata
- [ ] Preview button → PDF renders in iframe
- [ ] Download button → PDF downloads correctly
- [ ] Driver dropdown changes propagate to both locations
- [ ] Flush button works

### 8.3 Worker-specific: async fsReadFile
- [ ] Preview/Download still work (these use async `emu.fsReadFile()` in Worker mode, returning a Promise)

---

## 9. Edge Cases

### 9.1 No output
- [ ] Jobs tab shows empty table (just headers)
- [ ] Badge shows "0"
- [ ] Preview tab shows empty gray iframe

### 9.2 Form feed
- [ ] If printer output contains a form feed (FF / 0x0C), it creates a page break in the PDF

### 9.3 Window persistence
- [ ] Drag printer window to a new position
- [ ] Close and reopen — position is restored
- [ ] Resize printer window
- [ ] Close and reopen — size is restored

### 9.4 Multiple previews
- [ ] Preview job 1, then Preview job 2 — iframe updates to job 2's PDF
- [ ] Blob URLs are cached (no re-read from MEMFS on second click)

---

## 10. No Regressions

- [ ] Paper Tape window still works (output display, download)
- [ ] Terminal output unaffected
- [ ] Virtual screen switching (Alt+1..6) still shows correct device windows
- [ ] Config window other sections (Theme, Emulation, Terminal, Network) still work
- [ ] Build `make debug` (native) still compiles clean
