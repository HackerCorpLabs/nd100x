/* AUTO-GENERATED FILE. DO NOT EDIT! */

/* /home/ronny/repos/nd100x/src/debugger/debugger.c */
int debugger_thread_win(LPVOID lpParam);
void start_debugger(void);
void stop_debugger(void);
const char *cpuStopReasonToString(CpuStopReason r);
int step_cpu(DAPServer *server, const char *step_type);
int init_symbol_support(const char *filename);
int register_symbol_file_with_dap(DAPServer *server, const char *symbol_file);
int ndx_server_init(int port);
int ndx_server_stop(void);
void debugger_kbd_input(char c);
const char *find_symbol_by_address(symbol_table_t *symtab, uint16_t address);
const char *get_symbol_for_address(uint16_t address);
const char *get_source_location(uint16_t address, int *line);
void start_debugger(void);
int ndx_server_init(int port);
int ndx_server_stop(void);
void debugger_kbd_input(char c);
int set_default_dap_capabilities(DAPServer *server);
