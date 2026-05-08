/* AUTO-GENERATED FILE. DO NOT EDIT! */

/* /home/ronny/repos/nd100x/src/frontend/nd100x/config.c */
void Config_Init(Config_t *config);
bool Config_ParseCommandLine(Config_t *config, int argc, char *argv[]);
void Config_PrintHelp(const char *progName);

/* /home/ronny/repos/nd100x/src/frontend/nd100x/nd100x.c */
void handle_sigint(int sig);
void register_signals(void);
void dump_stats(void);
void initialize(void);
void cleanup(void);
int main(int argc, char *argv[]);

/* /home/ronny/repos/nd100x/src/frontend/nd100x/menu.c */
int show_floppy_menu(void);
