/* AUTO-GENERATED FILE. DO NOT EDIT! */
/* Prototypes from config.c */

/* config.c */
void Config_Init(Config_t *config);
bool Config_ParseCommandLine(Config_t *config, int argc, char *argv[]);
void Config_PrintHelp(const char *progName);

/* Prototypes from nd100x.c */

/* nd100x.c */
void handle_sigint(int sig);
void initialize(void);
void dump_stats(void);
void cleanup(void);
int main(int argc, char *argv[]);

