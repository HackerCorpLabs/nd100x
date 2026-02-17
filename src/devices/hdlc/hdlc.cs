#define DEBUG_DETAIL
#define DEBUG_DETAIL_PLUS_DESCRIPTION

#define DMA_DEBUG
#define RX_BLAST_LOGGING

using Emulated.HW.Common.Network;
using Emulated.HW.Common.Telnet;
using Emulated.HW.Memory;
using Emulated.HW.ND.CPU.NDBUS.HDLC;
using Emulated.HW.SMC.COM.COM5025;
using Emulated.Lib;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;


#region Docs
/*

HDLC INTERFACE
==============
 
Used as a synchronous modem controller HDLC INTERFACE includes several features by means of communication standards implemented in hardware.
Used as an intercomputer link, HDLC INTERFACE contains internal timing.

A strap switches from external timing (from modem) to internal timing which allows transfer rates from 2.4 kbps (kilo bits per second) up to 307.2 kbps.

Independent of the use, dependent of the maximum required speed, HDLC could be delivered as two different products
serving the same functions as observed in the external device (modem line, connected computer).

— NDNo. 723: CPU controlled input/output transfer (PIO)
* Maximum speed 19.2 kbps

— ND No. 720: Direct Memory Access (DMA) transfer including a DMA controller
* Maximum speed 307.2 kbps

 

                             FIRST   LAST  IDENT  INT.LEVEL   SINTRAN-III
HARDWARE DEVICE NAME         DEVNO  DEVNO  -CODE 10 11 12 13  LOG. DEVNO

HDLC / MEGALINK          1    1640   1657    150        X  X
HDLC / MEGALINK          2    1660   1677    151        X  X
HDLC / MEGALINK          3    1700   1717    152        X  X
HDLC / MEGALINK          4    1720   1737    153        X  X
HDLC / MEGALINK          5    1740   1757    154        X  X
HDLC / MEGALINK          6    1760   1777    155        X  X
HDLC / MEGALINK          7    2000   2017    200        X  X
HDLC / MEGALINK          8    2020   2037    201        X  X
HDLC / MEGALINK          9    2040   2057    202        X  X
HDLC / MEGALINK         10    2060   2077    203        X  X
HDLC AUTO LOAD                1600   1603

HDLC PIO                 1    1640   1657    150        X  X
HDLC PIO                 2    1660   1677    151        X  X
HDLC PIO                 3    1700   1717    152        X  X
HDLC PIO                 4    1720   1737    153        X  X
HDLC PIO                 5    1740   1757    154        X  X
HDLC PIO                 6    1760   1777    155        X  X
HDLC PIO                 7    2000   2017    200        X  X
HDLC PIO                 8    2020   2037    201        X  X
HDLC PIO                 9    2040   2057    202        X  X
HDLC PIO                10    2060   2077    203        X  X

HDLC REMOTE LOAD         1    1604   1607
HDLC REMOTE LOAD         2    1610   1613
HDLC REMOTE LOAD         3    1614   1617
HDLC REMOTE LOAD         4    1620   1623
HDLC REMOTE LOAD         5    1624   1627
HDLC REMOTE LOAD         6    1630   1633
HDLC REMOTE LOAD         7    1634   1637


From ND-110/CX machine with 2 HDLC cards in, and 3 Megalink cards in

                            FIRST   LAST       I D E N T  C O D E S       LOG.
HARDWARE DEVICE NAME        DEVNO  DEVNO    LEV10  LEV11  LEV12  LEV13    DEVNO
-------------------------------------------------------------------------------
HDLC REMOTE LOAD         1   1604   1607
HDLC REMOTE LOAD         2   1610   1613
HDLC / MEGALINK          1   1640   1657                    150    150     1360 <= PCB 3023 MegaLink (slot 5) - Pri 3
HDLC / MEGALINK          2   1660   1677                    151    151     1362 <= PCB 3015 HDLC     (slot 3) - Pri 1
HDLC / MEGALINK          3   1700   1717                    152    152     1364 <= PCB 3015 HDLC     (slot 4) - Pri 2
HDLC / MEGALINK          4   1720   1737                    153    153     1366 <= PCB 3023 MegaLink (slot 6) - Pri 4
HDLC / MEGALINK          5   1740   1757                    154    154     1370 <= PCB 3023 MegaLink (slot 11) - Pri 5



Logical devices for HDLC is one of: 1360, 1362, 1364, 1366, 1370, and 1372. Max 6 cards.

From SINTRAN Source code
 % 1360              SYNC MODEM FOR HDLC INTERFACE
 % 1360              HDLC-1 INPUT
 % 1361              HDLC-1 OUTPUT

 % 1362              SYNC MODEM FOR HDLC INTERFACE
 % 1362              HDLC-2 INPUT
 % 1363              HDLC-2 OUTPUT

 % 1364              SYNC MODEM FOR HDLC INTERFACE
 % 1364              HDLC-3 INPUT
 % 1365              HDLC-3 OUTPUT

 % 1366              SYNC MODEM FOR HDLC INTERFACE
 % 1366              HDLC-4 INPUT
 % 1367              HDLC-4 OUTPUT

 % 1370              SYNC MODEM FOR HDLC INTERFACE
 % 1370              HDLC-5 INPUT
 % 1371              HDLC-5 OUTPUT

 % 1372              SYNC MODEM FOR HDLC INTERFACE
 % 1372              HDLC-6 INPUT
 % 1373              HDLC-6 OUTPUT
 
 % 1374              X21 LINE 1
 % 1375              X21 LINE 2




	* PCB 3015 - HDLC + Autoload - https://www.ndwiki.org/wiki/3015
	* PCB 3023 - MegaLink - https://www.ndwiki.org/wiki/3023


ND-12.018.01 High Level Data Link Control (HDLC) Interface-Gandalf-OCR.PDF


	The HDLC controller uses a "Multi-Protocol Universal Synchronous Receiver/Transmitter" chip

	* The chip is "COM 5025" from SMC
	* https://datasheetspdf.com/pdf-file/1412060/SMSC/COM5025/1
	* http://sintran.com/norsk-data/library/libother/extern/COM5025.pdf 
	

	It also uses a "Multi-protocol communications controller (MPCC)"
	* SCN2652/SCN68652
    * https://datasheetspdf.com/pdf-file/524405/NXP/SCN2652/1
    * http://sintran.com/norsk-data/library/libother/extern/SCN2652.pdf
    


*/

#endregion Docs


namespace Emulated.HW.ND.CPU.NDBUS
{

	/// <summary>
	/// HDLC Interface 
	/// </summary>
	///
	public class NDBusHDLC : NDBusDeviceBase
	{
		#region Internal variables

		/// <summary>
		/// COM5025 us the HDLC controller chip (Multi-Protocol Universal Synchronous Receiver/Transmitter)
		/// </summary>
		COM5025 com5025 = new COM5025(0);

		/// <summary>
		/// Registers hold all the internal variables for the HDLC controller
		/// </summary>
		Registers regs = new Registers();


		/// <summary>
		/// Modem is used to transmit HDLC frames over TCP.
		/// Also has signaling like DTR and RTS
		/// </summary>
		TelnetModem modem;

		DMAEngine dmaEngine;

		#endregion Internal variables

		#region Event defintions
		public delegate void TransmitterOutput(byte serialOutput);
		public event TransmitterOutput? OnTransmitterOutput;
		#endregion Event defintions

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="thumbwheel"></param>
		/// <param name="proxy"></param>
		/// <param name="maxunits"></param>
		/// <param name="enableDMA"></param>
		/// <exception cref="Exception"></exception>
		public NDBusHDLC(byte thumbwheel, string proxy, byte maxunits = 1, bool enableDMA = true) : base("HDLC", maxunits)
		{
			// HDLC - MEGALINK

			switch (thumbwheel)
			{
				case 1:

					base.NDBusAddressBase = Numeric.ParseIntValue("01640"); // HDLC from  1640-1657
					base.IdentCode = (ushort)Numeric.ParseIntValue("0150");
					break;
				case 2:
					base.NDBusAddressBase = Numeric.ParseIntValue("01660"); // HDLC from  1660-1677
					base.IdentCode = (ushort)Numeric.ParseIntValue("0151");
					break;
				case 3:
					base.NDBusAddressBase = Numeric.ParseIntValue("01700"); // HDLC from  1700-1717
					base.IdentCode = (ushort)Numeric.ParseIntValue("0152");
					break;
				case 4:
					base.NDBusAddressBase = Numeric.ParseIntValue("01720"); // HDLC from  1720-1737
					base.IdentCode = (ushort)Numeric.ParseIntValue("0153");
					break;
				case 5:
					base.NDBusAddressBase = Numeric.ParseIntValue("01740"); // HDLC from  1740-1757
					base.IdentCode = (ushort)Numeric.ParseIntValue("0154");
					break;

				// 6-11 = HASP #1-#6
				default:
					throw new Exception($"Unexpected thumbwheel code {thumbwheel} in {MemoryName}");
			}

			com5025.OnPinValueChanged += Com5025_OnPinValueChanged;
			com5025.OnTransmitterOutput += Com5025_OnTransmitterOutput;

			base.InterruptLevel = 12; // Output channel interrupt 12
									  //	* The output channel on HDCL DATA is connected to level 12
									  //	* Normally output is connected to level 10
									  //	* This is done to reduce the possibility of transmitter underrun at high transfer rates
									  //
									  //
									  // Input interrupt is 13
									  //	* The input interrupt is high to make sure we prioritize reading the HDLC data buffers before they are lost


			base.NDBusAddressLength = 16;
			regs.DMAEnabled = enableDMA;

			if (enableDMA)
				base.SetMemoryName($"HDLC DMA {Numeric.Num2Str(thumbwheel, trim: true)}");
			else
				base.SetMemoryName($"HDLC PIO {Numeric.Num2Str(thumbwheel, trim: true)}");


			modem = new TelnetModem();

			// Identify how the modem should work
			if (!string.IsNullOrEmpty(proxy))
			{
				var config = proxy.Split(';');

				if (config.Length >= 3)
				{
					bool serverMode = string.Equals(config[0], "SERVER", StringComparison.OrdinalIgnoreCase);
					bool clientMode = string.Equals(config[0], "CLIENT", StringComparison.OrdinalIgnoreCase);



					int port;
					bool validPort = int.TryParse(config[2], out port);

					if ((validPort) && (clientMode || serverMode))
					{
						if (clientMode)
						{
							// Client
							Logger.Dbg($"HDLC: Starting in client-mode. host[{config[1]}] port[{port}]");
							modem.StartModem(false, config[1], port);
						}

						else if (serverMode)
						{
							//Server
							Logger.Dbg($"HDLC: Starting in server-mode. host[{config[1]}] port[{port}]");
							modem.StartModem(true, config[1], port);
						}
					}
					else
					{
						Logger.Dbg($"HDLC: Invalid Telnet mode [{config[0]}] or port [{config[2]}]");
					}
				}
			}

			modem.OnReceivedData += Modem_OnReceivedData;
			modem.OnRingIndicator += Modem_OnRingIndicator;
			modem.OnDataSetReady += Modem_OnDataSetReady;
			modem.OnSignalDetector += Modem_OnSignalDetector;
			modem.OnClearToSend += Modem_OnClearToSend;
			modem.OnRequestToSend += Modem_OnRequestToSend;
			modem.OnDataTerminalReady += Modem_OnDataTerminalReady;

			// Initialize DMA engine
			dmaEngine = new DMAEngine(true, regs, com5025, modem);

			dmaEngine.OnWriteDMA += DmaEngine_OnWriteDMA;
			dmaEngine.OnReadDMA += DmaEngine_OnReadDMA;
			dmaEngine.OnSetInterruptBit += DmaEngine_OnSetInterruptBit;
			dmaEngine.OnSendHDLCFrame += DmaEngine_OnSendHDLCFrame;			

		}

		private void DmaEngine_OnSendHDLCFrame(HDLCFrame frame)
		{
			// Send HDLC frame to modem	
			var frameData = frame.ToByteStuffedArray();

#if DEBUG_DETAIL

			StringWriter sb = new StringWriter();

			sb.Write($"Sending frame: ");
			for (int j = 0; j < frameData.Length; j++)
			{
				sb.Write($"0x{frameData[j]:X2} ");
			}
			sb.WriteLine("");
			Log(sb.ToString());
#endif

			// Blast the entire frame over the modem connection
			modem.SendBytes(frameData);
		}
		private void DmaEngine_OnSetInterruptBit(byte bit)
		{
			// Set interrupt bit in NDBus
			base.SetInterruptBit(bit);
		}

		private void DmaEngine_OnReadDMA(uint address, out int data)
		{
			data = base.DMARead(address);
		}

		private void DmaEngine_OnWriteDMA(uint address, ushort data)
		{
			base.DMAWrite(address, data);
		}

		override public void Reset()
		{
			regs.Clear();
			dmaEngine.Clear();
			com5025.Reset();
			UpdateRQTS();
		}


		// If ND-100 Clock is 40 Mhz, and we want to communicate on 64 kbps we need a RCP (Receiver Clock Pulse) signal that is 64Khz.
		// 40Mhz / 64000 = 
		//const int CPU_TICKS_DIVIDER = 625; //625 gives 12.12KB via HDLC test program
		//const int CPU_TICKS_DIVIDER = 24; // 24 gives slow 64Kbits! should be nice..   ?
		//								  // 24 now gives ~10KBits/sec

		public override void Clock(InterruptInfo ii)
		{
			com5025.Clock(ii);
			modem.Tick();
			dmaEngine.Tick();


			// DALYED TICK: Tick TX and RX clocks in the COM5025
			// (for this, use same clock speed for both TX and RX)
			regs.cpuTicks++;
			if (regs.cpuTicks >= regs.cpu_ticks_pr_tx)
			{
				regs.cpuTicks = 0;
				com5025.TickTXC();
				com5025.TickRXC();
				com5025.TickRegsClock();
			}
			base.Clock(ii);
		}

		/// <summary>
		/// IOX Read from HDLC controller
		/// </summary>
		/// <param name="address"></param>
		/// <returns></returns>
		override public ushort Read(int address)
		{
			Register r = (Register)address;
			ushort value = 0;


			switch (r)
			{
				case Register.ReadRxDR: // IOX+0
					value = com5025.Read((byte)RegistersByte.ReceiverDataBuffer);
					break;
				case Register.ReadReceiverStatusRegister: // IOX+2
					value = com5025.Read((byte)RegistersByte.ReceiverStatusRegister);
					break;
				case Register.WRITE_CharacterLength: // IOX+4 WTF ? The high byte of the Parameter Control Register (PCRH) is used to specify character length for receiver(bits 0 - 2) and transmitter(bits 5 - 7

					// Need to fix this. This reads settings from Parameters set by DMA, and PIO doesn't have this..
					// Test-program has 0x07D0 in A register when this is called

					// TODO: fix at a later point if necessary, dont think no logic cares as long as we are consistent

					//value = (byte)((this.parameterBuffer.ParameterControlRegister >> 8) & 0xFF);
					//com5025.Write((byte)RegistersByte.DataLengthSelectRegister, (byte)value);
					com5025.Write((byte)RegistersByte.DataLengthSelectRegister, 0); // use 8 bits

					break;
				case Register.ReadTransmitterStatusRegister: // IOX +6 
					value = com5025.Read((byte)RegistersByte.TransmitterStatusandControlRegister);
					break;

				case Register.ReadReceiverTransferStatus: //IOX+10
														  // Clear DMA flag pr doc before reading flags

#if DEBUG_DETAIL_PLUS_DESCRIPTION
					Log($"Read RRTS: {regs.ReceiverTransferStatus}");
#endif

					regs.ReceiverTransferStatus &= ~(RTSBits.DMAModuleRequest);


					// Latch RI, SD and DSR on read of this register
					regs.ReceiverTransferStatus &= ~(regs.RX_ModemFlagsMask); // Clear old modem-info bits
					regs.ReceiverTransferStatus |= (regs.RX_ModemFlags & regs.RX_ModemFlagsMask); ; // Latch in the new bits

					// Read flags into output variable
					value = (ushort)regs.ReceiverTransferStatus;

					// Clear bits 8-15 (DMA)
					regs.ReceiverTransferStatus = regs.ReceiverTransferStatus & ~(RTSBits.DMA_CLEAR_BITS);

					break;

				case Register.ReadTransmitterTransferStatus: //IOX+12

#if DEBUG_DETAIL_PLUS_DESCRIPTION
					Log($"Read RTTS: {regs.TransmitterTransferStatus}");
#endif

					// DMA Module Request is cleared at the beginning of IOX GP + 12.
					regs.TransmitterTransferStatus &= ~(TTSBits.DMAModuleRequest);

					// Latch ReadyForSending on read of this register
					// Latch new value from CCITT signal 106 (CTS, Ready For Sending/RFS)
					regs.TransmitterTransferStatus &= ~(regs.TX_ModemFlagsMask); // Clear old modem-info bits
					regs.TransmitterTransferStatus |= (regs.TX_ModemFlags & regs.TX_ModemFlagsMask); ; // Latch in the new bits

					// Read Transmitter Status register
					value = (ushort)regs.TransmitterTransferStatus;

					// Clear bits 8-15 (DMA)
					regs.TransmitterTransferStatus &= ~(TTSBits.DMA_CLEAR_BITS);

					break;

				case Register.ReadDMAAddress: // IOX +14
					value = regs.DMAAddressLO;
					break;


				case Register.ReadDMACommand: // IOX +16

					// Before a new command is written to the DMA module, this register should be inspected.
					// If it is zero, the new command sequence can be started.
					// If not, wait until it becomes zero. A MASTER CLEAR command sequence can, however, be started even if the command register is not zero.

					int cmd = (int)(regs.DMACommand);
					cmd = cmd << 8;
					value = (ushort)cmd;

					break;

				default:
					break;
			}
#if DEBUG_DETAIL
			Log($"IOX Read from {r} value {Numeric.Num2Str(value)}");
#endif
			return value;
		}


		/// <summary>
		/// IOX Write to HDLC controller
		/// </summary>
		/// <param name="address"></param>
		/// <param name="value"></param>
		override public void Write(int address, ushort value)
		{
			Register r = (Register)address;
#if DEBUG_DETAIL
			Log($"IOX Write to {r} value {Numeric.Num2Str(value)}");
#endif

			switch (r)
			{
				case Register.WritePCR: //IOX+1
					com5025.Write((byte)RegistersByte.ModeControlRegister, (byte)value); // Write COM5025 MODE register


#if DEBUG_DETAIL_PLUS_DESCRIPTION
					string pcrDescription = "";


					if (value.BIT(7)) pcrDescription += "[AllPartiesAddress] ";
					if (value.BIT(6)) pcrDescription += "[ProtoSelect 1] ";
					if (value.BIT(5)) pcrDescription += "[StripGo] ";
					if (value.BIT(4)) pcrDescription += "[SelAMode] ";
					if (value.BIT(3)) pcrDescription += "[IdleMode] ";

					if (value.BIT(2)) pcrDescription += "[CRC Z] ";
					if (value.BIT(1)) pcrDescription += "[CRC Y] ";
					if (value.BIT(0)) pcrDescription += "[CRC X] ";

					Log($"Writing WritePCR = {pcrDescription}");
#endif

					break;
				case Register.WriteSAR: //IOX+3 WRITE SYNC/ADDRESS REGISTER (SAR)
					com5025.Write((byte)RegistersByte.SYNCAddressRegister, (byte)value);
					break;
				case Register.WriteTransmitterDataRegister: //IOX+5  Transmit Data Register
					com5025.Write((byte)RegistersByte.TransmitterDataRegister, (byte)value);
					break;

				case Register.WriteTransmitterControlRegister: // IOX + 7
					com5025.Write((byte)RegistersByte.TransmitterStatusandControlRegister, (byte)value);
					break;

				case Register.WriteReceiverTransferControl: // IOX + 11

					if (value == 0)
					{
						// Clear maintenance/loopback
						regs.Maintenance = false;
					}

					regs.ReceiverTransferControl = (RTCBits)value;
					com5025.SetInputPinValue(SignalPinIn.RXENA, regs.ReceiverTransferControl.HasFlag(RTCBits.EnableReceiver));

					if (regs.ReceiverTransferControl.HasFlag(RTCBits.EnableReceiverDMA))
					{
#if DEBUG_DETAIL
						Log("DMA Receive enabled!");
#endif
					}


					// Moved to DMA dump
					//  Always 1 after IOX + 11 if inspected after a DUMP command (M11) (RH: not sure what this really means.. maybe something with DMA)
					//regs.ReceiverTransferControl |= ReceiverTransferControlFlags.Bit15;



					if (!regs.Maintenance)
					{
						if (regs.ReceiverTransferControl.HasFlag(RTCBits.DeviceClear_SelectMaintenance))
						{
							// Device Clear
							Log($"DeviceClear to Data module");
							regs.Clear(true);
							regs.ReceiverTransferControl &= ~(RTCBits.DeviceClear_SelectMaintenance);
							regs.Maintenance = true;

							com5025.SetInputPinValue(SignalPinIn.MSEL, true);

							UpdateRQTS();
						}
					}

					// Set DTR value to modem from bit DTR
					modem.SetDTR(regs.ReceiverTransferControl.HasFlag(RTCBits.DTR));

					CheckTriggerIRQ13();
					CheckTriggerInterrupt();

					break;

				case Register.WriteTransmitterTransferControlRegister: // IOX +13					
					regs.TransmitterTransferControl = (TTCBits)value;
					com5025.SetInputPinValue(SignalPinIn.TXENA, regs.TransmitterTransferControl.HasFlag(TTCBits.TransmitterEnabled));

					// Update RQTS from bit HalfDuplex and RequestToSend
					UpdateRQTS();


					// Is IRQ is being enabled, and we have some signals to be process?
					CheckTriggerIRQ12();

					break;

				case Register.WriteDMAAddress:// IOX+15

					// The 16 least significant bits for the first location in a load/dump area or
					// the first location in a list of buffer descriptors are written into a register(M3) in the DMA module.
					regs.DMAAddressLO = value;
					break;

				case Register.WriteDMACommand: // IOX +17

					// Before a new command is written to the DMA module, this register should be inspected.
					// If it is zero, the new command sequence can be started. If not, wait until it becomes zero.
					// A MASTER CLEAR command sequence can, however, be started even if the command register is not zero.


					// nope, this is just guessing. Not documented anywhere
					//if ((value & 1 << 5) != 0)
					//{
					//	// Device Clear
					//	Log($"WriteDMACommand: DeviceClear to DMA module");
					//	regs.Clear(true);
					//}

					regs.DMACommand = (DMACommands)((value >> 8) & 0x07);
					regs.DMABankBits = value & 0x0F;

					dmaEngine.ExecuteCommand();

					break;

				default:
					break;
			}
		}



		#region Event Handler for modem signals change and data received 


		/// <summary>
		/// Data received from the TCP modem
		/// </summary>
		/// <param name="data"></param>
		/// <param name="length"></param>
		private void Modem_OnReceivedData(byte[] data, int length)
		{
			try
			{
				if (dmaEngine.BurstMode)
				{
					// In burst mode we send the TCP data directly to the DMA engine
					dmaEngine.Receiver.DMABlastReceiveDataBuffer(data, length);
				}
				else
				{
					// When not in blat-mode we send the data through the COM5025 chip and its statemachine
					com5025.ReceiveData(data, length);
				}
			}
			catch (Exception ex)
			{
				Logger.Dbg($"Unexpected exception in Modem_OnReceivedData {ex}");
			}

		}


		/// <summary>
		/// Signal pin CTS (CCITT 106) HDLC: Also known as RFS/Ready For Sending
		///
		/// Only pin-change that can trigger INT 12
		/// </summary>
		/// <param name="pinValue"></param>
		private void Modem_OnClearToSend(bool pinValue)
		{
			// Input for Data
#if DEBUG_DETAIL
			Log($"MODEM: ClearToSend (CTS) changed to {pinValue}");
#endif
			if (pinValue)
			{

				regs.TX_ModemFlags |= TTSBits.ReadyForSending;
			}
			else
			{
				regs.TX_ModemFlags &= ~TTSBits.ReadyForSending;
			}


			CheckTriggerIRQ12();
		}

		private void Modem_OnDataTerminalReady(bool pinValue)
		{
			// Host changed signal DTR. In point-to-point setup, we connect DTR to DSR (Data Set Ready) 
			modem.SetDSR(pinValue);
		}

		private void Modem_OnRequestToSend(bool pinValue)
		{
			// Host sets RTS (Request to Send) signal, in point-to-point setup, we connect RTS directly to CTS (Clear to Send)
			modem.SetCTS(pinValue);
		}

		private void Modem_OnDataSetReady(bool pinValue)
		{
#if DEBUG_DETAIL
			Log($"MODEM: DataSetReady (DSR) changed to {pinValue}");
#endif

			if (pinValue)
			{

				regs.RX_ModemFlags |= RTSBits.DataSetReady;
			}
			else
			{
				regs.RX_ModemFlags &= ~RTSBits.DataSetReady;
			}
			CheckTriggerIRQ13();
		}


		private void Modem_OnRingIndicator(bool pinValue)
		{
			// Change RI ?
#if DEBUG_DETAIL
			Log($"MODEM: RingIndicator (RI) changed to {pinValue}");
#endif
			if (pinValue)
			{
				regs.RX_ModemFlags |= RTSBits.RingIndicator;
			}
			else
			{
				regs.RX_ModemFlags &= ~RTSBits.RingIndicator;
			}
			CheckTriggerIRQ13();
		}

		private void Modem_OnSignalDetector(bool pinValue)
		{
			// Change SD ?
#if DEBUG_DETAIL
			Log($"MODEM: SignalDetector (SD) changed to {pinValue}");
#endif

			if (pinValue)
			{
				regs.RX_ModemFlags |= RTSBits.SignalDetector;
			}
			else
			{
				regs.RX_ModemFlags &= ~RTSBits.SignalDetector;
			}
			CheckTriggerIRQ13();
		}

		#endregion Event Handler for modem signals change (for receiver part of HDLC)


		#region Event Handler for modem TX and RX of 8-bit data for integration with COM5025 chip


		/// <summary>
		/// Event from COM5025 when it has a byte ready to be transmitted
		/// </summary>
		/// <param name="serialOutput"></param>

		private void Com5025_OnTransmitterOutput(byte serialOutput)
		{
			//Log($"[TX 0x{serialOutput:X2}]");

			// If parent has hooked this event, use this to send the bytes
			if (OnTransmitterOutput != null)
			{
				OnTransmitterOutput(serialOutput);
			}
			else
			{
				modem.SendByte(serialOutput);
			}
		}




		/// <summary>
		/// Setting flags based on pin changes on the COM5025 chip
		/// 
		/// Internal function
		/// </summary>
		/// <param name="pin"></param>
		/// <param name="value"></param>
		private void Com5025_OnPinValueChanged(SignalPinOut pin, bool value)
		{

			if (pin != SignalPinOut.TSO)
			{
				//Log($"[PIN:{pin}={value}]");
				//Debug.Write($"");
			}

			switch (pin)
			{
				case SignalPinOut.SFR: // Sync/Flag received (bit 3)
					if (value)
						regs.ReceiverTransferStatus |= RTSBits.SyncFlagReceived;
					else
						regs.ReceiverTransferStatus &= ~RTSBits.SyncFlagReceived;
					break;
				case SignalPinOut.RXACT: //Receiver Active (bit 2)
					if (value)
						regs.ReceiverTransferStatus |= RTSBits.ReceiverActive;
					else
						regs.ReceiverTransferStatus &= ~RTSBits.ReceiverActive;
					break;
				case SignalPinOut.RDA: //Receiver Data Available (interrupt on level 13 if enabled)
					if (value)
					{
						regs.ReceiverTransferStatus |= RTSBits.DataAvailable;
						CheckTriggerInterrupt();
					}
					else
						regs.ReceiverTransferStatus &= ~RTSBits.DataAvailable;
					break;

				case SignalPinOut.RSA: // Receiver STATUS available (bit 0)
					if (value)
					{
						regs.ReceiverTransferStatus |= RTSBits.StatusAvailable;
						CheckTriggerInterrupt();
					}
					else
						regs.ReceiverTransferStatus &= ~RTSBits.StatusAvailable;


					break;

				case SignalPinOut.TXACT: //Transmitter Active (bit 2)
					if (value)
					{
						regs.TransmitterTransferStatus |= TTSBits.TransmitterActive;
					}
					else
					{
						regs.TransmitterTransferStatus &= ~TTSBits.TransmitterActive;
					}

					//TXACT may impact RQTS
					UpdateRQTS();

					break;

				case SignalPinOut.TBMT: //Transmitter Buffer Empty (interrupt on level 12 if enabled) (bit 0)
					if (value)
						regs.TransmitterTransferStatus |= TTSBits.TransmitBufferEmpty;
					else
						regs.TransmitterTransferStatus &= ~TTSBits.TransmitBufferEmpty;

					CheckTriggerInterrupt();


					break;
				case SignalPinOut.TSA: // Transmitter Status Available (TERR bit, indicating transmitter underflow)
					if (value)
						regs.TransmitterTransferStatus |= TTSBits.TransmitterUnderrun;
					else
						regs.TransmitterTransferStatus &= ~TTSBits.TransmitterUnderrun;

					CheckTriggerInterrupt();
					break;

			}



			if (regs.TransmitterTransferControl.HasFlag(TTCBits.EnableTransmitterDMA))
			{
				if ((pin == SignalPinOut.TBMT) && (value))
				{
					dmaEngine.Transmitter.DMA_SendChar(false);
				}
			}

		}

		#endregion Event Handler for modem TX and RX of 8-bit data for integration with COM5025 chip


		/// <summary>
		/// Update the RQTS signal based on the different input signals to the logic that combines RQTS signal
		/// See page A-19 in ND-12.018.01
		/// </summary>
		private void UpdateRQTS()
		{

			if (regs.Maintenance)
			{
				modem.SetRTS(true);
				return;
			}

			bool a = !modem.SignalDetector; // Negated Signal Detect

			bool b = regs.TransmitterTransferControl.HasFlag(TTCBits.RequestToSend);
			b |= regs.TransmitterTransferStatus.HasFlag(TTSBits.TransmitterActive); ;


			// negate B
			b = !b;


			// TransmitterEnable bit 5
			/// A 1 in this bit will cause the interface to operate in a half duplex mode.
			/// The request to send (RQTS) (CCITT circuit 1065) signal is not turned ON unless the Signal Detector (SD) (CCITT circuit 109) is off.
			/// A 0 in this bit will cause the interface to operate in a full duplex mode.


			bool c = regs.TransmitterTransferControl.HasFlag(TTCBits.HalfDuplex);

			// RQTS is a 3-input Negated-OR (NOR) from A,B and C
			bool new_rqts = !(a | b | c);


			modem.SetRTS(new_rqts);
		}

		#region Interrupt handling

		/// <summary>
		/// Check if there is a signal difference between the signal from the modem and what is latched into the TransmitterStatus register.
		/// If difference, trigger interrupt 12
		/// </summary>
		void CheckTriggerIRQ12()
		{
			// Check if TRANSMIT interrupt is enabled
			if (!regs.TransmitterTransferControl.HasFlag(TTCBits.ModemStatusChangeIE)) return;


			TTSBits write_signals = regs.TransmitterTransferStatus & regs.TX_ModemFlagsMask;
			TTSBits modem_signals = regs.TX_ModemFlags & regs.TX_ModemFlagsMask;

			// TMCS (Transmit Modem Status Change) set => Trigger interrupt 12 on change if OCW bit 7 is set
			bool TMCS = write_signals != modem_signals;
			if (TMCS)
			{
				SetInterruptBit(12);
			}

			// Reading Output Status register (OSR) will latch inn the modem values, and the diff wil disappear
			// Se page A-19

		}



		/// <summary>
		/// Check if there is a signal difference between the signal from the modem and what is latched into the ReceiverTransferStatus register.
		/// If difference, trigger interrupt 13
		/// </summary>
		void CheckTriggerIRQ13()
		{
			if (!regs.ReceiverTransferControl.HasFlag(RTCBits.ModemStatusChangeIE)) return;

			RTSBits read_signals = regs.ReceiverTransferStatus & regs.RX_ModemFlagsMask;
			RTSBits modem_signals = regs.RX_ModemFlags & regs.RX_ModemFlagsMask;

			//Receive Modem Status Change
			bool RMSC = read_signals != modem_signals;
			if (RMSC)
			{
				SetInterruptBit(13);
			}


			// Reading Input Status register (ISR) will latch inn the modem values, and the diff wil disappear
			// Se page A-19 ND-12.018.01
		}


		/// <summary>
		/// Check is we need to trigger an interrupt on level 13 or 12
		/// </summary>
		internal void CheckTriggerInterrupt()
		{
			/*** LEVEL 13 RX ***/


			if (regs.ReceiverTransferStatus.HasFlag(RTSBits.DataAvailable))
			{
				if (regs.ReceiverTransferControl.HasFlag(RTCBits.EnableReceiverDMA))
				{
					dmaEngine.Receiver.DataAvailableFromCOM5025();
				}

				if (regs.ReceiverTransferControl.HasFlag(RTCBits.DataAvailableIE))
				{
					SetInterruptBit(13, true);
				}
			}


			if (regs.ReceiverTransferStatus.HasFlag(RTSBits.StatusAvailable))
			{
				if (regs.ReceiverTransferControl.HasFlag(RTCBits.EnableReceiverDMA))
				{
					dmaEngine.Receiver.StatusAvailableFromCOM5025();
				}

				if (regs.ReceiverTransferControl.HasFlag(RTCBits.StatusAvailableIE))
					SetInterruptBit(13, true);
			}



			// Is checked in case read Register.ReadReceiverTransferStatus:
			//if (regs.ReceiverTransferControl.HasFlag(ReceiverTransferControlFlags.ModemStatusChangeIE))
			//	GenerateInterrupt(13, true);



			/*** LEVEL 12 TX ***/
			if (regs.TransmitterTransferStatus.HasFlag(TTSBits.TransmitBufferEmpty))
			{
				if (regs.TransmitterTransferControl.HasFlag(TTCBits.TransmitBufferEmptyIE))
				{
					SetInterruptBit(12, true);
				}
			}


			if (
				(regs.TransmitterTransferStatus.HasFlag(TTSBits.TransmitterUnderrun)) &&
				(regs.TransmitterTransferControl.HasFlag(TTCBits.TransmitBufferEmptyIE))
				)
				SetInterruptBit(12, true);
		}


		internal override ushort IDENT(ushort level)
		{
			Log($"HDLC IDENT level {level}");
			if (level == 12)
				regs.TransmitterTransferControl &= TTCBits.MASK_CLEAR_IDENT;

			if (level == 13)
				regs.ReceiverTransferControl &= RTCBits.MASK_CLEAR_IDENT;

			return base.IDENT(level);
		}

		public void SetTH1(TH1Wheel th)
		{
			regs.SetTH1(th);
		}

		#endregion Interrupt handling
	}

}


/*

X-C:start-link,1360,,,-1,,


Link table status: 4 entries. 1 in use. Max 1 used.

No  Addr. State Sysid Rcv Xmit  Lun Timeout  Soft-stat-hard TXData/Retry/RXBad
 1 134206 Call     0    0 SABM 1360  10/  7     103       0          0/0/0


Maybe MEM should be read again and again to check for changes ?


Sin
Start-x
exit
SET-AVAIL
START-TADADM
TADADM
X

DEF-REMOTE,,D100 100
DEF-REMOTE,,D102 102
DEF-REMOTE,,D103 103

START-LINK,1360,,,-1,,



STOP-LINK,1360,,
START-LINK,1360,,,-1,,



list-link
List-Routing-Info

debugtrace 2 5
setlogid 100
cont

debugtrace 2 5
setlogid 102
cont



*/

