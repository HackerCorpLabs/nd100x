
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

## 🔧 Building WebAssembly

We use a **separate build folder** to keep WASM builds isolated.

### 1️⃣ Set up the build folder:

```bash
mkdir build-wasm
cd build-wasm
```

### 2️⃣ Run CMake with Emscripten:

```bash
emcmake cmake ..
```

✅ This tells CMake to use the Emscripten toolchain.

### 3️⃣ Build the project:

```bash
cmake --build .
```

### 4️⃣ Output

The build will produce files like:

- `myproject.wasm`
- `myproject.js`
- (optionally) `myproject.html`

By default, the `.js` file is used to load and run the `.wasm` binary.

## 🚀 Running in the Browser

To test in a browser, you need to serve the files over HTTP (due to browser security restrictions).

You can use Python’s simple HTTP server:

```bash
cd build-wasm
python3 -m http.server
```

Then open `http://localhost:8000` in your browser.

## 🛠️ Customizing the Build

If you want to customize WASM-specific flags, check the `CMakeLists.txt` for a section like this:

```cmake
if(EMSCRIPTEN)
    message(STATUS "Building for WebAssembly")

    # Add your custom flags here
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -sEXPORTED_FUNCTIONS=['_my_func']")
endif()
```

---

# 📦 Additional Notes

- Emscripten sets up an **in-memory virtual file system (MEMFS)** by default. If your app needs file I/O or other advanced features (like multithreading or SIMD), check Emscripten’s docs for the right flags.
- Make sure your browser supports WebAssembly (all modern browsers do).
- If you're integrating the `.wasm` file into an existing HTML/JS project, you can load it manually via `fetch()` or let Emscripten’s generated `.js` file handle loading for you.
