#define DEBUG_DETAIL
#define DEBUG_DETAIL_PLUS_DESCRIPTION

#define DMA_DEBUG


using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;
using Emulated.HW.Common.Network;
using Emulated.HW.Common.Telnet;
using Emulated.HW.SMC.COM.COM5025;
using Emulated.Lib;

namespace Emulated.HW.ND.CPU.NDBUS.HDLC
{
	internal class DMAEngine
	{

		public event ReadWordByVirtualAddress? OnReadDMA;
		public event WriteWordByVirtualAddress? OnWriteDMA;
		public event SendHDLCFrameEventHandler? OnSendHDLCFrame;
		public event SetInterruptBitEventHandler? OnSetInterruptBit;

		/// <summary>
		/// Registers of the DMA engine
		/// </summary>
		DMAControlBlocks dmaCB;




		/// <summary>
		/// DMA Transmitter engine
		/// </summary>
		internal DMATransmitter Transmitter;

		/// <summary>
		/// DMA Receiver engine
		/// </summary>
		internal DMAReceiver Receiver;

		/// <summary>
		/// COM5025 us the HDLC controller chip (Multi-Protocol Universal Synchronous Receiver/Transmitter)
		/// </summary>
		COM5025 com5025;

		/// <summary>
		/// Registers hold all the internal variables for the HDLC controller
		/// </summary>
		Registers regs;


		/// <summary>
		/// Modem is used to transmit HDLC frames over TCP
		/// </summary>
		TelnetModem modem;


		public bool BurstMode => dmaCB.BurstMode;
		/// <summary>
		/// 
		/// </summary>
		/// <param name="burstMode">Burst mode enabled (TCP quick transfer)</param>
		/// <param name="r">Register</param>
		/// <param name="c">COM5025</param>
		/// <param name="t">TelnetModem</param>
		public DMAEngine(bool burstMode, Registers r, COM5025 c, TelnetModem t)
		{
			
			dmaCB = new DMAControlBlocks();
			dmaCB.SetBurstMode(burstMode);			

			regs = r;
			com5025 = c;
			modem = t;

			Transmitter = new DMATransmitter(r, dmaCB, c);
			Receiver = new DMAReceiver(r, dmaCB, c);

			Transmitter.OnSendHDLCFrame += (frame) => OnSendHDLCFrame?.Invoke(frame);

			Transmitter.OnSetInterruptBit += DMAEngineOnSetInternetbit;
			Receiver.OnSetInterruptBit += DMAEngineOnSetInternetbit;

			dmaCB.OnReadDMA += DmaCB_OnReadDMA;
			dmaCB.OnWriteDMA += DmaCB_OnWriteDMA;


		}		

		private void DmaCB_OnWriteDMA(uint address, ushort data)
		{
			if (OnWriteDMA != null)
			{
				OnWriteDMA(address, data);
			}
		}

		private void DmaCB_OnReadDMA(uint address, out int data)
		{
			data = -1;
			if (OnReadDMA != null)
			{
				OnReadDMA(address, out data);
			}
		}

		private void DMAEngineOnSetInternetbit(byte bit)
		{
			// pass it on
			if (OnSetInterruptBit != null)
			{
				OnSetInterruptBit(bit);
			}
		}

		private void Transmitter_OnSendHDLCFrame(HDLCFrame frame)
		{
			if (OnSendHDLCFrame != null)
			{
				OnSendHDLCFrame(frame);
			}
		}

		internal void Tick()
		{
			Transmitter.Tick();
			Receiver.Tick();
		}

		internal void Clear()
		{

		}

		int DMARead(uint address)
		{
			if (OnReadDMA != null)
			{
				OnReadDMA(address, out int data);
				return data;
			}
			return -1;
		}
		public void DMAWrite(uint coreAddress, ushort data)
		{
			if (OnWriteDMA != null)
			{
				OnWriteDMA(coreAddress, data);
			}
		}



		#region DMA COMMANDS
		/*
		The commands may be divided into 3 groups:

		1. Device Clear ( 1)
		2. Load/ Dump and Initialize (4)
		3. Data Transfer (3)


		Device Clear
			is started by placing octal 40 in the A register and executing IOX GP + 17 (octal).

		Load/ Dump and Initialize
			is started by first writing the least significant 16 bits of a buffer address to
			the interface (IOX GP + 15 (octal)) and then writing the two most
			significant buffer address bits (bank bits) together with the command bits to
			the interface (IOX GP + 17 (octal)).

		Data Transfer
			is started by first writing the least significant 16 bits of a buffer address to
			the interface (IOX GP + 15 (octal)), then the two most significant bits (bank
			bits) together with the command bits (IOX GP + 17 (octal)) and at last
			enable interrupt and DMA module (IOX GP + I l(octal) for receiver and IOX
			GP + 13 (octal) for transmitter).


			A command sequence should never be interrupted.

			The Specific Commands
			Eight different commands may be used. They are:
			A register when IOX GP + 17 is executed. Y is bank address: BE. P,3]

			- Device Clear
			- Initialize
			- Receiver Start
			- Receiver Continue
			- Transmitter Start
			- Dump Data Module
			- Dump Register
			- Load register
		*/
		internal void ExecuteCommand()
		{
#if DEBUG_DETAIL
			Log($"Executing command {regs.DMACommand} BANK[{regs.DMABankBits}]"); // Debug
#endif

			if ((regs.DMA_Address == 0) && (regs.DMACommand != DMACommands.DEVICE_CLEAR))
			{
				// Signal that command is complete
				ClearDMACommand();

				return; // Something is wrong..
			}


			switch (regs.DMACommand)
			{

				case DMACommands.DEVICE_CLEAR: // Initialization
					CommandDeviceClear();
					break;
				case DMACommands.INITIALIZE: // Initialization
					CommandInitialize();
					break;
				case DMACommands.RECEIVER_START: // Data transfer
					CommandReceiverStart();
					break;
				case DMACommands.RECEIVER_CONTINUE: // Data transfer
					CommandReceiverContinue();
					break;
				case DMACommands.TRANSMITTER_START: // Data transfer
					CommandTransmitterStart();
					break;
				case DMACommands.DUMP_DATA_MODULE: // Maintenance
					CommandDumpDataModule();

					//  Always 1 after IOX + 11 if inspected after a DUMP command (M11) (TODO: Validate)
					regs.ReceiverTransferControl |= RTCBits.Bit15;
					break;
				case DMACommands.DUMP_REGISTERS: // Maintenance
					CommandDumpRegisters();

					//  Always 1 after IOX + 11 if inspected after a DUMP command (M11) (TODO:Validate)
					regs.ReceiverTransferControl |= RTCBits.Bit15;
					break;
				case DMACommands.LOAD_REGISTERS: // Maintenance
					CommandLoadRegisters();
					break;
				default:
					break;
			}
		}

		

		/// <summary>
		/// Signal that command is complete
		/// </summary>
		void ClearDMACommand()
		{
			regs.DMACommand = 0;
		}

		/// <summary>
		/// Device Clear Function (Command 0)
		/// The Device Clear sequence as described above will stop all data transfers to and
		/// from the interface, and it can be used anytime.Device Clear will clear all interrupts
		/// from the interface, and a dialed up modem connection will be broken.
		/// </summary>
		private void CommandDeviceClear()
		{
			/// Recommended program for Device Clear is:
			///
			/// SAA 0 % A register = 0
			/// IOXGP + 11 (octal) % Write Receiver Transfer Control
			/// BSET ONE 50 DA % A register = 40(octal)
			/// IOX GP + 11 (octal) % Device Clear to Data Module
			/// IOX GP + 17 (octal) % Device Clear to DMA Module
			/// The Device Clear sequence as described above will stop all data transfers to and
			/// from the interface and it can be used at any time. Device Clear will clear all
			/// interrupts from the interface and a dialed up modem connection will be broken.


			regs.Clear();
			Clear();
			
			com5025.Reset();
			//TODO: FIX ? UpdateRQTS();

			ClearDMACommand();
		}

		/// <summary>
		/// INITALIZE (Command 1)
		/// </summary>
		private void CommandInitialize()
		{


			/// The Initialize sequence uses 7 locations in memory. The contents of the locations are:
			///
			///		1.Parameter Control Reg.	(8 least significant bits)
			///		2.Sync/Address Register		(8 least significant bits)
			///		3.Character Length			(8 least significant bits)
			///		4.Displacement 1			(No.of bytes, first block in frame)
			///		5.Displacement 2			(No.of bytes, other blocks in frame)
			///		6.Max.Rec.Block Length(No.of bytes, including displacement)
			///		7.Checksum(102164 is written back from interface
			///
			/// The content of the 3 first locations are written into the Data Module and the
			/// mapping of the control bits are described in data sheets for SMC COM 5025 and
			/// Signetics MPCC 2652.
			///
			/// Displacement 1 is the number of free bytes reserved at the
			/// beginning of each buffer containing the start of a message(Frame).
			///
			/// Displacement 2 is the number of free bytes reserved at the beginning of each buffer which
			/// does not contain the start of a message(Frame).
			///
			/// Maximum Receiver Block Length is the total number of bytes in a receiver buffer, including displacement.
			///
			/// The Checksum written back from the interface may be used as a control.
			///
			/// The interface should not be used in DMA mode if this checksum is wrong.


			uint dma_address = regs.DMA_Address;


			// Save here for later re-use
			dmaCB.Parameters.DmaBankBits = regs.DMABankBits;

			dmaCB.Parameters.ParameterControlRegister = DMARead(dma_address++);
			com5025.Write((byte)RegistersByte.ModeControlRegister, (byte)(dmaCB.Parameters.ParameterControlRegister)); // Write COM5025 MODE register

			dmaCB.Parameters.Sync_AddressRegister = DMARead(dma_address++);
			com5025.Write((byte)RegistersByte.SYNCAddressRegister, (byte)(dmaCB.Parameters.Sync_AddressRegister & 0x00FF));


			dmaCB.Parameters.CharacterLength = DMARead(dma_address++);
			com5025.Write((byte)RegistersByte.DataLengthSelectRegister, (byte)(dmaCB.Parameters.CharacterLength));

			dmaCB.Parameters.Displacement1 = DMARead(dma_address++);
			dma_registers[5] = (ushort)dmaCB.Parameters.Displacement1;

			dmaCB.Parameters.Displacement2 = DMARead(dma_address++);
			dma_registers[6] = (ushort)dmaCB.Parameters.Displacement2;

			dmaCB.Parameters.MaxReceiverBlockLength = DMARead(dma_address++);
			dma_registers[7] = (ushort)dmaCB.Parameters.Displacement2;

			int checksum = DMARead(dma_address);

#if DMA_DEBUG
			Log("--------------------------------------------------------------------------");
			Log($"DMA CommandInitialize    : 0x{regs.DMA_Address:X6} ");
			Log("");
			Log($"DmaBankBits              : 0x{regs.DMABankBits:X4} ");
			Log($"ParameterControlRegister : 0x{dmaCB.Parameters.ParameterControlRegister:X4} ");
			Log($"Sync_AddressRegister     : 0x{dmaCB.Parameters.Sync_AddressRegister:X4} ");
			Log($"CharacterLength          : 0x{dmaCB.Parameters.CharacterLength:X4} ");
			Log($"Displacement1            : 0x{dmaCB.Parameters.Displacement1:X4} ");
			Log($"Displacement2            : 0x{dmaCB.Parameters.Displacement2:X4} ");
			Log($"MaxReceiverBlockLength   : 0x{dmaCB.Parameters.MaxReceiverBlockLength:X4} ");
			Log("--------------------------------------------------------------------------");
#endif


			if (checksum == 0) // Only write back of the memory address is 0, if not something is off..
			{
				DMAWrite(dma_address++, (ushort)0x8474); // 0102164 is written back from interface
			}

			ClearDMACommand();
		}








		/// <summary>
		/// RECEIVER START (Command 2)
		/// 
		/// The address written to the interface in a Receiver Start sequence is denoted a "List Pointer".
		/// This address is the first address of a list containing "Buffer Descriptors" (See HDLC DMA Structure).
		/// This command also selects Displacement 1 for the first buffer, and should therefore be used the first time the receiver is started after a power up or receiver disable.
		///
		/// The receiver should normally run.
		///
		/// A Receiver Request from the DATA module will then automatically be handled.		
		/// </summary>
		private void CommandReceiverStart()
		{

			dmaCB.SetRXPointer(regs.DMA_Address, 0);

			Receiver.SetReceiverState();
			

#if DMA_DEBUG
			Log("--------------------------------------------------------------------------");
			Log($"DMA CommandReceiverStart    : 0x{regs.DMA_Address:X6}  ({Numeric.Num2Str(regs.DMA_Address, len: 8)})");
			Log($"RX BufferDescription        : 0x{dmaCB.RX_ListPointer:X6} ");
			Log($"RX Displacement             : 0x{dmaCB.RX_DCB?.Displacement:X2} ");
			Log("--------------------------------------------------------------------------");
#endif

			ClearDMACommand();
		}


		/// <summary>
		/// RECEIVER CONTINUE (Command 3)
		///
		/// This command is used to write a new List Pointer to an enabled and working interface. It should only be used as a response to a "List Empty" interrupt.
		/// </summary>
		private void CommandReceiverContinue()
		{
			dmaCB.SetRXPointer(regs.DMA_Address, 0);
			Receiver.SetReceiverState();

#if DMA_DEBUG
			Log("--------------------------------------------------------------------------");
			Log($"DMA CommandReceiver CONTINUE  : 0x{regs.DMA_Address:X6} ({Numeric.Num2Str(regs.DMA_Address, len: 8)})");
			Log($"RX ListPointer                : 0x{dmaCB.RX_ListPointer:X6} ");
			Log($"RX Displacement (#1?)         : 0x{dmaCB.RX_DCB?.Displacement:X2} ");
			Log("--------------------------------------------------------------------------");
#endif

			ClearDMACommand();

		}



		/// <summary>
		/// TRANSMITTER START(Command 4)
		///
		/// This command is always used to start transmission of data.
		/// As for RECEIVER START, an address is written to the interface when the transmitter is started.
		///
		/// To enable the transfer, the Transmitter Control register(10X<device no.> +13) has to be loaded.
		/// </summary>
		private void CommandTransmitterStart()
		{
			
			

#if DMA_DEBUG
			Log("--------------------------------------------------------------------------");
			Log($"DMA CommandTransmitterStart   : 0x{regs.DMA_Address:X6}  ({Numeric.Num2Str(regs.DMA_Address, len: 8)})");
			Log("--------------------------------------------------------------------------");
#endif

			dmaCB.SetTXPointer(regs.DMA_Address, 0);
			dmaCB.DebugTXFrames();

			// This command is always used to start transmission of data. The address written to the
			// interface is the 'Transmitter List Pointer' or the start address for the list of
			// 'Buffer Descriptors'

			Transmitter.SetSenderState(DmaEngineSenderState.BLOCK_READY_TO_SEND);
			/*
			if (regs.DMA_TX_BufferDescription.Key == KeyFlags.BlockToBeTransmitted) //| (regs.DMA_TX_BufferDescription.Key == KeyFlags.AlreadyTransmittedBlock))
			{
				regs.DMA_TX_BufferDescription.Displacement = (ushort)parameterBuffer.Displacement1;


				SetTSOM(true); // Start of message
				SetTEOM(false); // Make sure TEOM is off

				//DMA_SendChar(true);
				dmaSenderState = DMA_SenderState.SENDING;
			}
			else
			{
				dmaSenderState = DMA_SenderState.IDLE;
				//SetTXDMAFlag(TransmitterStatusBits.ListEnd);
			}
			*/

			// Signal that command is complete
			ClearDMACommand();
		}


	



	


		/// <summary>
		/// DUMP DATA MODULE (Command 5)
		/// </summary>
		private void CommandDumpDataModule()
		{
			ushort data;
			/*
			 This command is mainly for maintenance purpose.
			It requires 5 locations in memory, where the contents of the following registers are stored: 
				1. Parameter Control Register (8 least sign. bits)  IOX+1
				2. Sync/Address Register (8 least sign. bits) IOX+3
				3. Character Length (8 least sign. bits) IOX+4
				4. Receiver Status Register (8 least sign. bits, not accumulated)
				5. Transmitter Status Register least sign. bits, not accumulated)

			The contents of the registers in the Multi Protocol Communication Controller MPCC) are transferred to memory.
			The Receiver Status Register is also OR-ed into the Receiver Dataflow Status Register to prevent loss of information. 

			*/
			uint dma_address = regs.DMA_Address;


			data = com5025.Read((byte)RegistersByte.ModeControlRegister);
			DMAWrite(dma_address++, (byte)data);

			data = com5025.Read((byte)RegistersByte.SYNCAddressRegister);
			DMAWrite(dma_address++, (byte)data);

			data = com5025.Read((byte)RegistersByte.DataLengthSelectRegister);
			DMAWrite(dma_address++, (byte)data);

			data = com5025.Read((byte)RegistersByte.ReceiverStatusRegister); //TODO: The Receiver Status Register is also OR-ed into the Receiver Dataflow Status Register to prevent loss of information. 
			DMAWrite(dma_address++, (byte)data);

			data = com5025.Read((byte)RegistersByte.TransmitterStatusandControlRegister);
			DMAWrite(dma_address++, (byte)data);

			// Signal that command is complete
			ClearDMACommand();
		}


		ushort[] dma_registers = new ushort[256];
		/// <summary>
		/// DUMP REGISTER (Command 6)
		/// </summary>
		private void CommandDumpRegisters()
		{
			ushort data = 0;

			/*

				This command can be used to dump the contents of any number of the 256 random access memory registers in the DMA module.
				Required space in memory is 2 locations plus one location for each register to be dumped.

					The contents of the two locations are:
						1. First Register Address
						2. Number of Registers

				If both values are zero, the contents of the 16 registers in the Bit Slice are written into memory. 
			*/


			uint dma_address = regs.DMA_Address;

			ushort firstReg = (ushort)(DMARead(dma_address++) & 0x00FF);
			ushort numreg = (ushort)(DMARead(dma_address++) & 0x00FF);

			if ((firstReg == 0) && (numreg == 0))
			{
				//If both values are zero, the contents of the 16 registers in the _Bit Slice_ are written into memory.
				for (byte i = 0; i < 16; i++)
				{
					data = i; // what to report ??
					DMAWrite(dma_address++, (ushort)data);
				}
			}
			else
			{
				for (ushort i = 0; i < numreg; i++)
				{
					int offset = firstReg + i;
					if (offset < 256)
					{
						data = dma_registers[offset];
						DMAWrite(dma_address++, (ushort)data);

						Log($"DUMP REGISTER {offset} = {Numeric.Num2Str(data)}");
					}
				}
			}

			// Signal that command is complete
			ClearDMACommand();
		}



		/// <summary>
		/// LOAD REGISTER (Command 7) 
		/// </summary>
		private void CommandLoadRegisters()
		{
			/*
				This command can be used to load any number of the 256 random access memory registers in the DMA module.

				Required space in memory is 2 locations plus one location for each register to be loaded.
				The contents of the two locations are:
					1. First register address
					2. Number of Registers 

				The Load Register command is simular to Dump Register, except that data is moved in the opposite direction.

				It is not possible to load the registers in the Bit Slice by this command. 
				The commands, how to activate them and use them in some simple debugging programs are given in Appendix A5. 


			 */

			uint dma_address = regs.DMA_Address;

			ushort data = 0;
			ushort firstReg = (ushort)(DMARead(dma_address++) & 0x00FF);
			ushort numreg = (ushort)(DMARead(dma_address++) & 0x00FF);


			for (ushort i = 0; i < numreg; i++)
			{
				int offset = firstReg + i;
				if (offset < 256)
				{
					int readVal = DMARead(dma_address++); // returns -1 if it fails
					if (readVal >= 0)
					{
						dma_registers[offset] = (ushort)readVal;
						Log($"LOAD REGISTER {offset} = {Numeric.Num2Str(readVal)}");
					}
				}
			}

			// Signal that command is complete
			ClearDMACommand();
		}





		internal ushort GetBufferKeyVault(uint list_pointer, ushort offset)
		{
			uint current_list_pointer = list_pointer + (uint)(offset * 4);
			ushort keyValue = (ushort)DMARead(current_list_pointer);
			return keyValue;
		}

		







		#endregion COMMANDS








		uint ScanNextTXBuffer(uint start)
		{
			uint nextMem = start;

			while (true)
			{
				int memkey = DMARead(nextMem);
				if (memkey == 0) return 0; // end of pointers

				KeyFlags key = (KeyFlags)(memkey) & KeyFlags.MASK_KEY;

				if (key == KeyFlags.BlockToBeTransmitted) return nextMem;
				if (key == KeyFlags.NewListPointer) return nextMem;

				nextMem += 4; //next memory
			}


			return 0;
		}


		

		#region Helper functions for commands
		private void ReadParameterBuffer()
		{
			uint dma_address = regs.DMA_Address;


			dmaCB.Parameters.ParameterControlRegister = DMARead(dma_address++);
			//regs.ReceiverTransferControl = (ReceiverTransferControlFlags)(parameterBuffer.ParameterControlRegister & 0x00FF) | (ReceiverTransferControlFlags)((ushort)regs.ReceiverTransferControl & 0xFF00);

			com5025.Write((byte)RegistersByte.ModeControlRegister, (byte)(dmaCB.Parameters.ParameterControlRegister >> 8)); // Write COM5025 MODE register

			com5025.Write((byte)RegistersByte.SYNCAddressRegister, (byte)dmaCB.Parameters.ParameterControlRegister); // Write COM5025 MODE register

			dmaCB.Parameters.Sync_AddressRegister = DMARead(dma_address++);
			com5025.Write((byte)RegistersByte.SYNCAddressRegister, (byte)(dmaCB.Parameters.Sync_AddressRegister & 0x00FF));


			dmaCB.Parameters.CharacterLength = DMARead(dma_address++);
			com5025.Write((byte)RegistersByte.DataLengthSelectRegister, (byte)(dmaCB.Parameters.CharacterLength));

			dmaCB.Parameters.Displacement1 = DMARead(dma_address++);
			dma_registers[5] = (ushort)dmaCB.Parameters.Displacement1;

			dmaCB.Parameters.Displacement2 = DMARead(dma_address++);
			dma_registers[6] = (ushort)dmaCB.Parameters.Displacement2;

			dmaCB.Parameters.MaxReceiverBlockLength = DMARead(dma_address++);
			dma_registers[7] = (ushort)dmaCB.Parameters.Displacement2;

			int checksum = DMARead(dma_address);

			DMAWrite(dma_address++, (ushort)0x8474); // 0102164 is wrritten back from interface
		}

	


		private void Log(string logMsg)
		{
			Logger.Log($"DMAEngine: {logMsg}", Logger.LogLevel.Device);
		}
		#endregion Helper functions for commands
	}
}

