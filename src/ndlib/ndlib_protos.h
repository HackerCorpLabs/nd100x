/* AUTO-GENERATED FILE. DO NOT EDIT! */

/* /home/ronny/repos/nd100x/src/ndlib/aout.c */
const char *magic2str(uint16_t magic);
const char *get_symbol_type(uint16_t type);
void add_symbol(const symbol_t *sym);
void load_symbols_with_string_table(FILE *f, long sym_offset, uint32_t num_bytes_syms, bool verbose);
int load_aout(const char *filename, bool verbose);

/* /home/ronny/repos/nd100x/src/ndlib/bpun.c */
int LoadBPUN(const char *filename, bool verbose);
bool LoadBPUNStream(FILE *bpunStream, BPUN_Header *header);
int bp_load(const char *bpfile);

/* /home/ronny/repos/nd100x/src/ndlib/ndlib.c */
void blocksignals(void);
void setsignalhandlers(void);
void unsetcbreak(void);
void setcbreak(void);

/* /home/ronny/repos/nd100x/src/ndlib/log.c */
void Log_SetMinLevel(LogLevel level);
void Log(LogLevel level, const char *format, ...);
