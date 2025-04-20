/* AUTO-GENERATED FILE. DO NOT EDIT! */
/* Prototypes from device.c */

/* device.c */
uint8_t Device_GetOddParity(uint8_t value);
void Device_Init(Device *dev, uint8_t thumbwheel);
void Device_Destroy(Device *dev);
void Device_Reset(Device *dev);
uint16_t Device_Tick(Device *dev);
int32_t Device_Boot(Device *dev, uint16_t device_id);
bool Device_IsInAddress(Device *dev, uint32_t address);
uint32_t Device_RegisterAddress(Device *dev, uint32_t address);
uint16_t Device_Read(Device *dev, uint32_t address);
void Device_Write(Device *dev, uint32_t address, uint16_t value);
uint16_t Device_Ident(Device *dev, uint16_t level);
void Device_QueueIODelay(Device *dev, uint16_t ticks, IODelayedCallback cb, int param, uint8_t irqlevel);
void Device_TickIODelay(Device *dev);
void Device_ClearInterrupt(Device *dev, uint16_t level);
void Device_GenerateInterrupt(Device *dev, uint16_t level);
void Device_SetInterruptStatus(Device *dev, bool active, uint16_t level);
int32_t Device_IO_Seek(Device *dev, FILE *f, long offset);
int32_t Device_IO_ReadWord(Device *dev, FILE *f);
int32_t Device_IO_WriteWord(Device *dev, FILE *f, uint16_t data);
uint32_t Device_DMAWrite(uint32_t coreAddress, uint16_t data);
int32_t Device_DMARead(uint32_t coreAddress);

/* Prototypes from devicemanager.c */

/* devicemanager.c */
void DeviceManager_Init(LogLevel level);
void DeviceManager_Destroy(void);
void DeviceManager_AddAllDevices(void);
void DeviceManager_MasterClear(void);
bool DeviceManager_AddDevice(DeviceType type, uint8_t thumbwheel);
uint16_t DeviceManager_Read(uint32_t address);
void DeviceManager_Write(uint32_t address, uint16_t value);
int DeviceManager_Ident(uint16_t level);
uint16_t DeviceManager_Tick(void);
uint16_t DeviceManager_Tick_RTC(void);
Device *DeviceManager_GetDeviceByAddress(uint32_t address);
int DeviceManager_Boot(uint16_t device_id);

/* Prototypes from floppy/deviceFloppyPIO.c */

/* floppy/deviceFloppyPIO.c */
void FloppyPIO_ExecuteGo(Device *self, FloppyPIOCommand command);
Device *CreateFloppyPIODevice(uint8_t thumbwheel);

/* Prototypes from floppy/deviceFloppyDMA.c */

/* floppy/deviceFloppyDMA.c */
Device *CreateFloppyDMADevice(uint8_t thumbwheel);

/* Prototypes from papertape/devicePapertape.c */

/* papertape/devicePapertape.c */
Device *CreatePaperTapeDevice(uint8_t thumbwheel);

/* Prototypes from smd/deviceSMD.c */

/* smd/deviceSMD.c */
Device *CreateSMDDevice(uint8_t thumbwheel);

/* Prototypes from smd/diskSMD.c */

/* smd/diskSMD.c */
void DiskSMD_Init(DiskInfo *disk, uint8_t unit, char *diskFileName);
void DiskSMD_SetDiskType(DiskInfo *disk, DiskType dt);

/* Prototypes from rtc/deviceRTC.c */

/* rtc/deviceRTC.c */
Device *CreateRTCDevice(uint8_t thumbwheel);

/* Prototypes from panel/panel.c */

/* panel/panel.c */
void setup_pap(void);
void ProcessMessageControl(PANC_Register panc);
void ProcessTerminalPanc(void);
void ProcessTerminalLamp(void);
void UpdateMachineTime(void);
void panel_thread(void);
void panel_event(void);
void panel_processor_thread(void);

/* Prototypes from terminal/deviceTerminal.c */

/* terminal/deviceTerminal.c */
void Terminal_QueueKeyCode(Device *self, uint8_t keycode);
Device *CreateTerminalDevice(uint8_t thumbwheel);

