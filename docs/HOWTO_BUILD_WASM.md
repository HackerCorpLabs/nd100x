
# 🛠️ WebAssembly Build Setup

This project supports building to **WebAssembly (WASM)** for running in the browser.

## ✅ Prerequisites

Before building, make sure you have the following installed:

1. **CMake**  
   Install from https://cmake.org/download/

2. **Emscripten SDK (emsdk)**  
   Follow these steps to install and activate Emscripten:

   Important! Install this in its own folder, not inside nd100x !!

   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh  # For Linux/macOS
   # On Windows: emsdk_env.bat
   ```

   ✅ This will install the Emscripten compiler and tools (like `emcc`, `emcmake`, etc.).

# 📦 Additional Notes

- Emscripten sets up an **in-memory virtual file system (MEMFS)** by default. 
- Make sure your browser supports WebAssembly (all modern browsers do).
- If you're integrating the `.wasm` file into an existing HTML/JS project, you can load it manually via `fetch()` or let Emscripten’s generated `.js` file handle loading for you.

---
# 🏃 Getting the emulator ready to run in the browser

- The default index.html for initilising and running the nd100x in the browser it the index.html found in the template folder.
- The template uses MEMFS and for the emulator to work you need to copy SMD0.IMG and/or FLOPPY.IMG to the build_wasm/bin folder.
- The template uses [xterm.js](https://xtermjs.org/) for emulating the terminal for console and has 4 more terminal devices enabled which will allow up to 5 users in Sintran.
   - Remember to execute @set-avail after sintran has booted to enable login on all terminals

## 🚀 Running in the Browser

To test in a browser, you need to serve the files over HTTP (due to browser security restrictions).
You can use Python’s simple HTTP server:

```bash
cd build-wasm
python3 -m http.server
```

Then open `http://localhost:8000` in your browser.
