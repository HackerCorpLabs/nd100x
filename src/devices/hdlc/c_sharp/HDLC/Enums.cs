using System;

namespace Emulated.HW.ND.CPU.NDBUS.HDLC
{
    /// <summary>
    /// DMA Commands to the ND HDLC interface
    /// </summary>
    public enum DMACommands
    {
        // Initialization
        DEVICE_CLEAR,
        INITIALIZE,

        // Data Transfer
        RECEIVER_START,
        RECEIVER_CONTINUE,
        TRANSMITTER_START,

        // Maintenance
        DUMP_DATA_MODULE,
        DUMP_REGISTERS,
        LOAD_REGISTERS,
    }

    /// <summary>
    /// TH1 - baud rate selection thumbwheel
    /// </summary>
    public enum TH1Wheel
    {
        /// <summary>
        /// TW0 = 307.2 kbps
        /// </summary>
        TW0_307200,

        /// <summary>
        /// Invalid choice 1
        /// </summary>
        TW1_invalid,

        /// <summary>
        /// Invalid choice 2
        /// </summary>
        TW2_invalid,

        /// <summary>
        /// TW3 - 1.2 kbps
        /// </summary>
        TW3_1200,

        /// <summary>
        /// Invalid choice 4
        /// </summary>
        TW4_invalid,

        /// <summary>
        /// Invalid choice 5
        /// </summary>
        TW5_invalid,

        /// <summary>
        /// 6 - 9.6 kbps
        /// </summary>
        TW6_9600,

        /// <summary>
        /// 7 - 38.4 kbps
        /// </summary>
        TW7_38400,

        /// <summary>
        /// 8 - 153.6 kbps
        /// </summary>
        TW8_153600,

        /// <summary>
        /// 9 - 76.8 kbps
        /// </summary>
        TW9_76800,

        /// <summary>
        /// Invalid choice 10
        /// </summary>
        TW10_invalid,

        /// <summary>
        /// 11 - 19.2 kbps
        /// </summary>
        TW11_19200,

        /// <summary>
        /// Invalid choice 12
        /// </summary>
        TW12_invalid,

        /// <summary>
        /// 13 - 4.8 kbps
        /// </summary>
        TW13_4800,

        /// <summary>
        /// 14 - 2.4 kbps
        /// </summary>
        TW14_2400
    }

    /// <summary>
    /// DMA receive operation status results
    /// </summary>
    public enum DmaReceiveStatus
    {
        OK,
        FAILED,
        BUFFER_FULL,
        NO_BUFFER
    }

    /// <summary>
    /// DMA sender state machine states
    /// </summary>
    public enum DmaEngineSenderState
    {
        /// <summary>
        /// DMA transmission is stopped
        /// </summary>
        STOPPED,

        /// <summary>
        /// Block ready to send, waiting for enable flags
        /// </summary>
        BLOCK_READY_TO_SEND,

        /// <summary>
        /// Currently sending a block
        /// </summary>
        SENDING_BLOCK,

        /// <summary>
        /// Frame has been sent
        /// </summary>
        FRAME_SENT
    }

    /// <summary>
    /// DMA block sending states
    /// </summary>
    public enum DmaBlockSendState
    {
        /// <summary>
        /// Not sending data
        /// </summary>
        IDLE,

		/// <summary>
		/// Starting a new block
		/// </summary>
		START_BLOCK,


		/// <summary>
		/// Sending data from buffer
		/// </summary>
		SEND_DATA,

        /// <summary>
        /// Sending frame end
        /// </summary>
        SEND_FRAME_END
    }

    /// <summary>
    /// Receiver Transfer Status Bits (RRTS register, IOX+10)
    ///
    /// The low byte is the receiver transfer status from the data modules.
    /// The high byte is the transfer status from the DMA module, and is not used unless the DMA module is installed.
    /// </summary>
    [Flags]
    public enum RTSBits : ushort
    {
        /***** DATA MODULE *****/

        /// <summary>
        /// Data Available (bit 0) - CRITICAL for packet processing
        ///
        /// Indicates that a character has been assembled and may be read from the Receiver Data Register (RDSRL). Interrupt on level 13 if enabled.
        ///
        /// DMA Mode:
        /// SINTRAN Usage: Primary packet ready indicator in HIINT
        /// Logic: IF A NBIT 0 THEN GO OUT1 (drop packet if clear)
        /// Meaning: Valid packet data is available for processing
        /// Impact: REQUIRED - Packet dropped if this bit is clear
        /// This is the PRIMARY indicator that a packet is ready for processing
        /// </summary>
        DataAvailable = 1 << 0,

        /// <summary>
        /// Status Available
        ///
        /// Indicates that status information is available in the Receiver Status Register (RDSRH). Interrupt on level 13 if enabled.
        /// </summary>
        StatusAvailable = 1 << 1,

        /// <summary>
        /// The receiver has seen the start of a frame, but not the end. This means that the receiver is active within a frame.
        /// </summary>
        ReceiverActive = 1 << 2,

        /// <summary>
        /// Sync FlagReceived
        ///
        /// At least one SYNC character or FLAG has been receiver after the last reading of Receiver Transfer Status or Master Clear/Device Clear.
        /// </summary>
        SyncFlagReceived = 1 << 3,

        /// <summary>
        /// DMA Module Request (bit 4) - Hardware handshaking
        ///
        /// This bit is activated by the DMA module. If the DMA module is installed, this bit may be the reason for an interrupt on level 13 if enabled.
        /// It is, however, always read as 0 because it is cleared at the beginning of IOX GP + 10. If the DMA module caused an interrupt,
        /// the reason for this interrupt is given in the most significant byte of the Transfer Status.
        ///
        /// SINTRAN Usage: Cleared on read, not used in processing logic
        /// Meaning: DMA module requesting attention
        /// Impact: Hardware status only, not checked in packet validation
        ///
        /// Note: ListEmpty (bit 11) always sets this bit when active
        ///
        /// Auto-cleared when RTSR register is read.
        /// </summary>
        DMAModuleRequest = 1 << 4,

        /// <summary>
        /// Signal Detector (SD) - bit 5 - COMPLETELY IGNORED BY SINTRAN
        /// (Latched when reading Input Status Register)
        ///
        /// Status of the Signal Detector (CCITT circuit 109) from the Data Communication Equipment.
        /// A _change_ in the status causes an interrupt on level 13 if enabled.
        ///
        /// SINTRAN Usage: NEVER TESTED - NO REFERENCES IN SINTRAN SOURCE CODE
        /// Logic: No SINTRAN logic references this bit
        /// Meaning: Modem signal status (hardware diagnostic only)
        /// Impact: NONE - Setting or clearing this bit has ZERO effect on packet processing
        ///
        /// CRITICAL INSIGHT: Despite being included in 0x036D "optimal pattern", this bit
        /// is included only for hardware compatibility. SINTRAN completely ignores it.
        /// You can set it to 0 or 1 - makes no difference to SINTRAN packet processing.
        /// </summary>
        SignalDetector = 1 << 5,

        /// <summary>
        /// Data Set Ready/I (DSR) - bit 6 - COMPLETELY IGNORED BY SINTRAN
        /// (Latched when reading Input Status Register)
        ///
        /// Status of the Data Set Ready (CCITT circuit 107) signal (V-24,X-21 BIS) or the I signal (X-21) from the Data Communication Equipment.
        /// A _change_ in the status causes an interrupt on level 13 if enabled.
        ///
        /// SINTRAN Usage: NEVER TESTED - NO REFERENCES IN SINTRAN SOURCE CODE
        /// Logic: No SINTRAN logic references this bit
        /// Meaning: Modem signal status (hardware diagnostic only)
        /// Impact: NONE - Setting or clearing this bit has ZERO effect on packet processing
        ///
        /// CRITICAL INSIGHT: Like SignalDetector, this bit is included in 0x036D pattern
        /// purely for hardware compatibility. SINTRAN completely ignores it for packet processing.
        /// You can set it to 0 or 1 - makes no difference to SINTRAN packet processing.
        /// </summary>
        DataSetReady = 1 << 6,

        /// <summary>
        /// Ring Indicator (RI)
        /// (Latched when reading Input Status Register)
        ///
        /// Status of the Ring Indicator (CCITT circuit 125) from the Data Communication.
        /// A _change_ in the status causes an interrupt on level 13 if enabled.
        /// </summary>
        RingIndicator = 1 << 7,

        /***** DMA MODULE *****/
        // Note: Bits 8-15 are cleared when reading the Receiver Transfer Status

        /// <summary>
        /// Block End Status bit from DMA module (bit 8) - IGNORED BY SINTRAN FOR FLOW CONTROL
        ///
        /// Hardware status indicating DMA block processing complete. This bit is set by hardware
        /// to indicate that a DMA block transfer has completed.
        ///
        /// SINTRAN Usage: NEVER TESTED FOR FLOW CONTROL - IGNORED IN PACKET PROCESSING
        /// Logic: SINTRAN never tests this bit for block completion decisions
        /// Meaning: Hardware status only - NOT used for packet validation or processing control
        /// Impact: Required for IRQ generation but IGNORED for all processing logic
        ///
        /// CRITICAL INSIGHT: SINTRAN tests XBLDN (bit 3 in LKEY DMA descriptor) instead of this
        /// RRTS bit for actual block completion control. This bit serves only to trigger interrupts
        /// when BlockEndIE is enabled, but SINTRAN processing logic completely ignores it.
        ///
        /// Auto-cleared when RTSR register is read.
        /// </summary>
        BlockEnd = 1 << 8,

        /// <summary>
        /// Frame End Status bit from DMA module (bit 9) - IGNORED BY SINTRAN FOR FLOW CONTROL
        ///
        /// Hardware status indicating DMA frame processing complete. This bit is set by hardware
        /// to indicate that an HDLC frame transfer has completed.
        ///
        /// SINTRAN Usage: NEVER TESTED FOR FLOW CONTROL - IGNORED IN PACKET PROCESSING
        /// Logic: SINTRAN never tests this bit for frame completion decisions
        /// Meaning: Hardware status only - NOT used for packet validation or processing control
        /// Impact: Required for IRQ generation but IGNORED for all processing logic
        ///
        /// CRITICAL INSIGHT: Like BlockEnd, this bit is purely for hardware interrupt generation.
        /// SINTRAN processing logic completely ignores it and relies on LKEY descriptor flags
        /// for actual flow control decisions.
        ///
        /// Auto-cleared when RTSR register is read.
        /// </summary>
        FrameEnd = 1 << 9,

        /// <summary>
        /// List End Status bit from DMA module (bit 10).
        ///
        /// SINTRAN Usage: NOT directly referenced in receiver processing
        /// Meaning: Indicates end of DMA descriptor list
        /// Impact: DMA status only, cleared on read - NOT used for packet validation
        /// Do NOT set for normal packet reception
        /// </summary>
        ListEnd = 1 << 10,

        /// <summary>
        /// List Empty Status bit from DMA module.
        ///
        /// If the DMA processor reaches the NEW LIST POINTER and then Full Receiver Block, a LIST EMPTY interrupt is generated
        ///
        ///
        /// List Empty Status bit from DMA module (bit 11).
        /// Note that List Empty (Receiver Transfer Status, Bit 11) always gives a DMA Module Request (Bit 4).
        ///
        /// SINTRAN Usage: CRITICAL - EMTY test causes IMMEDIATE receiver shutdown
        /// Logic: IF HASTA/"EMTY" >< 0 THEN 0=:ACTSW (force device inactive)
        /// Meaning: NO receive buffers available in DMA list
        /// Impact: FATAL CONDITION - Forces receiver shutdown, stops all processing
        /// WARNING: Setting this bit SHUTS DOWN the entire receiver until manual restart
        /// Only set if emulating system buffer exhaustion - NOT for normal packets
        /// Note: List Empty always gives a DMA Module Request (Bit 4) when set
        /// </summary>
        ListEmpty = 1 << 11,

        /// <summary>
        /// Unused ?
        /// </summary>
        Undefined12 = 1 << 12,

        // According to SINTRAN Source code Bit 13 and 14 is used for X21 error codes (address 104450), HX21M=060000

        /// <summary>
        /// X21D - X.21 Data Indication Error (bit 13)
        ///
        /// BIT 13 set to 1 means that the transfer just completed is in error.
        ///
        /// SINTRAN Usage: Part of HX21M mask (0x6000) for X.21 error detection
        /// Logic: IF A/\ HX21M >< 0 THEN CALL X21ERR (triggers X.21 error handling)
        /// Meaning: X.21 protocol data indication problem
        /// Impact: Triggers protocol error handling, may terminate connection
        ///
        /// Do NOT set for normal packet reception
        /// </summary>
        X21D = 1 << 13,

        /// <summary>
        /// X21S - X.21 Call Setup/Clear Indication Error (bit 14)
        ///
        /// SINTRAN Usage: Part of HX21M mask (0x6000) for X.21 error detection
        /// Logic: IF A/\ HX21M >< 0 THEN CALL X21ERR (triggers X.21 error handling)
        /// Meaning: X.21 protocol call setup/clear indication problem
        /// Impact: Triggers connection termination procedures
        /// NOTE: This is NOT just "X.21 Clear" - indicates setup/clear problems
        /// Do NOT set for normal packet reception
        ///
        /// BIT 14 set to 1 means that the connection is broken due to DCE clearing.
        /// CLEAR INDICATION is a control signal sent from the DCE to the DTE to indicate that a previously established call (connection) is being terminated or disconnected.
        ///
        /// RetroCore: Set this when TCP connection is closed by remote side
        /// </summary>
        X21S = 1 << 14,

        /// <summary>
        /// Receiver Overrun (bit 15) - Buffer overrun condition
        ///
        /// SINTRAN Usage: Not directly referenced in receiver interrupt processing
        /// Meaning: Receiver buffer overrun occurred
        /// Impact: Not checked in main HIINT packet validation logic
        /// </summary>
        ReceiverOverrun = 1 << 15,

        /// <summary>
        /// Used for clearing DMA info after read
        /// </summary>
        DMA_CLEAR_BITS = BlockEnd | FrameEnd | ListEnd | ListEmpty | Undefined12
    }

	
		/// <summary>
	/// Receiver Transfer Control Bits (Written to by WRTC)
	///
	/// The low byte is for interrupt and data enabling on the data module and also some Data Communication Equipment control signals.
	/// The high byte is for DMA module control signal.
	/// </summary>
	[Flags]
	internal enum RTCBits : int
	{
		/// <summary>
		/// Data Available Interrupt Enable
		/// (Bit 0)
		/// 
		/// A 1 in this bit together with Data Available (RXDA) will cause an interrupt on level 13.
		///
		/// The bit is cleared by a servicing IDENT, by MASTER CLEAR and by DEVICE CLEAR.
		/// </summary>
		DataAvailableIE = 1 << 0,

		/// <summary>
		/// Status Available lnterrupt Enable
		/// (Bit 1)
		/// 
		/// A 1 in this bit together with Status Available (RXSA) will cause an interrupt on level 13.
		///
		/// The bit is cleared by a servicing IDENT, by MASTER CLEAR and by DEVICE CLEAR.
		/// </summary>
		StatusAvailableIE = 1 << 1,

		/// <summary>
		/// Enable Receiver (RXE)
		/// (Bit 2)
		/// 
		/// Incoming serial data stream is enabled into the receiver.
		///
		/// The bit is cleared by MASTER CLEAR
		/// </summary>
		EnableReceiver = 1 << 2,

		/// <summary>
		/// Enable Receiver DMA
		/// (Bit 3)
		/// 
		/// With a 1 in this bit, Data Available (RXDA) will cause a request to the DMA module.
		/// 
		/// The bit is cleared by MASTER CLEAR and by a "List Empty" key during DMA operation.
		/// </summary>
		EnableReceiverDMA = 1 << 3,

		/// <summary>
		/// DMA Module Interrupt Enable
		///(Bit 4)
		///
		/// A 1 in this bit together with a request from the DMA module will cause an interrupt on level 13.
		/// 
		/// The bit is cleared by a servicing IDENT, by MASTER CLEAR and by DEVICE CLEAR.
		///
		/// Note that List Empty (Receiver Transfer Status, Bit 11) always gives a DMA Module Request (Bit 4).
		/// </summary>
		DMAModuleIE = 1 << 4,


		/// <summary>
		/// Device Clear / Select Maintenance
		/// (Bit 5)
		///
		/// Writing a 1 into this bit first gives a DEVICE CLEAR, clearing interrupts and interrupt enabling flip-flops,
		/// control signals to the Data Communication Equipment, transmitter control signals,
		/// Data Communication Equipment status latches and the Multi Protocol Communication Controller.
		///
		/// Then it turns the Multi Protocol Communication Controller into maintenance mode, looping
		/// transmitted data back to the received data.
		///
		/// When the interface is in maintenance mode, the DEVICE CLEAR function is disabled.The bit is cleared by MASTER CLEAR.
		/// </summary>
		DeviceClear_SelectMaintenance = 1 << 5,

		/// <summary>
		/// Data Terminal Ready/C (DTR)
		/// (Bit 6)
		///
		/// This bit controls a line to the Data Communication Equipment.
		/// It is the Data Terminal Ready (CCITT circuit 108) signal (V-24, X-21 BIS) or the C signal (X-21).
		/// The bit is cleared by MASTER CLEAR.
		/// </summary>
		DTR = 1 << 6,

		/// <summary>
		/// Modem Status Change Interrupt Enable
		/// (Bit 7)
		///
		/// When set, this bit will cause an interrupt on level 13 when one or more of the Data Communication Equipment status signals
		/// connected to the receiver changed to a state different from the last reading (SD, DS/I, RI).
		///
		/// The bit is cleared by servicing IDENT, by MASTER CLEAR and DEVICE CLEAR.
		/// </summary>
		ModemStatusChangeIE = 1 << 7,

		/**** DMA MODULE ONLY ****/

		/// <summary>
		/// Block End Interrupt Enable
		/// (Bit 8)
		///
		/// This bit will, together with Block End and DMA Module Interrupt Enable, cause an interrupt on level 13.
		/// </summary>
		BlockEndIE = 1 << 8,

		/// <summary>
		/// Frame End Interrupt Enable
		/// (Bit 9)
		///
		/// This bit will, together with FrameEnd and DMA Module Interrupt Enable, cause an interrupt on level 13.
		/// </summary>
		FrameEndIE = 1 << 9,

		/// <summary>
		/// List End Interrupt Enable
		/// (Bit 10)
		///
		/// This bit will, together with ListEnd and DMA Module Interrupt Enable, cause an interrupt on level 13.
		/// </summary>
		ListEndIE = 1 << 10,


		/// <summary>
		/// Bit15
		///
		/// Always 1 after IOX + 11 if inspected after a DUMP command (M11)
		/// </summary>
		Bit15 = 1 << 15,

		MASK_CLEAR_IDENT = ~(DataAvailableIE | StatusAvailableIE | DMAModuleIE | ModemStatusChangeIE),
		MASK_CLEAR_DEVICE_CLEAR = ~(DataAvailableIE | StatusAvailableIE | DMAModuleIE | ModemStatusChangeIE),


	}

	/// <summary>
	/// Read Transmitter Transfer Status Flags (RTTS)
	/// (IOX+12) 
	/// 
	/// The low byte is the transmitter transfer status from the data module.
	/// The high byte is the transfer status from the DMA module if installed.
	/// </summary>
	[Flags]
	internal enum TTSBits : int
	{
		/// <summary>
		/// TXBE
		/// Transmit Buffer Empty
		/// (Bit 0)
		///
		/// Indicates that the Transmit Buffer (TDSRL) may be loaded with a new character.
		/// Interrupt on level 12 if enabled.
		/// </summary>
		TransmitBufferEmpty = 1 << 0,

		/// <summary>
		/// TXU
		/// Transmitter Underrun
		/// (Bit 1)
		///
		/// Indicates that the Transmit Buffer has not been loaded with a new character in time.
		/// The transmitter will act as defined by the IOX GP + 1 instruction(PCSARH).
		/// The underrun condition may cause an interrupt on level 12 if enabled.
		/// Transmitter Underrun may be cleared by Master Clear, Device Clear or Transmit Start of Message(TSOM) command.
		/// </summary>
		TransmitterUnderrun = 1 << 1,


		/// <summary>
		/// TXA
		/// Transmitter Active
		/// (Bit 2)
		///
		/// This bit is turned on by sending Start of Message. It will go off when Transmitter Enable (TXE) is turned off and
		/// the characters or sequences already in the transmitter are shifted out on the Transmit Data Line (TSO)
		/// </summary>
		TransmitterActive = 1 << 2,

		// Bit 3 not used

		/// <summary>
		/// DMA RQ
		/// DMA Module Request
		/// (Bit 4)
		///
		/// This bit is activated by the DMA module, and thus it has no meaning unless the DMA module is installed.
		/// It is, however, always read as 0 because it is cleared at the beginning of IOX GP + 12.
		/// If the DMA module is installed, additional information is given in the high byte.
		///
		/// DMA Module Request causes an interrupt on level 12 if enabled.
		///
		/// Note: Note that Transmission Finished (Transmitter Transfer Status, bit 11) always gives a DMA Module Request (bit 4).
		/// </summary>
		DMAModuleRequest = 1 << 4,

		// Bit 5 not used

		/// <summary>
		/// RFS
		/// Ready for Sending (RFS)
		/// (Bit 6)
		///
		/// Status signal from the Data Communication Equipment (CCITT circuit 106).
		/// A change in the status causes an interrupt on level 12 if enabled.
		/// 
		/// (Input signal to HDLC card)
		/// </summary>
		ReadyForSending = 1 << 6,

		// Bit 7 not used 		

		/// <summary>
		/// BE
		/// Block End status Bit from DMA module
		/// (bit 8)
		///
		/// Set by hardware after each individual DMA buffer has been completely transmitted to the COM5025 chip.
		/// For multi-buffer frames, this indicates progression through the buffer chain - can be set multiple times
		/// per frame as each buffer completes. Auto-cleared when RTTS register is read.
		///
		/// Usage: Allows tracking of DMA buffer-by-buffer progression, especially important for large frames
		/// spanning multiple descriptors where BlockEnd=1 with FrameEnd=0 means "more blocks coming".
		///
		/// Auto-cleared when RTTS register is read.
		/// </summary>
		BlockEnd = 1 << 8,


		/// <summary>
		/// FE
		/// Frame End status Bit from DMA module
		/// (bit 9)
		///
		/// Set by hardware when a complete HDLC frame transmission finishes (COM5025 generates closing FLAG + FCS).
		/// Only set when processing a buffer descriptor with TEOM (Transmit End of Message) bit set in LKEY field.
		/// For multi-buffer frames, this is set only on the final buffer, not intermediate buffers.
		///
		/// Usage: Indicates protocol-level frame completion, triggers frame statistics updates in SINTRAN.
		/// Critical for distinguishing between buffer completion (BlockEnd) and logical frame completion.
		/// 
		/// Auto-cleared when RTTS register is read.
		/// </summary>
		FrameEnd = 1 << 9,


		/// <summary>
		/// LE
		/// List End status Bit from DMA module
		/// (bit 10)
		///
		/// Set by hardware after all DMA descriptors in the current descriptor list have been processed.
		/// Triggered when DMA encounters a descriptor with KeyValue = 0x0000 (end marker) or list termination.
		/// Always accompanied by TransmissionFinished bit being set.
		///
		/// Usage: SINTRAN checks this with "BSKP ONE 10 DA" instruction for buffer management and flow control.
		/// Indicates need for new descriptor list setup if more data requires transmission. Critical for
		/// multi-frame operations where current list is exhausted.
		///
		/// Auto-cleared when RTTS register is read.
		/// </summary>
		ListEnd = 1 << 10,


		/// <summary>
		/// TRFIN
		/// Transmission Finished status bit from the DMA module.
		/// (bit 11)
		///
		/// Set by hardware when complete transmission sequence is done - all descriptors processed AND
		/// final frame transmitted. Indicates entire DMA operation complete with no more data to process.
		///
		/// CRITICAL HARDWARE BEHAVIOR: TransmissionFinished (bit 11) ALWAYS automatically sets
		/// DMA Module Request (bit 4), which triggers Level 12 interrupt in SINTRAN.
		///
		/// Usage: Indicates completion of DMA processing (not necessarily success - errors are separate).
		/// Can be set even if individual frames had errors. Used by SINTRAN for buffer management flow
		/// control but is NOT an error condition.
		///
		/// Auto-cleared when RTTS register is read.
		/// </summary>
		TransmissionFinished = 1 << 11,

		// Bits 12 to 14 are undefined for RTTS, but stil autocleared if set
		Undefined12 = 1 << 12,
		Undefined13 = 1 << 13,
		Undefined14 = 1 << 14,

		/// <summary>
		/// ERR
		/// Illegal Key or Illegal Format in Transmitter Buffer Descriptor
		/// (Bit 15)
		///
		/// This status bit indicates an error stop and the transmitter should be restarted.
		/// Note that bit 15 is 1 if inspected after a DUMP command.
		///
		/// Not auto-cleared
		/// </summary>
		Illegal = 1 << 15,

		MASK_CLEAR_DEVICE_CLEAR = ~(TransmitterUnderrun),

		/// <summary>
		/// Used for clearing DMA info after read
		/// </summary>
		DMA_CLEAR_BITS = BlockEnd | FrameEnd | ListEnd | TransmissionFinished | Undefined12 | Undefined13 | Undefined14
	}

	/// <summary>
	/// Transmitter Transfer Control (Written to by WTTC)
	/// IOX+13 WTTC
	///
	/// The low byte is for interrupt and data enabling on the data module and also two signals concerning the connection to the Data Communication Equipment.
	/// The high byte if for the DMA module.
	/// </summary>
	[Flags]
	internal enum TTCBits : int
	{

		/*** DATA MODULE ***/

		/// <summary>
		/// Transmit Buffer Empty Interrupt Enable
		/// (Bit 0)
		///
		/// A 1 in this bit together with Transmit Buffer Empty (TXBE) will cause an interrupt on level 12.
		///
		/// This bit is cleared by a servicing IDENT, by MASTER CLEAR or DEVICE CLEAR.
		/// </summary>
		TransmitBufferEmptyIE = 1 << 0,

		/// <summary>
		/// Transmitter Underrun Interrupt Enabled
		/// (Bit 1)
		/// 
		/// A 1 in this bit together with a Transmitter Underrun condition will cause an interrupt on level 12.
		///
		/// The bit is cleared by a servicing IDENT, by MASTER CLEAR and by DEVICE CLEAR
		/// </summary>
		TransmitterUnderrunIE = 1 << 1,

		/// <summary>
		/// Transmitter Enabled (TXE)
		/// (Bit 2)
		///
		/// A 1 in this bit together with Ready for Sending (RFS) (CCITT circuit 106) enables the transmitter part of the Multi Protocol Communication Control (MPCC) to be 1 (MARK)
		/// and the Transmitter (TXA) to go off when closing flag or last character has been transmitted.
		///
		/// The bit is cleared by MASTER CLEAR and by DEVICE CLEAR.
		/// </summary>
		TransmitterEnabled = 1 << 2,

		/// <summary>
		/// Enable Transmitter DMA
		/// (Bit 3)
		/// With a 1 in this bit, Transmitter Buffer Empty (TXBE) will cause a request to the DMA module.
		///
		/// This bit is cleared by MASTER CLEAR by Transmission Finished or by Illegal Key/Format (DMA operation).
		/// </summary>
		EnableTransmitterDMA = 1 << 3,


		/// <summary>
		/// DMA Module Interrupt Enable
		/// (Bit 4)
		///
		/// A 1 in this bit together with a request from the DMA module will cause an interrupt on level 12.
		///
		/// The bit is cleared by a servicing IDENT, by MASTER CLEAR and by DEVICE CLEAR.
		/// </summary>
		DMAModuleIE = 1 << 4,


		/// <summary>
		/// Half Duplex
		/// (Bit 5)
		///
		/// A 1 in this bit will cause the interface to operate in a half duplex mode.
		/// The request to send (RQTS) (CCITT circuit 1065) signal is not turned ON unless the Signal Detector (SD) (CCITT circuit 109) is off.
		/// A 0 in this bit will cause the interface to operate in a full duplex mode.
		///
		/// The bit is cleared by MASTER CLEAR and by DEVICE CLEAR
		/// </summary>
		HalfDuplex = 1 << 5,

		/// <summary>
		/// Request to Send (RQTS)
		/// (Bit 6)
		///
		/// This is a control signal to the Data Communication Equipment (CCITT circuit 105). (OUT FROM HDLC controller)
		///
		/// In full duplex, 1 means ON and 0 means OFF.
		/// In half duplex, Signal Detector (SD) (CCITT circuit 109) must be OFF before the Request to Send line goes ON.
		///
		/// Normal response from the Data Communication Equipment is to turn Ready for Sending (CCITT circuit 106) ON when Request to Send is ON.
		///
		/// The bit is cleared by MASTER CLEAR and by DEVICE CLEAR.
		/// </summary>
		RequestToSend = 1 << 6,

		/// <summary>
		/// Modem Status Change Interrupt Enable
		/// (Bit 7)
		///
		/// When set, this bit will cause an interrupt on level 12 when Ready for Sending from the Data Communication Equipment changes to a state different from the last reading.
		/// 
		/// The bit is cleared by servicing IDENT, by MASTER CLEAR and by DEVICE CLEAR.
		/// </summary>
		ModemStatusChangeIE = 1 << 7,

		/*** DMA MODULE ***/

		/// <summary>
		/// Block End Interrupt Enable
		/// (Bit 8)
		///
		/// This bit will, together with Block End and DMA Module interrupt Enable, cause an interrupt on level 12.
		/// </summary>
		BlockEndIE = 1 << 8,

		/// <summary>
		/// Frame End Interrupt Enable
		/// (Bit 9)
		///
		/// This bit will, together with Frame End and DMA Module interrupt Enable, cause an interrupt on level 12.
		/// </summary>
		FrameEndIE = 1 << 9,

		/// <summary>		
		/// List End Interrupt Enable
		/// (Bit 10)
		///
		/// This bit will, together with List End and DMA Module Interrupt Enable, cause an interrupt on level 12.
		/// </summary>
		ListEndIE = 1 << 10,

		/// <summary>
		/// Bit 15
		///
		/// Always 1 after IOX GP + 13 if inspected after a DUMP command (M15)
		/// </summary>
		Bit15 = 1 << 15,

		MASK_CLEAR_IDENT = ~(TransmitBufferEmptyIE | TransmitterUnderrunIE | DMAModuleIE | ModemStatusChangeIE),
		MASK_CLEAR_DEVICE_CLEAR = ~(TransmitBufferEmptyIE | TransmitterUnderrunIE | TransmitterEnabled | DMAModuleIE | HalfDuplex | RequestToSend | ModemStatusChangeIE)
	}


	/// <summary>
	/// IOX Registers for the HDLC controller 
	/// </summary>
	internal enum Register
	{
		// IOX 0-6 reads/writes to HDLC chip

		/// <summary>
		/// READ RECEIVER DATA REGISTER (RxDR)
		/// IOX +0
		///
		/// Receiver Data Register is the low byte of the Receiver DataIStatus Register
		/// (RDSRL) as described in the data sheet.An assembled character(byte) is read
		/// from the interface into the A register in the CPU. (Character length is specified by
		/// IOX GP + 4 or indicated by RDSRH (IOX GP + 2.) The received character is right
		/// justified.
		/// </summary>
		ReadRxDR,

		/// <summary>
		/// WRITE PARAMETER CONTROL REGISTER (PCSARH)
		/// IOX +1
		///
		/// This is the high byte (bits 8-15) of the Parameter Control SyncIAddress Register
		/// (PCSARH) described in the data sheet.The register defines protocol, etc. Refer
		/// to the data sheet
		/// </summary>
		WritePCR, // 

		/// <summary>
		/// Read Receiver status register
		/// IOX +2
		///
		/// This is the high byte of the Receive DataIStatus Register (RDSRH) and contains
		/// receiver status information. Bit mapping is described in the data sheet.
		/// </summary>
		ReadReceiverStatusRegister,

		/// <summary>
		/// WRITE SYNC/ADDRESS REGISTER (SAR)
		/// IOX + 3
		///
		/// The Sync/Address Register holds the secondary station address in bit-oriented
		/// procedures or the SYNC character in byte-oriented procedures.it is the lower
		/// byte (Byte Control Procedure) of the Parameter Control Sync/Address Register
		/// (PCSARL). Refer to the data sheet.
		/// </summary>
		WriteSAR,


		/// <summary>			
		/// WRITE CHARACTER LENGTH (CL)  <= WRITE ?? This is READ address
		/// IOX+4
		///
		/// The high byte of the Parameter Control Register (PCRH) is used to specify
		/// character length for receiver (bits 0-2) and transmitter(bits 5-71. At this point there
		/// is a difference between X2652 and Signetics and COM 5025 from SMC Micro
		/// systems. See the data sheet. Equal operation when bits 3 and 4 are 0.
		/// </summary>
		WRITE_CharacterLength,


		/// <summary>
		/// WRITE TRANSMITTER DATA REGISTER
		/// IOX+5
		/// </summary>
		WriteTransmitterDataRegister,

		/// <summary>
		/// TRANSMITTER STATUS REGISTERS (TxSR)
		/// IOX +6
		///
		/// The high byte of the Transmit Data/Status Register (TDSRH) contains
		/// transmitter command and status information. The functions of the different bits
		/// are described in the data sheets.
		/// </summary>
		ReadTransmitterStatusRegister,




		/// <summary>
		/// WRITE TRANSMITTER CONTROL REGISTER (TxCW)
		/// IOX+7
		///
		/// This is the same byte as may be read by IOX+6.
		/// </summary>
		WriteTransmitterControlRegister,


		//* FROM HERE INTERNAL REGISTERS ON ND CONTROLLER */

		/// <summary>
		/// RECEIVER TRANSFER STATUS REGISTER (RRTS)
		/// IOX+10
		/// 
		/// The low byte is the receiver transfer status from the data modules. The high byte
		/// is the transfer status from the DMA module, and is not used unless the DMA
		/// module is installed.
		/// 
		/// </summary>
		ReadReceiverTransferStatus,

		/// <summary>
		/// Write Receiver Transfer Control:
		/// IOX+11
		///
		/// The low byte is for interrupt and data enabling on the data module and also some
		/// Data Communication Equipment control signals. The high byte is for DMA
		/// module control signal.
		/// </summary>
		WriteReceiverTransferControl,

		/// <summary>
		/// Read Transmitter Transfer Status (RTTS)
		/// IOX+12
		/// 
		/// The low byte is the transmitter transfer status from the data module. The high
		/// byte is the transfer status from the DMA module if installed.			
		/// </summary>
		ReadTransmitterTransferStatus,

		/// <summary>
		/// Write Transmitter Transfer Control
		/// IOX+13
		///
		/// The low byte is for interrupt and data enabling on the data module and also two
		/// signals concerning the connection to the Data Communication Equipment. The
		/// high byte if for the DMA module.
		/// </summary>
		WriteTransmitterTransferControlRegister,

		/// <summary>
		/// Read DMA Address
		/// IOX+14
		/// 
		/// The last value written to this register by IOX GP + 15 is read back. May be used for debugging or control.
		/// </summary>
		ReadDMAAddress,

		/// <summary>
		/// Write DMA Address
		/// IOX+15
		/// 
		/// The 16 least significant bits for the first location in a loadldump area or the first
		/// location in a list of buffer descriptors are written into a register (M3) in the DMA
		/// module.
		/// 
		/// </summary>
		WriteDMAAddress,

		/// <summary>
		/// Read DMA Command Register
		/// IOX+16
		/// 
		/// Before a new command is written to the DMA module, this register should be
		/// inspected. If it is zero, the new command sequence can be started.If not, wait until
		/// it becomes zero.A MASTER CLEAR command sequence can, however, be started even if the command register is not zero.
		/// 
		/// </summary>
		ReadDMACommand,

		/// <summary>
		/// Write DMA Command
		/// IOX+17
		///
		/// The two most significant bits of the address for the first location in a load/dump
		/// area or the first location in a list of buffer descriptors are written into a register
		/// (M2) in the DMA module together with a value giving one of 8 commands.The
		/// data format for this instruction is described in the next section.
		/// The HDLC DMA module is partly controlled by 110 instructions, and partly by control
		/// information in buffers in main memory. 110 instructions are used to set buffer
		/// addresses, to start operations (give commands), to enable interrupts and to read
		/// status.
		/// Control information in the memory is used as additional information for the interface
		/// when an operation has been started(by a command).
		/// </summary>
		WriteDMACommand
	}


	/// <summary>
	/// Key Flags for the DMA Data Buffer
	/// </summary>
	[Flags]
	internal enum KeyFlags
	{

		// ------------- RCOST -------------
		// RCOST is identical to Receiver Status Register in the MPCC.

		RCOST_RSOM = 1 << 0,
		RCOST_REOM = 1 << 1,
		RCOST_RX_GAAB = 1 << 2,
		RCOST_RX_OVR = 1 << 3,

		RCOST_RX_ERR = 1 << 7,

		// ------------- DUAL-PURPOSE BIT 3 -------------
		/// <summary>
		/// External Block Done (XBLDN) - bit 3 in LKEY
		///
		/// SINTRAN Usage: Critical for block processing control
		/// Logic: IF A NBIT XBLDN THEN (continue processing)
		/// Meaning: DMA block has been completed and is ready for processing
		/// Impact: ESSENTIAL - SINTRAN uses this for actual flow control
		///
		/// NOTE: This is the SAME bit position as RCOST_RX_OVR (bit 3)
		/// Context determines meaning:
		/// - In RCOST context: RX_OVR = receiver overflow error
		/// - In LKEY control context: XBLDN = external block done
		/// For successful reception: XBLDN=1, which means bit 3 is set
		/// </summary>
		XBLDN = 1 << 3,

		// ------------- KEY INFORMATON -------------
		/// <summary>
		/// Empty Receiver Block
		/// Legal for RECEIVER
		/// </summary>
		EmptyReceiverBlock = 0b010 << 8,

		/// <summary>
		/// Full Receiver Block
		/// Legal for ?????
		/// </summary>
		FullReceiverBlock = 0b011 << 8,

		/// <summary>
		/// Block To be transmitted
		/// Legal for TRANSMITTER
		/// </summary>
		BlockToBeTransmitted = 0b100 << 8,

		/// <summary>
		/// Already transmitted block
		/// Legal for ???
		/// </summary>
		AlreadyTransmittedBlock = 0b101 << 8,

		/// <summary>
		/// New list pointer
		/// Legal for RECEIVER and TRANSMITTER
		/// </summary>
		NewListPointer = 0b110 << 8,


		/// <summary>
		/// Block done bit (both for RX and TX blocks)
		/// </summary>
		BLOCK_DONE_BIT = 0b1 << 8,





		MASK_KEY = 0b111 << 8,
		MASK_DATAFLOW_COST = 0xFF,

	}
}
