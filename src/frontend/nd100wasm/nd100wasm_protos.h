
/* /home/ronny/repos/nd100x/src/frontend/nd100wasm/nd100wasm.c */
EMSCRIPTEN_EXPORT void Init(void);
EMSCRIPTEN_EXPORT int SendKeyToTerminal(int terminalId, int keyCode);
EMSCRIPTEN_EXPORT int GetTerminalAddress(int terminalId);
EMSCRIPTEN_EXPORT void Setup(const char *config);
EMSCRIPTEN_EXPORT void Step(int steps);
EMSCRIPTEN_EXPORT void Start(void);
EMSCRIPTEN_EXPORT void Stop(void);
EMSCRIPTEN_EXPORT void SetTerminalOutputCallback(int terminalId, void (*callback )(int terminalId, char c ));
int main(int argc, char *argv[]);
