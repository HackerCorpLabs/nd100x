/* AUTO-GENERATED FILE. DO NOT EDIT! */

/* /home/ronny/repos/nd100x/src/machine/machine.c */
void machine_init(bool debuggerEnabled);
void cleanup_machine(void);
void machine_run(int ticks);
void machine_stop(void);
void setdefaultconfig(void);
void program_load(BOOT_TYPE bootType, char *imageFile, bool verbose);

/* /home/ronny/repos/nd100x/src/machine/io.c */
void IO_Init(void);
void IO_Destroy(void);
uint16_t IO_Read(uint32_t address);
void IO_Write(uint32_t address, uint16_t value);
int IO_Ident(uint16_t level);
void IO_Tick(void);
ushort io_op(ushort ioadd, ushort regA);
