#define DEBUG_DETAIL
#define DEBUG_DETAIL_PLUS_DESCRIPTION

#define DMA_DEBUG
//#define RX_BLAST_LOGGING


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Emulated.HW.Common.Network;
using Emulated.Lib;

namespace Emulated.HW.ND.CPU.NDBUS.HDLC
{
	internal class DMAControlBlocks
	{

		public event ReadWordByVirtualAddress? OnReadDMA;
		public event WriteWordByVirtualAddress? OnWriteDMA;
		public event SendHDLCFrameEventHandler? OnSendHDLCFrame;
		public event SetInterruptBitEventHandler? OnSetInterruptBit;

		#region DMA MODULE registers
		/// <summary>
		/// Outbound data buffer for accumulating frame data across multiple DCBs
		/// </summary>
		public List<byte> OutboundBuffer { get; internal set; } = new List<byte>();

		/// <summary>
		/// Current BufferDescription for TRANSMIT
		/// </summary>
		public DCB? TX_DCB { get; internal set; } = null;

		/// <summary>
		/// Pointer to the FIRST ListPointer for TRANSMIT
		/// Set by DMA RECEIVER START command or DMA TRANSMITTER START command
		/// </summary>
		public uint TX_ListPointer { get; internal set; } = 0;

		/// <summary>
		/// WHat are our current buffer offset from the start of the TX buffer (not in bytes, but by numbers of blocks)
		/// </summary>
		public ushort TX_ListPointer_Offset { get; internal set; } = 0;
		/// <summary>
		/// Current BufferDescription for RECEIVE
		/// </summary>
		public DCB? RX_DCB { get; internal set; } = null;

		/// <summary>
		/// Pointer to the FIRST ListPointer for RECEIVE
		/// Set by DMA RECEIVER START command or DMA RECEIVER CONTINUE command
		/// </summary>
		public uint RX_ListPointer { get; internal set; } = 0;

		/// <summary>
		/// WHat are our current buffer offset from the start of the RX buffer (not in bytes, but by numbers of blocks)
		/// </summary>
		public ushort RX_ListPointer_Offset { get; internal set; } = 0;

		/// <summary>
		/// Used in DMA Burst mode
		/// </summary>
		public HDLCFrame HDLCReceiveFrame { get; internal set; }  = new HDLCFrame();


		/// <summary>
		///  ParameterBuffer keeps all the parameters set by HDLC DMA configuration
		/// </summary>
		public ParameterBuffer Parameters { get; internal set; }  = new();


		#region DMA State machine
		internal DmaEngineSenderState dmaSenderState = DmaEngineSenderState.STOPPED;
		internal DmaBlockSendState dmaSendBlockState = DmaBlockSendState.IDLE;

		/// <summary>
		///  Wait ticks for DMA sender state machine delays
		/// </summary>
		internal int dmaWaitTicks = -1;


		/// <summary>
		/// TCP Burst mode (bypassing COM5025 for faster transfer)
		/// </summary>
		internal bool BurstMode { get; set; } = false;
		#endregion


		#endregion DMA MODULE registers


		#region TX
		internal void SetTXPointer(uint list_pointer, int offset)
		{
			TX_ListPointer = list_pointer;
			TX_ListPointer_Offset = 0;

			LoadTXBuffer();

		}

		internal void DebugTXFrames()
		{
			for (int i =0; i<100; i++)
			{
				uint addr = TX_ListPointer + (uint)(i * 4);

				ushort keyValue = (ushort)DMARead(addr);
				KeyFlags key = (KeyFlags)(keyValue & 0xFF00);

				KeyFlags key_small = (KeyFlags)(keyValue & 0x00FF);

				Log($"TXAnalyse: Offset={i} LP=0x{addr:X6} Key=0x{keyValue:X4} ({key} {key_small})");

				if (( keyValue == 0) ||(key == KeyFlags.NewListPointer))
				{
					Log($"TXAnalyse: End of frames at offset={i}");
					return;
				}
			}
		}

		/// <summary>
		/// Reload current TX buffer description from ND-100 memory
		/// </summary>
		internal void LoadTXBuffer()
		{
			TX_DCB = LoadBufferDescription(TX_ListPointer, TX_ListPointer_Offset, false);
		}


		/// <summary>
		/// Load next TX buffer description
		/// Return true if we have reached a block to be transmitted
		/// </summary>
		/// <returns></returns>
		internal bool LoadNextTXBuffer()
		{
			TX_ListPointer_Offset++;
			LoadTXBuffer();

			return TX_DCB?.Key == KeyFlags.BlockToBeTransmitted;
		}
		
		/// <summary>
		/// DMA Back to ND machine with a new KEY telling that this buffer has been sent
		///
		/// Update KEY = AlreadyTransmittedBlock
		/// </summary>		
		internal void MarkBufferSent()
		{

			ushort data = 0;
			if (TX_DCB.Key == KeyFlags.BlockToBeTransmitted)
			{
				data = (ushort)(TX_DCB.DataFlowCost | (ushort)(KeyFlags.AlreadyTransmittedBlock));
				TX_DCB.KeyValue = data;

				Log($"MarkBufferSent: Status Word = 0x{data:X4}");
				DMAWrite(TX_DCB.BufferAddress, data);
			}
		}



		#endregion TX

		#region RX
		internal void SetRXPointer(uint list_pointer, int offset)
		{
			RX_ListPointer = list_pointer;
			RX_ListPointer_Offset = (ushort)offset;

			LoadRXBuffer();
		}

		/// <summary>
		/// Reload current RX buffer description from ND-100 memory
		/// </summary>
		internal void LoadRXBuffer()
		{
			RX_DCB = LoadBufferDescription(RX_ListPointer, RX_ListPointer_Offset, true);
		}

		/// <summary>
		/// Load next RX buffer description
		/// Return true if we have reached an empty receiver block
		/// </summary>
		/// <returns></returns>
		internal bool LoadNextRXBuffer()
		{
			RX_ListPointer_Offset++;

			if (RX_ListPointer_Offset> 128) // Assuming max 128 buffers in the list (for safefty)
			{
				// We have reached the end of the RX buffer list
				// We need to restart from the beginning
				RX_ListPointer_Offset = 0;
			}

			LoadRXBuffer();

			return RX_DCB?.Key == KeyFlags.EmptyReceiverBlock;
		}

		/// <summary>
		/// Is the next RX buffer valid (Contains EmptyReceiverBlock)
		/// </summary>
		/// <returns></returns>
		internal bool IsNextRXbufValid() 
		{
			uint listpointer = RX_ListPointer + (uint)((RX_ListPointer_Offset+1) * 4);

			ushort KeyValue = (ushort)DMARead(listpointer);              // INTEGER LKEY   % LIST KEY

			KeyFlags key = (KeyFlags)(KeyValue & 0x00FF);

			return (key == KeyFlags.EmptyReceiverBlock);
		}
		/// <summary>
		/// DMA Back to ND machine with a new KEY telling that this buffer has been received
		///
		/// Update KEY = FullReceiverBlock
		/// </summary>		
		internal void MarkBufferReceived(byte RXStatus)
		{
			if (RX_DCB == null) return;

			if (RX_DCB.Key == KeyFlags.EmptyReceiverBlock)
			{
				// Set RCOST in the low 8 bits, its actually the ReceiverStatusRegister
				RX_DCB.KeyValue = (ushort)((RX_DCB.KeyValue & 0xFF00) | RXStatus);

				// Mark block as DONE (ie filled up)
				RX_DCB.KeyValue |= (ushort)(KeyFlags.BLOCK_DONE_BIT);

				DMAWrite(RX_DCB.BufferAddress, RX_DCB.KeyValue);				



#if DMA_DEBUG

				string flags = "";

				if (RX_DCB.HasRSOMFlag) flags += "RSOM ";
				if (RX_DCB.HasREOMFlag) flags += "REOM ";


				Log("--------------------------------------------------------------------------");
				Log($"DMA Buffer received           : 0x{RX_DCB.BufferAddress:X6} Flags: {flags}");
				Log($"ListPointer                   : 0x{RX_DCB.ListPointer:X6}  lp[0x{RX_ListPointer:X6}] offset[{RX_ListPointer_Offset}]");
				Log("");
				Log($"KeyValue                      : 0x{DMARead(RX_DCB.BufferAddress + 0):X4} {RX_DCB.Key} [MarkBufferReceived]");
				Log($"ByteCount                     : 0x{DMARead(RX_DCB.BufferAddress + 1):X4}");
				Log($"MostAddress                   : 0x{DMARead(RX_DCB.BufferAddress + 2):X4}");
				Log($"LeastAddress                  : 0x{DMARead(RX_DCB.BufferAddress + 3):X4}");
				Log($"DMA bytes written             : {RX_DCB.dma_bytes_written}");

				if (RX_DCB.dma_bytes_written > 0) // description.dma_bytes_written ByteCount ?
				{
					RX_DCB.dmaAddress = RX_DCB.DataMemoryAddress;

					string bytes = "";
					for (int i = 0; i < RX_DCB.dma_bytes_written; i++)
					{
						bytes = bytes + $"0x{ReadNextByteDMA(true):X2} ";
					}

					Log($"Received block [{RX_DCB.BufferAddress:X6}:{RX_ListPointer_Offset}]: {bytes} [RSOM:{RX_DCB.HasRSOMFlag}] [REOM:{RX_DCB.HasREOMFlag}]");
				}
				Log("--------------------------------------------------------------------------");
#endif


			}
		}



		#endregion

		/// <summary>
		/// Load a buffer description from a given memory address
		/// If the Key=NewListPointer we automatically read the next buffer description from the original list pointer
		/// 
		/// ==>  If the DMA processor reaches the NEW LIST POINTER and than Full Receiver Block, a LIST EMPTY interrupt is generated.
		/// 
		/// </summary>
		/// <param name="address"></param>
		/// <returns></returns>
		internal DCB LoadBufferDescription(uint list_pointer, ushort offset, bool isRX = false)
		{

			if (list_pointer == 0) return null;

			uint actualListPointer = list_pointer + (uint)(offset * 4);

			DCB description = new DCB();
			description.ListPointer = actualListPointer;
			description.OffsetFromLP = offset;

			

			
			description.BufferAddress = actualListPointer;

			uint address = actualListPointer;
			description.KeyValue = (ushort)DMARead(address++);              // INTEGER LKEY   % LIST KEY

			if (description.KeyValue != 0)
			{
				description.ByteCount = (ushort)DMARead(address++);             // address+1: ByteCount (in BYTES)
				description.MostAddress = (ushort)DMARead(address++);           // address+2: Buffer address MSW
				description.LeastAddress = (ushort)DMARead(address++);          // address+3: Buffer address LSW
				
			}

			string disp = "";
			if (offset == 0)
			{
				// First buffer in the list, so we use Displacement1
				description.Displacement = (ushort)Parameters.Displacement1;
				disp = "1";
			}
			else
			{
				// All other buffers in the list, so we use Displacement2
				description.Displacement = (ushort)Parameters.Displacement2;
				disp = "2";
			}


#if DMA_DEBUG
			string mode = isRX ? "RX" : "TX";
			Log("--------------------------------------------------------------------------");
			Log($"LoadBufferDescription {mode}      : 0x{actualListPointer:X6} lp[0x{list_pointer:X6}] offset[{offset}]");
			Log("");
			Log($"KeyValue                      : 0x{description.KeyValue:X4} {description.Key}");

			if (description.Key != KeyFlags.EmptyReceiverBlock)
			{
				Log($"ByteCount                     : 0x{description.ByteCount:X4} ");
			}
			Log($"MostAddress                   : 0x{description.MostAddress:X4} ");
			Log($"LeastAddress                  : 0x{description.LeastAddress:X4} ");
			Log($"Displacement                  : {description.Displacement} : Use Displacement{disp} ");

			if ((description.ByteCount > 0) && (description.Key == KeyFlags.BlockToBeTransmitted))
			{
				description.dmaAddress = description.DataMemoryAddress;

				string bytes = "";
				for (int i = 0; i < description.ByteCount; i++)
				{
					bytes = bytes + $"0x{ReadNextByteDMA(isRX):X2} ";
				}

				Log($"DATA: {bytes}");
			}
			Log("--------------------------------------------------------------------------");
#endif


#if DEBUG_DETAIL
			if (description.Key == KeyFlags.EmptyReceiverBlock)
			{
				Log($"Loading Buffer from 0x{list_pointer:X8} Key={description.Key}");
			}
			else
			{
				Log($"Loading Buffer from 0x{list_pointer:X8} Key={description.Key} DataFlowCost=0x{description.DataFlowCost:X8} ByteCount={description.ByteCount} RSOMFlag={description.HasRSOMFlag} REOMFlag={description.HasREOMFlag}");
			}
#endif

			// Set up the DMA helpers

			description.dmaAddress = description.DataMemoryAddress;
			description.dma_read_data = -1;
			description.dma_bytes_written = 0;
			description.dma_bytes_read = 0;

			return description;
		}
		internal void SetBurstMode(bool burstEnabled)
		{
			BurstMode = burstEnabled;
		}

		void Clear()
		{
			TX_DCB = null;
			TX_ListPointer = 0;
			TX_ListPointer_Offset = 0;
			RX_DCB = null;
			RX_ListPointer = 0;
			RX_ListPointer_Offset = 0;
			Parameters.Clear();
		}

		private void DMAWrite(uint address, ushort data)
		{
			if (OnWriteDMA != null)
			{
				OnWriteDMA(address, data);
			}
		}

		private int DMARead(uint address)
		{
			int data = 0;
			if (OnReadDMA != null)
			{
				OnReadDMA(address, out data);
			}
			return data;
		}

		private void Log(string strMessage)
		{
			Logger.Log($"DMACB: {strMessage}",Logger.LogLevel.Device);
		}

		#region Helper for DMA read and write (byte oriented reads)

		/// <summary>
		/// Read next byte via DMA from ND-100 memory
		/// </summary>
		/// <param name="description">Buffer description keeping all neccesary meta-information</param>
		/// <returns>byte of data read from ND-100 memory</returns>
		internal byte ReadNextByteDMA(bool isRx = true)
		{
			DCB description;
			if (isRx)
			{
				description = RX_DCB;
			}
			else
			{
				description = TX_DCB;
			}

			if (description is null) return 00;
			
			byte data = 0;

			// Adjust for "displacement"
			if ((description.dma_bytes_read == 0) && (description.Displacement > 0))
			{
				// We need to skip "displacement" number of bytes
				description.dma_bytes_read += description.Displacement;

				// and we need to calculate the new dmaAddress
				description.dmaAddress += (uint)(description.Displacement / 2);
			}


			// Start reading
			if ((description.dma_bytes_read % 2) == 0) //  0 ==> is even (High byte), 1 ==> odd (Low byte)
			{
				description.dma_read_data = (ushort)DMARead(description.dmaAddress);
				data = (byte)(description.dma_read_data >> 8);

			}
			else
			{
				if (description.dma_read_data == -1) // data not read (might happen because of displacement)
					description.dma_read_data = (ushort)DMARead(description.dmaAddress);

				data = (byte)(description.dma_read_data & 0xFF);

				description.dmaAddress++;
			}
			description.dma_bytes_read++;

			//Log($"DMA Read: 0x{data:X2} #{description.dma_bytes_read}"); // Debug
			return data;
		}


		/// <summary>
		/// Write next byte via DMA to ND-100 memory (which is WORD size)
		/// </summary>
		/// <param name="description">Buffer description keeping all neccesary meta-information</param>
		/// <param name="data">Byte of data to write to ND-100 word memory</param>
		internal void WriteNextByteDMA(byte data, bool isRx = true)
		{
			ushort dma_write_data;
			DCB description;
			if (isRx)
			{
				description = RX_DCB;
			}
			else
			{
				description = TX_DCB;
			}

#if RX_BLAST_LOGGING
			Log($" DMA_WRITE_ENTRY: data=0x{data:X2}, bytes_written={description.dma_bytes_written}, displacement={description.Displacement}, dmaAddress=0x{description.dmaAddress:X8}");
#endif

#if DEBUG_DETAIL
			Log($"DMA Write: 0x{data:X2}  #{description.dma_bytes_written}"); //Debug
#endif
			// Adjust for "displacement"
			if ((description.dma_bytes_written == 0) && (description.Displacement > 0))
			{
				// We need to skip "displacement" number of bytes
				description.dma_bytes_written += description.Displacement;

				// and we need to calculate the new dmaAddress
				description.dmaAddress += (uint)(description.Displacement / 2);

#if RX_BLAST_LOGGING
				Log($"DMA_WRITE_DISPLACEMENT: skipped {description.Displacement} bytes, new_bytes_written={description.dma_bytes_written}, new_dmaAddress=0x{description.dmaAddress:X8}");
#endif
			}


			// Start writing
			if ((description.dma_bytes_written % 2) == 0) // ==0 is even, 1 ==odd
			{
				int memData = DMARead(description.dmaAddress);
				dma_write_data = (ushort)((memData & 0x00FF) | ((data & 0xFF) << 8));
				DMAWrite(description.dmaAddress, dma_write_data);
			}
			else
			{
				int memData = DMARead(description.dmaAddress);
				dma_write_data = (ushort)((memData & 0xFF00) | (data & 0xFF));
				DMAWrite(description.dmaAddress, dma_write_data);

				description.dmaAddress++;
			}
			description.dma_bytes_written++;

			// Update DCB with bytes written as "ByteCount"
			DMAWrite(description.ListPointer + 1, (ushort)description.dma_bytes_written);

#if RX_BLAST_LOGGING
			Log($"DMA_WRITE_EXIT: bytes_written={description.dma_bytes_written}, final_dmaAddress=0x{description.dmaAddress:X8}");
#endif
		}
		



		#endregion Helper for DMA read and write (byte oriented reads)
	}
}
