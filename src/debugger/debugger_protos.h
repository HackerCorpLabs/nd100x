/* AUTO-GENERATED FILE. DO NOT EDIT! */

/* /home/ronny/repos/nd100x/src/debugger/debugger.c */
int debugger_thread_win(LPVOID lpParam);
void start_debugger(void);
void ndx_server_terminate(int sig);
void stop_debugger_thread(void);
const char *cpuStopReasonToString(CpuStopReason r);
int32_t find_stack_return_address(void);
void debugger_update_jpl_entrypoint(uint16_t ea);
void debugger_build_stack_trace(uint16_t pc, uint16_t operand);
int step_cpu(DAPServer *server, StepType step_type);
void update_stack_frame(DAPServer *server, int frame_index, int frame_id, uint16_t memory_reference, uint16_t entry_point);
void free_symbol_table(void);
int init_symbol_support(const char *filename, SymbolType symbol_type);
int ndx_server_init(int port);
int ndx_server_stop(void);
void debugger_kbd_input(char c);
const char *find_symbol_by_address(symbol_table_t *symtab, uint16_t address);
const char *get_symbol_for_address(uint16_t address);
const char *get_source_location(uint16_t address, int *line);
int set_default_dap_capabilities(DAPServer *server);
void start_debugger(void);
int ndx_server_init(int port);
int ndx_server_stop(void);
void debugger_kbd_input(char c);
