
/* /home/ronny/repos/nd100x/src/frontend/nd100wasm/nd100wasm.c */
EMSCRIPTEN_EXPORT void TerminalOutputToJS(int identCode, char c);
EMSCRIPTEN_EXPORT void SetJSTerminalOutputHandler(int enable);
EMSCRIPTEN_EXPORT void Init(int boot_smd);
EMSCRIPTEN_EXPORT int SendKeyToTerminal(int identCode, int keyCode);
EMSCRIPTEN_EXPORT int GetTerminalAddress(int terminalId);
EMSCRIPTEN_EXPORT int GetTerminalIdentCode(int terminalId);
EMSCRIPTEN_EXPORT void Setup(const char *config);
EMSCRIPTEN_EXPORT void Step(int steps);
EMSCRIPTEN_EXPORT void Stop(void);
EMSCRIPTEN_EXPORT void SetTerminalOutputCallback(int identCode, void (*callback )(int identCode, char c ));
int main(int argc, char *argv[]);
