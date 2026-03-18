/* AUTO-GENERATED FILE. DO NOT EDIT! */

/* Forward declarations for generated prototypes */
typedef struct PdfDocument PdfDocument;
typedef struct EscpContext EscpContext;
typedef struct EscpSpan EscpSpan;
typedef struct PrintJob PrintJob;
typedef enum PjPrinterType PjPrinterType;
typedef enum PjOutputFormat PjOutputFormat;


/* /home/ronny/repos/nd100x/src/ndlib/load_bpun.c */
int LoadBPUN(const char *filename, bool verbose);
bool LoadBPUNStream(FILE *bpunStream, BPUN_Header *header);
int bp_load(const char *bpfile);

/* /home/ronny/repos/nd100x/src/ndlib/ndlib.c */
void unsetcbreak(void);
void setcbreak(void);

/* /home/ronny/repos/nd100x/src/ndlib/log.c */
void Log_SetMinLevel(LogLevel level);
void Log_SetOutputHandler(LogOutputFunc handler);
void Log(LogLevel level, const char *format, ...);

/* /home/ronny/repos/nd100x/src/ndlib/download.c */
size_t get_downloaded_size(void);
char *download_file(const char *url);
size_t get_downloaded_size(void);
char *download_file(const char *url);

/* /home/ronny/repos/nd100x/src/ndlib/keyboard.c */
int read_key_sequence(char *buf, size_t bufsize);
int is_alt_digit_key(const char *keybuf);
int is_f12_key(const char *keybuf);
int is_escape_key(const char *keybuf);

/* /home/ronny/repos/nd100x/src/ndlib/pdfwriter.c */
PdfDocument *Pdf_Create(void);
int Pdf_AddPage(PdfDocument *doc);
void Pdf_AddTextSpan(PdfDocument *doc, int pageIndex, float x, float y, uint8_t style, float fontSize, const char *text);
bool Pdf_WriteToFile(PdfDocument *doc, const char *filename);
void Pdf_Destroy(PdfDocument *doc);

/* /home/ronny/repos/nd100x/src/ndlib/escp.c */
EscpContext *Escp_Create(void);
void Escp_PutChar(EscpContext *ctx, uint8_t c);
const EscpSpan *Escp_GetSpans(EscpContext *ctx, int *count);
int Escp_GetPageCount(EscpContext *ctx);
void Escp_Reset(EscpContext *ctx);
void Escp_Destroy(EscpContext *ctx);
char Escp_StripToPlainChar(EscpContext *ctx, uint8_t c);

/* /home/ronny/repos/nd100x/src/ndlib/printjob.c */
PrintJob *PrintJob_Create(PjPrinterType printerType, PjOutputFormat format, const char *outputDir);
void PrintJob_PutChar(PrintJob *pj, char c);
bool PrintJob_CheckTimeout(PrintJob *pj);
void PrintJob_Flush(PrintJob *pj);
void PrintJob_Destroy(PrintJob *pj);
