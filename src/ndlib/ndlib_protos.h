/* AUTO-GENERATED FILE. DO NOT EDIT! */

/* /home/ronny/repos/nd100x/src/ndlib/load_bpun.c */
int LoadBPUN(const char *filename, bool verbose);
bool LoadBPUNStream(FILE *bpunStream, BPUN_Header *header);
int bp_load(const char *bpfile);

/* /home/ronny/repos/nd100x/src/ndlib/ndlib.c */
void unsetcbreak(void);
void setcbreak(void);

/* /home/ronny/repos/nd100x/src/ndlib/log.c */
void Log_SetMinLevel(LogLevel level);
void Log(LogLevel level, const char *format, ...);
