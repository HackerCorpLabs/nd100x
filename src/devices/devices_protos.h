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
bool DeviceManager_AddHDLCDevice_WithConfig(int thumbwheel, bool isServer, const char *address, int port);
void DeviceManager_MasterClear(void);
bool DeviceManager_AddDevice(DeviceType type, uint8_t thumbwheel);
uint16_t DeviceManager_Read(uint32_t address);
void DeviceManager_Write(uint32_t address, uint16_t value);
int DeviceManager_Ident(uint16_t level);
uint16_t DeviceManager_Tick(void);
Device *DeviceManager_GetDeviceByAddress(uint32_t address);
int DeviceManager_GetDeviceCount(void);
Device *DeviceManager_GetDeviceByIndex(int index);
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
void PaperTape_LoadTape(Device *self, const uint8_t *data, size_t length);
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

/* /home/ronny/repos/nd100x/src/devices/lineprinter/deviceLinePrinter.c */
Device *CreateLinePrinterDevice(uint8_t thumbwheel);

/* /home/ronny/repos/nd100x/src/devices/papertapewriter/devicePaperTapeWriter.c */
const uint8_t *PaperTapeWriter_GetTapeData(Device *self, size_t *length);
Device *CreatePaperTapeWriterDevice(uint8_t thumbwheel);

/* /home/ronny/repos/nd100x/src/devices/hdlc/deviceHDLC.c */
Device *CreateHDLCDevice(uint8_t thumbwheel);
bool HDLC_GetRxFrameStatus(const HDLCData *data, HDLCRxFrameStatus *status);

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
void COM5025_SetReceiverStatus(COM5025State *chip, uint16_t newRxStatus);
void COM5025_SetTransmitterOutputCallback(COM5025State *chip, void (*callback )(void *context, uint8_t data ), void *context);
void COM5025_SetPinValueChangedCallback(COM5025State *chip, void (*callback )(void *context, COM5025SignalPinOut pin, bool value ), void *context);

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

/* /home/ronny/repos/nd100x/src/devices/hdlc/modem.c */
void Modem_Init(ModemState *modem, Device *hdlcDevice);
void Modem_Destroy(ModemState *modem);
void Modem_StartModem(ModemState *modem, bool isServer, const char *address, int port);
void Modem_Tick(ModemState *modem);
void Modem_SendByte(ModemState *modem, uint8_t data);
void Modem_SendBytes(ModemState *modem, const uint8_t *data, int length);
void Modem_SetDTR(ModemState *modem, bool value);
void Modem_SetRTS(ModemState *modem, bool value);
void Modem_SetDSR(ModemState *modem, bool value);
void Modem_SetCTS(ModemState *modem, bool value);
void Modem_SetReceivedDataCallback(ModemState *modem, ModemDataCallback callback);
void Modem_SetRingIndicatorCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetDataSetReadyCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetSignalDetectorCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetClearToSendCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetRequestToSendCallback(ModemState *modem, ModemSignalCallback callback);
void Modem_SetDataTerminalReadyCallback(ModemState *modem, ModemSignalCallback callback);

/* /home/ronny/repos/nd100x/src/devices/hdlc/dmaEngine.c */
void DMAEngine_Init(DMAEngine *dma, bool burstMode, struct Device *hdlcDevice, void *modem, void *com5025);
void DMAEngine_Destroy(DMAEngine *dma);
void DMAEngine_Clear(DMAEngine *dma);
void DMAEngine_Tick(DMAEngine *dma);
int DMAEngine_DMARead(DMAEngine *dma, uint32_t address);
void DMAEngine_DMAWrite(DMAEngine *dma, uint32_t address, uint16_t data);
void DMAEngine_OnSetInterruptBit(DMAEngine *dma, uint8_t bit);
void DMAEngine_OnWriteDMA(DMAEngine *dma, uint32_t address, uint16_t data);
void DMAEngine_OnReadDMA(DMAEngine *dma, uint32_t address, int *data);
void DMAEngine_ExecuteCommand(DMAEngine *dma);
void DMAEngine_CommandDeviceClear(DMAEngine *dma);
void DMAEngine_CommandInitialize(DMAEngine *dma);
void DMAEngine_CommandReceiverStart(DMAEngine *dma);
void DMAEngine_CommandReceiverContinue(DMAEngine *dma);
void DMAEngine_CommandTransmitterStart(DMAEngine *dma);
void DMAEngine_CommandDumpDataModule(DMAEngine *dma);
void DMAEngine_CommandDumpRegisters(DMAEngine *dma);
void DMAEngine_CommandLoadRegisters(DMAEngine *dma);
void DMAEngine_SetDMAAddress(DMAEngine *dma, uint32_t address);
void DMAEngine_ClearDMACommand(DMAEngine *dma);
uint16_t DMAEngine_GetBufferKeyVault(DMAEngine *dma, uint32_t listPointer, uint16_t offset);
uint32_t DMAEngine_ScanNextTXBuffer(DMAEngine *dma, uint32_t start);
void DMAEngine_SetWriteDMACallback(DMAEngine *dma, DMAWriteCallback callback);
void DMAEngine_SetReadDMACallback(DMAEngine *dma, DMAReadCallback callback);
void DMAEngine_SetInterruptCallback(DMAEngine *dma, DMASetInterruptCallback callback);
void DMAEngine_SetSendFrameCallback(DMAEngine *dma, DMASendFrameCallback callback);
void DMAEngine_SetUpdateReceiverStatusCallback(DMAEngine *dma, DMAUpdateReceiverStatusCallback callback);
void DMAEngine_SetClearCommandCallback(DMAEngine *dma, DMAClearCommandCallback callback);
void DMAEngine_Log(DMAEngine *dma, const char *format, ...);

/* /home/ronny/repos/nd100x/src/devices/hdlc/dmaDCB.c */
void DCB_Init(HdlcDCB *dcb);
void DCB_Clear(HdlcDCB *dcb);
KeyFlags DCB_GetKey(const HdlcDCB *dcb);
bool DCB_HasRSOMFlag(const HdlcDCB *dcb);
bool DCB_HasREOMFlag(const HdlcDCB *dcb);
uint16_t DCB_GetDataFlowCost(const HdlcDCB *dcb);
uint32_t DCB_GetDataMemoryAddress(const HdlcDCB *dcb);
void DCB_SetDataMemoryAddress(HdlcDCB *dcb, uint32_t address);
void DCB_SetBufferAddress(HdlcDCB *dcb, uint32_t address);
uint32_t DCB_GetBufferAddress(const HdlcDCB *dcb);
void DCB_SetOffsetFromLP(HdlcDCB *dcb, uint16_t offset);
uint16_t DCB_GetOffsetFromLP(const HdlcDCB *dcb);
void DCB_SetKeyValue(HdlcDCB *dcb, uint16_t keyValue);
uint16_t DCB_GetKeyValue(const HdlcDCB *dcb);
void DCB_SetByteCount(HdlcDCB *dcb, uint16_t byteCount);
uint16_t DCB_GetByteCount(const HdlcDCB *dcb);
void DCB_SetDisplacement(HdlcDCB *dcb, uint16_t displacement);
uint16_t DCB_GetDisplacement(const HdlcDCB *dcb);
void DCB_SetListPointer(HdlcDCB *dcb, uint32_t listPointer);
uint32_t DCB_GetListPointer(const HdlcDCB *dcb);
void DCB_SetDMAAddress(HdlcDCB *dcb, uint32_t address);
uint32_t DCB_GetDMAAddress(const HdlcDCB *dcb);
void DCB_SetDMABytesRead(HdlcDCB *dcb, int bytesRead);
int DCB_GetDMABytesRead(const HdlcDCB *dcb);
void DCB_SetDMABytesWritten(HdlcDCB *dcb, int bytesWritten);
int DCB_GetDMABytesWritten(const HdlcDCB *dcb);
void DCB_SetDMAReadData(HdlcDCB *dcb, int data);
int DCB_GetDMAReadData(const HdlcDCB *dcb);
bool DCB_IsDMAReadDataValid(const HdlcDCB *dcb);
void DCB_ClearDMAReadData(HdlcDCB *dcb);
void DCB_Print(const HdlcDCB *dcb);

/* /home/ronny/repos/nd100x/src/devices/hdlc/dmaControlBlocks.c */
void DMAControlBlocks_Init(DMAControlBlocks *dcbs, struct Device *hdlcDevice);
void DMAControlBlocks_Destroy(DMAControlBlocks *dmaCB);
void DMAControlBlocks_Clear(DMAControlBlocks *dmaCB);
void DMAControlBlocks_SetTXPointer(DMAControlBlocks *dmaCB, uint32_t listPointer, int offset);
void DMAControlBlocks_DebugTXFrames(DMAControlBlocks *dmaCB);
void DMAControlBlocks_LoadTXBuffer(DMAControlBlocks *dmaCB);
bool DMAControlBlocks_LoadNextTXBuffer(DMAControlBlocks *dmaCB);
void DMAControlBlocks_MarkBufferSent(DMAControlBlocks *dmaCB);
void DMAControlBlocks_SetRXPointer(DMAControlBlocks *dmaCB, uint32_t listPointer, int offset);
void DMAControlBlocks_LoadRXBuffer(DMAControlBlocks *dmaCB);
bool DMAControlBlocks_LoadNextRXBuffer(DMAControlBlocks *dmaCB);
bool DMAControlBlocks_IsNextRXbufValid(DMAControlBlocks *dmaCB);
void DMAControlBlocks_MarkBufferReceived(DMAControlBlocks *dmaCB, uint8_t rxStatus);
HdlcDCB *DMAControlBlocks_LoadBufferDescription(DMAControlBlocks *dmaCB, uint32_t listPointer, uint16_t offset, bool isRX);
uint8_t DMAControlBlocks_ReadNextByteDMA(DMAControlBlocks *dmaCB, bool isRx);
void DMAControlBlocks_WriteNextByteDMA(DMAControlBlocks *dmaCB, uint8_t data, bool isRx);
void DMAControlBlocks_SetReadDMACallback(DMAControlBlocks *dmaCB, DMAControlBlocks_ReadCallback callback, void *context);
void DMAControlBlocks_SetWriteDMACallback(DMAControlBlocks *dmaCB, DMAControlBlocks_WriteCallback callback, void *context);
void DMAControlBlocks_SetSendHDLCFrameCallback(DMAControlBlocks *dmaCB, DMAControlBlocks_SendFrameCallback callback, void *context);
void DMAControlBlocks_SetInterruptCallback(DMAControlBlocks *dmaCB, DMAControlBlocks_InterruptCallback callback, void *context);

/* /home/ronny/repos/nd100x/src/devices/hdlc/dmaParamBuf.c */
void ParameterBuffer_Init(ParameterBuffer *paramBuf);
void ParameterBuffer_Clear(ParameterBuffer *paramBuf);
int ParameterBuffer_GetParameterControlRegister(const ParameterBuffer *paramBuf);
int ParameterBuffer_GetSyncAddressRegister(const ParameterBuffer *paramBuf);
int ParameterBuffer_GetCharacterLength(const ParameterBuffer *paramBuf);
int ParameterBuffer_GetDisplacement1(const ParameterBuffer *paramBuf);
int ParameterBuffer_GetDisplacement2(const ParameterBuffer *paramBuf);
int ParameterBuffer_GetMaxReceiverBlockLength(const ParameterBuffer *paramBuf);
int ParameterBuffer_GetReceiverStatusReg(const ParameterBuffer *paramBuf);
int ParameterBuffer_GetTransmitterStatusReg(const ParameterBuffer *paramBuf);
int ParameterBuffer_GetDmaBankBits(const ParameterBuffer *paramBuf);
void ParameterBuffer_SetParameterControlRegister(ParameterBuffer *paramBuf, int value);
void ParameterBuffer_SetSyncAddressRegister(ParameterBuffer *paramBuf, int value);
void ParameterBuffer_SetCharacterLength(ParameterBuffer *paramBuf, int value);
void ParameterBuffer_SetDisplacement1(ParameterBuffer *paramBuf, int value);
void ParameterBuffer_SetDisplacement2(ParameterBuffer *paramBuf, int value);
void ParameterBuffer_SetMaxReceiverBlockLength(ParameterBuffer *paramBuf, int value);
void ParameterBuffer_SetReceiverStatusReg(ParameterBuffer *paramBuf, int value);
void ParameterBuffer_SetTransmitterStatusReg(ParameterBuffer *paramBuf, int value);
void ParameterBuffer_SetDmaBankBits(ParameterBuffer *paramBuf, int value);
void ParameterBuffer_Print(const ParameterBuffer *paramBuf);

/* /home/ronny/repos/nd100x/src/devices/hdlc/dmaTransmitter.c */
void DMATransmitter_Init(DMATransmitter *transmitter, void *com5025, DMAControlBlocks *dmaCB, struct Device *hdlcDevice);
void DMATransmitter_Destroy(DMATransmitter *transmitter);
void DMATransmitter_Clear(DMATransmitter *transmitter);
void DMATransmitter_Tick(DMATransmitter *transmitter);
void DMATransmitter_SetTXDMAFlag(DMATransmitter *transmitter, uint16_t flag);
bool DMATransmitter_SendAllBuffers(DMATransmitter *transmitter);
void DMATransmitter_SetEngineSenderState(DMATransmitter *transmitter, int state);
void DMATransmitter_SetSenderState(DMATransmitter *transmitter, int senderState);
void DMATransmitter_SetSendFrameCallback(DMATransmitter *transmitter, DMATransmitterSendFrameCallback callback);
void DMATransmitter_SetInterruptCallback(DMATransmitter *transmitter, DMATransmitterSetInterruptCallback callback);

/* /home/ronny/repos/nd100x/src/devices/hdlc/tcpReceiveBuffer.c */
void TcpReceiveBuffer_Init(TcpReceiveBuffer *buf, int capacity);
void TcpReceiveBuffer_Destroy(TcpReceiveBuffer *buf);
int TcpReceiveBuffer_Enqueue(TcpReceiveBuffer *buf, const uint8_t *data, int length);
bool TcpReceiveBuffer_DequeueByte(TcpReceiveBuffer *buf, uint8_t *out);
int TcpReceiveBuffer_Available(TcpReceiveBuffer *buf);
void TcpReceiveBuffer_Clear(TcpReceiveBuffer *buf);

/* /home/ronny/repos/nd100x/src/devices/hdlc/dmaReceiver.c */
void DMAReceiver_Init(DMAReceiver *receiver, void *com5025, DMAControlBlocks *dmaCB, struct Device *hdlcDevice);
void DMAReceiver_Destroy(DMAReceiver *receiver);
void DMAReceiver_Clear(DMAReceiver *receiver);
void DMAReceiver_Tick(DMAReceiver *receiver);
void DMAReceiver_ReceiveDataFromModem(DMAReceiver *receiver, const uint8_t *data, int length);
int DMAReceiver_ProcessBufferedData(DMAReceiver *receiver);
bool DMAReceiver_ProcessCompleteFrame(DMAReceiver *receiver);
void DMAReceiver_ClearReceiveFrameState(DMAReceiver *receiver);
void DMAReceiver_SetReceiverState(DMAReceiver *receiver);
void DMAReceiver_SetInterruptCallback(DMAReceiver *receiver, DMAReceiverSetInterruptCallback callback);
bool DMAReceiver_FindNextReceiveBuffer(DMAReceiver *receiver);
DMAReceiveStatus DMAReceiver_ReceiveDataBufferByte(DMAReceiver *receiver, uint8_t data);
void DMAReceiver_SetRXDMAFlag(DMAReceiver *receiver, uint16_t flag);
void DMAReceiver_EnableHDLCReceiver(DMAReceiver *receiver, bool enable);
