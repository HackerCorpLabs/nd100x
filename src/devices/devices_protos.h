/* AUTO-GENERATED FILE. DO NOT EDIT! */

/* /home/ronny/repos/nd100x/src/devices/device.c */
uint8_t Device_GetOddParity(uint8_t value);
void Device_Init(Device *dev, uint8_t thumbwheel, DeviceClass deviceClass, size_t blockSize);
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
int32_t Device_IO_BufferReadWord(Device *dev, uint8_t *buf, int32_t word_offset);
int32_t Device_IO_WriteWord(Device *dev, FILE *f, uint16_t data);
int32_t Device_IO_BufferWriteWord(Device *dev, uint8_t *buf, int32_t word_offset, uint16_t data);
void Device_DMAWrite(uint32_t coreAddress, uint16_t data);
int32_t Device_DMARead(uint32_t coreAddress);
void Device_SetCharacterOutput(Device *dev, CharacterDeviceOutputFunc outputFunc);
void Device_SetCharacterInput(Device *dev, CharacterDeviceInputFunc inputFunc);
void Device_OutputCharacter(Device *dev, char c);
void Device_InputCharacter(Device *dev, char c);
void Device_SetBlockRead(Device *dev, BlockDeviceReadFunc readFunc, void *userData);
void Device_SetBlockWrite(Device *dev, BlockDeviceWriteFunc writeFunc, void *userData);
void Device_SetBlockDiskInfo(Device *dev, BlockDeviceDiskInfoFunc infoFunc, void *userData);
int Device_ReadBlock(Device *dev, uint8_t *buffer, size_t size, uint32_t blockAddress, int unit);
int Device_WriteBlock(Device *dev, const uint8_t *buffer, size_t size, uint32_t blockAddress, int unit);

/* /home/ronny/repos/nd100x/src/devices/devicemanager.c */
void DeviceManager_Init(LogLevel level);
void DeviceManager_Destroy(void);
void DeviceManager_AddAllDevices(void);
bool DeviceManager_AddHDLCDevice(void);
bool DeviceManager_AddHDLCDevice_WithConfig(bool isServer, const char *address, int port);
void DeviceManager_MasterClear(void);
bool DeviceManager_AddDevice(DeviceType type, uint8_t thumbwheel);
uint16_t DeviceManager_Read(uint32_t address);
void DeviceManager_Write(uint32_t address, uint16_t value);
int DeviceManager_Ident(uint16_t level);
uint16_t DeviceManager_Tick(void);
Device *DeviceManager_GetDeviceByAddress(uint32_t address);
int DeviceManager_Boot(uint16_t device_id);

/* /home/ronny/repos/nd100x/src/devices/panel/panel.c */
void setup_pap(void);
void ProcessMessageControl(PANC_Register panc);
void ProcessTerminalPanc(void);
void ProcessTerminalLamp(void);
void UpdateMachineTime(void);
void panel_thread(void);
void panel_event(void);
void panel_processor_thread(void);

/* /home/ronny/repos/nd100x/src/devices/rtc/deviceRTC.c */
Device *CreateRTCDevice(uint8_t thumbwheel);

/* /home/ronny/repos/nd100x/src/devices/terminal/deviceTerminal.c */
void Terminal_QueueKeyCode(Device *self, uint8_t keycode);
Device *CreateTerminalDevice(uint8_t thumbwheel);

/* /home/ronny/repos/nd100x/src/devices/papertape/devicePapertape.c */
Device *CreatePaperTapeDevice(uint8_t thumbwheel);

/* /home/ronny/repos/nd100x/src/devices/floppy/deviceFloppyDMA.c */
Device *CreateFloppyDMADevice(uint8_t thumbwheel);

/* /home/ronny/repos/nd100x/src/devices/floppy/deviceFloppyPIO.c */
void FloppyPIO_ExecuteGo(Device *self, FloppyPIOCommand command);
Device *CreateFloppyPIODevice(uint8_t thumbwheel);

/* /home/ronny/repos/nd100x/src/devices/smd/deviceSMD.c */
Device *CreateSMDDevice(uint8_t thumbwheel);
void SMD_Destroy(Device *dev);

/* /home/ronny/repos/nd100x/src/devices/smd/diskSMD.c */
void DiskSMD_SetDiskType(DiskInfo *disk, DiskType dt);

/* /home/ronny/repos/nd100x/src/devices/hdlc/deviceHDLC.c */
Device *CreateHDLCDevice(uint8_t thumbwheel);

/* /home/ronny/repos/nd100x/src/devices/hdlc/chipCOM5025.c */
void COM5025_Init(COM5025State *chip);
void COM5025_Reset(COM5025State *chip);
void COM5025_MasterReset(COM5025State *chip);
uint8_t COM5025_ReadByte(COM5025State *chip, COM5025RegistersByte reg);
void COM5025_WriteByte(COM5025State *chip, COM5025RegistersByte reg, uint8_t value);
uint16_t COM5025_ReadWord(COM5025State *chip, COM5025RegistersWord reg);
void COM5025_WriteWord(COM5025State *chip, COM5025RegistersWord reg, uint16_t value);
void COM5025_SetInputPin(COM5025State *chip, COM5025SignalPinIn pin, bool value);
bool COM5025_GetInputPin(COM5025State *chip, COM5025SignalPinIn pin);
bool COM5025_GetOutputPin(COM5025State *chip, COM5025SignalPinOut pin);
void COM5025_ClockReceiver(COM5025State *chip);
void COM5025_ClockTransmitter(COM5025State *chip);
void COM5025_ProcessBit(COM5025State *chip, bool bit);
void COM5025_ReceiveData(COM5025State *chip, const uint8_t *data, int length);
void COM5025_TransmitData(COM5025State *chip, uint8_t data);

/* /home/ronny/repos/nd100x/src/devices/hdlc/chipCOM5025Registers.c */
void COM5025Registers_Init(COM5025Registers *regs);
void COM5025Registers_Clear(COM5025Registers *regs);
void COM5025Registers_Destroy(COM5025Registers *regs);
void COM5025Registers_SetReceiverStatus(COM5025Registers *regs, uint16_t status);
uint8_t COM5025Registers_GetReceiverCharacterLen(COM5025Registers *regs);
uint8_t COM5025Registers_GetTransmitterCharacterLen(COM5025Registers *regs);
void COM5025Registers_SetModeControl(COM5025Registers *regs, uint16_t modeControl);
bool COM5025Registers_IsProtocolModeCCP(COM5025Registers *regs);
void COM5025Registers_SetClockSpeed(COM5025Registers *regs, int speed);
void COM5025Registers_Clock(COM5025Registers *regs);
void COM5025Registers_AdjustTimer(COM5025Registers *regs, int ticks, int param, COM5025TimerFlags timer);
bool COM5025Registers_QueueReceivedData(COM5025Registers *regs, uint16_t data);
bool COM5025Registers_DataReceived(COM5025Registers *regs);
bool COM5025Registers_IsNextByteSync(COM5025Registers *regs);
void COM5025Registers_MarkDataAsReceived(COM5025Registers *regs);
uint16_t COM5025Registers_CalcCRC(COM5025Registers *regs, uint16_t crc, uint8_t data);
void COM5025Registers_CalcRXCrc(COM5025Registers *regs, uint8_t data);
void COM5025Registers_ClearRXCRC(COM5025Registers *regs);
bool COM5025Registers_IsRxCrcEqual(COM5025Registers *regs, uint16_t crc);
void COM5025Registers_AggregateTXCrc(COM5025Registers *regs, uint8_t data);
void COM5025Registers_ClearTXCRC(COM5025Registers *regs);
uint16_t COM5025Registers_CalcFinalTxCrc(COM5025Registers *regs);
bool COM5025Registers_IsTxCrcEqual(COM5025Registers *regs, uint16_t crc);
void COM5025IOTimer_Init(COM5025IOTimer *timer);
void COM5025IOTimer_Clear(COM5025IOTimer *timer);
void COM5025IOTimer_SetCallback(COM5025IOTimer *timer, void (*callback )(void *context, int param ), void *context);
void COM5025IOTimer_SetClockSpeed(COM5025IOTimer *timer, int speed);
void COM5025IOTimer_AdjustTimer(COM5025IOTimer *timer, int ticks, int param, bool enable);
void COM5025IOTimer_Clock(COM5025IOTimer *timer);

/* /home/ronny/repos/nd100x/src/devices/hdlc/hdlc_crc.c */
uint16_t HDLC_CRC_CalculateCRC16Buffer(uint16_t crc, const uint8_t *buf, int length);
uint16_t HDLC_CRC_CalcCrc16(uint16_t crc, uint8_t byte);
uint16_t HDLC_CRC_CalcCCITT(uint16_t fcs, uint8_t byte);
uint8_t HDLC_CRC_CalculateParityBit(uint8_t data, HDLCParityMode mode);
uint8_t HDLC_CRC_AddParityBit(uint8_t data, HDLCParityMode mode);
bool HDLC_CRC_CheckParity(uint8_t data, HDLCParityMode mode);
