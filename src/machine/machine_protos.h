/* AUTO-GENERATED FILE. DO NOT EDIT! */

/* /home/ronny/repos/nd100x/src/machine/machine.c */
void init_drive_arrays(void);
void machine_init(bool debuggerEnabled);
void cleanup_machine(void);
void machine_run(int ticks);
void machine_stop(void);
void setdefaultconfig(void);
void write_memory(uint16_t address, uint16_t value);
void program_load(BOOT_TYPE bootType, const char *imageFile, bool verbose);
void mount_drive(DRIVE_TYPE drive_type, int unit, const char *md5, const char *name, const char *description, const char *image_path);
void unmount_drive(DRIVE_TYPE drive_type, int unit);
int read_block(DRIVE_TYPE drive_type, int unit, void *buffer, int block_size);
int write_block(DRIVE_TYPE drive_type, int unit, const void *buffer, int block_size);
MountedDriveInfo_t *list_mount(DRIVE_TYPE drive_type);
int machine_block_read(Device *device, uint8_t *buffer, size_t size, uint32_t blockAddress, int unit);
int machine_block_write(Device *device, const uint8_t *buffer, size_t size, uint32_t blockAddress, int unit);
int machine_block_disk_info(Device *device, size_t *image_size, bool *is_write_protected, int unit);

/* /home/ronny/repos/nd100x/src/machine/io.c */
void IO_Init(void);
void IO_Destroy(void);
uint16_t IO_Read(uint32_t address);
void IO_Write(uint32_t address, uint16_t value);
int IO_Ident(uint16_t level);
void IO_Tick(void);
ushort io_op(ushort ioadd, ushort regA);
