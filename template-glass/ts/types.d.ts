//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs -- https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// Ambient type declarations for the Glass UI terminal modules.
// These describe globals provided by xterm.js CDN, the emu proxy layer,
// and other Glass JS modules so TypeScript can check cross-file references
// without import/export (module: "none" build).

// ---- xterm.js (loaded from CDN) ----

declare class Terminal {
  constructor(opts?: any);
  options: any;
  element: HTMLElement;
  rows: number;
  cols: number;
  loadAddon(addon: any): void;
  open(container: HTMLElement): void;
  write(data: string): void;
  refresh(start: number, end: number): void;
  focus(): void;
  dispose(): void;
  onKey(handler: (ev: { key: string; domEvent: KeyboardEvent }) => void): void;
  clearTextureAtlas?(): void;
}

declare var FitAddon: {
  FitAddon: new () => {
    fit(): void;
    proposeDimensions?(): { cols: number; rows: number } | undefined;
  };
};

declare var CanvasAddon: {
  CanvasAddon: new () => any;
} | undefined;

// ---- Emu proxy (emu-proxy.js / emu-proxy-worker.js) ----

interface EmuProxy {
  isReady(): boolean;
  sendKey(identCode: number, keyCode: number): number;
  getTerminalAddress(index: number): number;
  getTerminalIdentCode(index: number): number;
  getTerminalLogicalDevice(index: number): number;
  setTerminalCarrier(state: number, identCode: number): void;
  hasJSTerminalHandler(): boolean;
  hasAddFunction(): boolean;
  addFunction(fn: Function, sig: string): number;
  hasSetTerminalOutputCallback(): boolean;
  setTerminalOutputCallback(identCode: number, cb: number): void;
  hasRingBuffer?(): boolean;
  enableRingBuffer?(): boolean;
}

declare var emu: EmuProxy;

// ---- Shared terminal types ----

interface TerminalEntry {
  term: Terminal;
  fitAddon: any;
  container: HTMLElement;
  resizeTerminal: () => void;
}

interface ColorTheme {
  background: string;
  foreground: string;
  cursor: string;
}

interface TerminalSettings {
  fontFamily: string;
  colorTheme: string;
}

interface FontOption {
  val: string;
  label: string;
}

interface ColorOption {
  val: string;
  label: string;
}

// ---- Globals from module-init.js ----

declare var terminals: { [identCode: number]: TerminalEntry };
declare var loadingOverlayVisible: boolean;
declare var hasReceivedTerminalOutput: boolean;
declare var hasEverStartedEmulation: boolean;
declare function handleTerminalOutput(identCode: number, charCode: number): number;

// ---- Globals from toolbar.js ----

declare function makeDraggable(win: HTMLElement, header: HTMLElement, storageKey: string): void;
declare function makeResizable(win: HTMLElement, handle: HTMLElement, storageKey: string, minW: number, minH: number): void;
declare function openWindow(winId: string): void;
declare function closeWindow(winId: string): void;

// ---- Window manager (toolbar.js) ----

declare var windowManager: {
  register(id: string, label: string): void;
  focus(id: string): void;
  updateTaskbar(): void;
};

// ---- Globals exported by terminal-core.ts ----

interface Window {
  terminalColorThemes: { [name: string]: ColorTheme };
  createScaledTerminal: (container: HTMLElement, opts?: any) => {
    term: Terminal;
    fitAddon: any;
    resizeTerminal: () => void;
  };
  fitTerminalScaled: (term: Terminal, fitAddon: any, sizeDisplay?: HTMLElement) => void;
  terminalFontOptions: FontOption[];
  terminalColorOptions: ColorOption[];
  getTerminalSettings: (identCode: number) => TerminalSettings;
  saveTerminalSettings: () => void;
  loadTerminalSettings: () => void;
  setupTerminalKeyHandler: (term: Terminal, sendCallback: (keyCode: number) => void, isActiveCheck?: () => boolean) => void;
  buildFontSelectHTML: (identCode: number) => string;
  buildColorSelectHTML: (identCode: number) => string;
  getOpaqueTheme: (themeName: string) => ColorTheme;

  // terminal-manager.ts globals
  activeTerminalId: number;
  terminalDisplayNames: { [identCode: number]: string };
  terminalSettings: { [identCode: number]: TerminalSettings };
  applySettingsToTerminal: (identCode: number) => void;

  // terminal-bridge.ts globals
  popOutTerminal: (identCode: number) => void;
  popInTerminal: (identCode: number) => void;
  isPoppedOut: (identCode: number) => boolean;
  bufferPopoutOutput: (identCode: number, charCode: number) => void;
  broadcastThemeChange: (themeName: string) => void;
  broadcastSettingsChange: (identCode: number) => void;
}
