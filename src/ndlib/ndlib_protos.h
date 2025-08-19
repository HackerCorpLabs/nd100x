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

/* /home/ronny/repos/nd100x/src/ndlib/download.c */
size_t get_downloaded_size(void);
char *download_file(const char *url);

/* /home/ronny/repos/nd100x/src/ndlib/keyboard.c */
int read_key_sequence(char *buf, size_t bufsize);
int is_f12_key(const char *keybuf);
int is_escape_key(const char *keybuf);
