#define DEBUG_DETAIL
#define DEBUG_DETAIL_PLUS_DESCRIPTION

#define DMA_DEBUG
#define RX_BLAST_LOGGING


using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Emulated.HW.Common.Network;
using Emulated.HW.SMC.COM.COM5025;
using Emulated.Lib;

namespace Emulated.HW.ND.CPU.NDBUS.HDLC
{
	internal class DMAReceiver
	{
		public event SetInterruptBitEventHandler? OnSetInterruptBit;

		private Registers regs;
		private DMAControlBlocks dmaRegs;
		private COM5025 com5025;

		public DMAReceiver(Registers r, DMAControlBlocks dr, COM5025 c)
		{
			regs = r;
			dmaRegs = dr;
			com5025 = c;
		}

		internal void SetReceiverState()
		{
			// Called by CommandReceiverStart and CommandReceiverContinue
			// Enable the HDLC receiver and ensure DMA is ready for incoming data

			// Enable receiver DMA - this allows the receiver to process incoming data
			regs.ReceiverTransferControl |= RTCBits.EnableReceiverDMA;

			// Enable the HDLC receiver hardware
			EnableHDLCReceiver(true);

			// Clear any previous receiver overrun or error states
			regs.ReceiverTransferStatus &= ~(RTSBits.ReceiverOverrun | RTSBits.ListEmpty);

			// Set receiver as active and ready to receive
			regs.ReceiverTransferStatus |= RTSBits.ReceiverActive;

			// Find the first empty receive buffer if not already loaded
			if (dmaRegs.RX_DCB == null)
			{
				dmaRegs.LoadRXBuffer();
			}

			// Ensure we have a valid empty buffer to start receiving into
			if (dmaRegs.RX_DCB?.Key != KeyFlags.EmptyReceiverBlock)
			{
				FindNextReceiveBuffer();
			}

#if DMA_DEBUG
			Log($"DMA SetReceiverState: EnableReceiverDMA={regs.ReceiverTransferControl.HasFlag(RTCBits.EnableReceiverDMA)}, " +
				$"ReceiverActive={regs.ReceiverTransferStatus.HasFlag(RTSBits.ReceiverActive)}, " +
				$"BufferKey={dmaRegs.RX_DCB?.Key}");
#endif
		}



		/// <summary>
		/// Will get called when we have data in the ReceiverDataBuffer,   ( which sets the flag ReceiverTransferStatusFlags.DataAvailable is set
		/// (Require that the ReceiverTransferStatusFlags.EnableReceiverDMA is set)
		///
		/// ALso gets called when Transceiver is enabled, IF there is data available in the COM5025 
		/// </summary>
		internal void DataAvailableFromCOM5025()
		{
			byte data = com5025.Read((byte)RegistersByte.ReceiverDataBuffer);
			ReceiveByteFromCOM50250(data);
		}

		/// <summary>
		/// Set DMA flag, and raise an interrupt on level 13 (if its RX)
		/// </summary>
		/// <param name="flag"></param>
		/// <param name="isTx"></param>
		internal void SetRXDMAFlag(RTSBits flag)
		{

			string irq_reason = "";

			if (dmaRegs.BurstMode)
			{
				// TODO: ANALYSE if this should be set always?
				flag |= RTSBits.SignalDetector | RTSBits.DataSetReady;
			}

			if (!dmaRegs.IsNextRXbufValid())
			{
				// If we have no valid next RX buffer, we cannot receive any more data
				flag |= RTSBits.ListEmpty;
			}

			regs.ReceiverTransferStatus |= flag;

			if (flag.HasFlag(RTSBits.ListEmpty))
			{
				// List empty always triggers DMA request
				regs.ReceiverTransferStatus |= RTSBits.DMAModuleRequest;
				irq_reason += "ListEmpty ";
			}


			if (regs.ReceiverTransferControl.HasFlag(RTCBits.BlockEndIE) && regs.ReceiverTransferStatus.HasFlag(RTSBits.BlockEnd))
			{
				regs.ReceiverTransferStatus |= RTSBits.DMAModuleRequest;
				irq_reason += "BlockEnd ";
			}

			if (regs.ReceiverTransferControl.HasFlag(RTCBits.FrameEndIE) && regs.ReceiverTransferStatus.HasFlag(RTSBits.FrameEnd))
			{
				regs.ReceiverTransferStatus |= RTSBits.DMAModuleRequest;
				irq_reason += "FrameEnd ";
			}

			if (regs.ReceiverTransferControl.HasFlag(RTCBits.ListEndIE) && regs.ReceiverTransferStatus.HasFlag(RTSBits.ListEnd))
			{
				regs.ReceiverTransferStatus |= RTSBits.DMAModuleRequest;
				irq_reason += "ListEnd ";
			}

#if RX_BLAST_LOGGING
			Log($"RX_FLAGS_SET: {flag} | Total_Status={regs.ReceiverTransferStatus} | IRQ_Reason=[{irq_reason.Trim()}]");
			Log($"RX_CONTROL_STATE: DMAModuleIE={regs.ReceiverTransferControl.HasFlag(RTCBits.DMAModuleIE)}, DMAModuleRequest={regs.ReceiverTransferStatus.HasFlag(RTSBits.DMAModuleRequest)}");
#endif

#if DMA_DEBUG
			Log("--------------------------------------------------------------------------");
			Log($"DMA SetRXDMAFlag : {flag} ({(ushort)flag}) IRQ[{irq_reason}]");
			Log($"ReceiverStatus   : {regs.ReceiverTransferStatus} ({(ushort)regs.ReceiverTransferStatus})");
			Log($"ReceiverEnable   : {regs.ReceiverTransferControl}");
			Log("--------------------------------------------------------------------------");
#endif

			if (regs.ReceiverTransferControl.HasFlag(RTCBits.DMAModuleIE) && regs.ReceiverTransferStatus.HasFlag(RTSBits.DMAModuleRequest))
			{
#if RX_BLAST_LOGGING
				Log($"RX_INTERRUPT: Triggering IRQ 13 for {irq_reason.Trim()}");
#endif
				SetInterruptBit(13);
			}
			else
			{
#if RX_BLAST_LOGGING
				Log($" RX_NO_INTERRUPT: IRQ 13 NOT triggered - DMAModuleIE={regs.ReceiverTransferControl.HasFlag(RTCBits.DMAModuleIE)}, DMAModuleRequest={regs.ReceiverTransferStatus.HasFlag(RTSBits.DMAModuleRequest)}");
#endif
			}


		}


		private void EnableHDLCReceiver(bool enable = true)
		{

			if (enable)
				regs.ReceiverTransferControl |= RTCBits.EnableReceiver;
			else
				regs.ReceiverTransferControl &= ~RTCBits.EnableReceiver;

			com5025.SetInputPinValue(SignalPinIn.RXENA, regs.ReceiverTransferControl.HasFlag(RTCBits.EnableReceiver));
		}



		bool FindNextReceiveBuffer()
		{
			if (regs == null) return false;


			if ((dmaRegs.RX_DCB != null) && (dmaRegs.RX_DCB.Key == KeyFlags.EmptyReceiverBlock))
			{
				return true;
			}

			// Scan from start.			
			while (true)
			{
				// Load next RX buffer description returns true if we found an empty receive buffer
				if (!dmaRegs.LoadNextRXBuffer()) break;

				if (dmaRegs.RX_DCB?.Key == KeyFlags.FullReceiverBlock) continue;

				// We have 0 or something else in the Key (so not a known RX buffer key state)
				break;
			}


			if (dmaRegs.RX_DCB == null) return false;

			// Loop around to start of list.
			if (dmaRegs.RX_DCB.Key == KeyFlags.NewListPointer)
			{

				dmaRegs.RX_ListPointer_Offset = 0;
				dmaRegs.LoadRXBuffer();

				if (dmaRegs.RX_DCB.Key == KeyFlags.FullReceiverBlock)
				{

					// ND-12.018.01 - Page II-3-13:
					//
					// This looping in receiver list requires that driver software is fast enough to reset the list entries.
					// If the DMA processor reaches the NEW LIST POINTER and than Full Receiver Block, a LIST EMPTY interrupt is generated.
					// (See Receiver TransferStatus Register).

					SetRXDMAFlag(RTSBits.ListEmpty);

					return false; // we are done here, received data will be lost
				}
			}

			if (dmaRegs.RX_DCB.Key != KeyFlags.EmptyReceiverBlock)
			{
#if RX_BLAST_LOGGING
				Log($"BUFFER_FIND_FAILED: Buffer key is {dmaRegs.RX_DCB?.Key}, not EmptyReceiverBlock");
#endif

				dmaRegs.RX_ListPointer_Offset = 0;
				return false;
			}

#if RX_BLAST_LOGGING
			Log($" BUFFER_FIND_SUCCESS: Found empty buffer at offset [{dmaRegs.RX_ListPointer_Offset}]");
#endif

			return true;
		}



		public enum DmaReceiveStatus
		{
			OK,
			FAILED,
			BUFFER_FULL,
			NO_BUFFER
		}

		/// <summary>
		/// Receive a single byte via DMA into the current receive buffer
		/// </summary>
		/// <param name="data"></param>
		/// <returns>True if was written to buffer/MEMORY - FALSE is data lost</returns>
		DmaReceiveStatus DMA_ReceiveDataBufferByte(byte data)
		{
			if (dmaRegs.RX_DCB == null) return DmaReceiveStatus.NO_BUFFER;

			// Check if we have a valid buffer description
			if (dmaRegs.RX_DCB.Key == KeyFlags.EmptyReceiverBlock)
			{
				// DMA the data to ND-100 memory (via the current buffer description)
				dmaRegs.WriteNextByteDMA(data, true);
			}

			if (dmaRegs.RX_DCB.dma_bytes_written >= dmaRegs.Parameters.MaxReceiverBlockLength)
			{
#if RX_BLAST_LOGGING
				Log($" BUFFER_OVERFLOW: offset[{dmaRegs.RX_ListPointer_Offset}] full ({dmaRegs.RX_DCB.dma_bytes_written}/{dmaRegs.Parameters.MaxReceiverBlockLength}), switching buffers");
#endif

				//Buffer filled up ? Mark it as complete woth RSOM (no REOM) and find next receive buffer				
				byte RXStatus = 0x01;  // 01=rsom, 02=reom
				dmaRegs.MarkBufferReceived(RXStatus);

				// Tell ND that the Block has ended (but FRAME is not yet ended)
				SetRXDMAFlag(RTSBits.BlockEnd | RTSBits.ReceiverActive | RTSBits.SyncFlagReceived | RTSBits.DataAvailable); // | RTSBits.FrameEnd);

				dmaRegs.RX_DCB = null;
				return DmaReceiveStatus.BUFFER_FULL; // Buffer is full, need to find next buffer
			}

			return DmaReceiveStatus.OK;
		}
		/// <summary>
		/// Bypasses COM3025 chip for direct DMA receive of databuffers via TCP
		///
		/// Only called in DMA_BLAST mode
		/// </summary>
		/// <param name="tcp_data"></param>
		/// <param name="tcp_packet_length"></param>
		internal void DMABlastReceiveDataBuffer(byte[] tcp_data, int tcp_packet_length)
		{

#if RX_BLAST_LOGGING
			Log($" TCP_RX: {tcp_packet_length} bytes received, EnableDMA={regs.ReceiverTransferControl.HasFlag(RTCBits.EnableReceiverDMA)}");
			var hexData = string.Join(" ", tcp_data.Take(Math.Min(16, tcp_packet_length)).Select(b => $"0x{b:X2}"));
			Log($" TCP_DATA: {hexData}{(tcp_packet_length > 16 ? "..." : "")}");
#endif

			// Check if receiver DMA is enabled before processing data in BLAST mode
			if (!regs.ReceiverTransferControl.HasFlag(RTCBits.EnableReceiverDMA))
			{
				//Log($"TCP_RX: IGNORED - Receiver DMA disabled");
				// CRITICAL FIX: Clear frame to prevent contamination when DMA is disabled
				Log($"HDLCFrame cleared due to receiver DMA disabled");
				dmaRegs.HDLCReceiveFrame.Clear();
				return; // Ignore data when receiver DMA is disabled
			}

			if (dmaRegs.RX_DCB is null)
			{
				// if its still null, we have an issue
				Log($"RX_DCB: No RX buffer description loaded - DATA LOST");
				// CRITICAL FIX: Clear frame to prevent contamination when no buffer available
				Log($"HDLCFrame cleared due to no RX buffer");
				dmaRegs.HDLCReceiveFrame.Clear();
				return; // No RX buffer description loaded ??
			}


			try
			{
				for (int i = 0; i < tcp_packet_length; i++)
				{
					if (dmaRegs?.RX_DCB.Key != KeyFlags.EmptyReceiverBlock)
					{
						if (dmaRegs.LoadNextRXBuffer())
						{
#if RX_BLAST_LOGGING
							Log($"RX NEXT_BUFFER: Switched buffer. Offset[{dmaRegs.RX_ListPointer_Offset}]");
#endif
						}
					}


					if ((dmaRegs.RX_DCB == null) || (dmaRegs.RX_DCB.Key != KeyFlags.EmptyReceiverBlock))
					{
#if RX_BLAST_LOGGING
						Log($" BUFFER_EXHAUSTED: No more receive buffers available -  REMAINING DATA WILL BE LOST (lost bytes:{tcp_packet_length - i})");
#endif
						Log($"HDLCFrame cleared");
						dmaRegs.HDLCReceiveFrame.Clear();
						SetRXDMAFlag(RTSBits.ReceiverOverrun);
						return; // no more rx buffers exit and lose data
					}

					// Send byte to HDLC receiver temporary buffer
					HDLCFrame.HDLCReceiveState preState = dmaRegs.HDLCReceiveFrame.ReceiveState; // debugging
					dmaRegs.HDLCReceiveFrame.AddByte(tcp_data[i]);
					//Log($"TCP: Adding byte 0x{tcp_data[i]:X2} to buffer. Buffer length={dmaRegs.HDLCReceiveFrame.BufferLength} Buffer state after add: State={dmaRegs.HDLCReceiveFrame.ReceiveState} State was={preState} FCSValid={dmaRegs.HDLCReceiveFrame.FCSValid}");

					if (dmaRegs.HDLCReceiveFrame.ReceiveState == HDLCFrame.HDLCReceiveState.HDLC_STATE_FRAME_COMPLETE)
					{
						var frame = dmaRegs.HDLCReceiveFrame.GetFrameBytesNoFCS(); // Get all bytes (excluding FCS and FLAGS)

#if RX_BLAST_LOGGING
						Log($" HDLC_FRAME_COMPLETE: FCS_Valid={dmaRegs.HDLCReceiveFrame.FCSValid}, FCS=0x{dmaRegs.HDLCReceiveFrame.FCS:X4}Frame_Size={frame.Length}, DCB offset={dmaRegs.RX_ListPointer_Offset}");
#endif

						if (dmaRegs.HDLCReceiveFrame.FCSValid)
						{
							bool needBufferNow = i < (tcp_packet_length - 1); // More data coming in this TCP packet?
							bool frameWriteSuccess = true;

#if RX_BLAST_LOGGING
							Log($" FRAME_TO_BUFFER: Writing {frame.Length - 2} bytes to buffer {dmaRegs.RX_ListPointer_Offset}, MoreDataComing={needBufferNow}");

							StringBuilder sb = new StringBuilder();
							sb.Append($" FRAME_DATA: ");
#endif

							for (int j = 0; j < frame.Length; j++)
							{
#if RX_BLAST_LOGGING
								sb.Append($"0x{frame[j]:X2} ");
#endif
								DmaReceiveStatus rstat = DMA_ReceiveDataBufferByte(frame[j]);

								switch (rstat)
								{
									case DmaReceiveStatus.OK:
										break;
									case DmaReceiveStatus.FAILED:
										frameWriteSuccess = false;
										break;
									case DmaReceiveStatus.BUFFER_FULL:
										// Need to find next buffer on next byte to write
										if (j - (frame.Length - 1) > 0)
										{
											Log("Need to find new buffer!)");
											// we need to find a new buffer to the next bytes
											if (!FindNextReceiveBuffer())
											{
												Log("Unable to find new empty buffer for receive");
												SetRXDMAFlag(RTSBits.ListEmpty | RTSBits.ReceiverOverrun);
												frameWriteSuccess = false;

												break;
											}
										}

										break;
									case DmaReceiveStatus.NO_BUFFER:
										frameWriteSuccess = false;
										break;
									default:
										break;
								}

								if (frameWriteSuccess == false)
								{
#if RX_BLAST_LOGGING
									Log($" BUFFER_WRITE_FAILED: Frame byte {j} write failed - PARTIAL DATA LOST");
#endif
									break;
								}
							}

#if RX_BLAST_LOGGING
							Log(sb.ToString());

							if (frameWriteSuccess)
							{
								Log($" FRAME_WRITE_SUCCESS: All {frame.Length} bytes written successfully");
							}
							else
							{
								Log($" FRAME_WRITE_PARTIAL: Frame write incomplete - DATA LOST");
							}
#endif

							if (frameWriteSuccess)
							{
								dmaRegs.MarkBufferReceived(0x03); // Mark buffer received with RSOM and REOM
								com5025.SetReceiverStatus((ReceiverStatusFlags)0x03); // Update receiver status register RSOM and REOM in COM5025 so SINTRAN can see it

#if RX_BLAST_LOGGING
								Log($" BUFFER_COMPLETE: Buffer offset [{dmaRegs.RX_ListPointer_Offset}] marked received, setting flags");
#endif

								// SetRXDMAFlag(RTSBits.FrameEnd | RTSBits.BlockEnd | RTSBits.DataAvailable | RTSBits.StatusAvailable | RTSBits.SyncFlagReceived);

								SetRXDMAFlag(RTSBits.FrameEnd | RTSBits.BlockEnd | RTSBits.DataAvailable | RTSBits.ReceiverActive | RTSBits.SyncFlagReceived);

							}
						}
						else
						{
#if RX_BLAST_LOGGING
							Log($" HDLC_FRAME_ERROR: FCS validation failed - FRAME DISCARDED ({frame.Length} bytes lost)");
							var errorData = string.Join(" ", frame.Take(Math.Min(16, frame.Length)).Select(b => $"0x{b:X2}"));
							Log($" ERROR_FRAME_DATA: {errorData}{(frame.Length > 16 ? "..." : "")}");
#endif
							// Flag error
							Debug.WriteLine("FRAME HAD FCS ERROR!!!!");

							// TODO: Set error flags somewhere ?
						}
						Log($"HDLCFrame cleared");
						dmaRegs.HDLCReceiveFrame.Clear();
					}
				}

#if RX_BLAST_LOGGING
				Log($" HDLC_PROCESS_END: Processed {tcp_packet_length} bytes");
#endif
			}
			catch (Exception ex)
			{
#if RX_BLAST_LOGGING
				Log($" HDLC_EXCEPTION: {ex.Message} - DATA PROCESSING FAILED");
#endif
				Debug.WriteLine($"Ex {ex}");

				// CRITICAL FIX: Clear the HDLC frame to prevent contamination of next frame
				Log($"HDLCFrame cleared due to exception");
				dmaRegs.HDLCReceiveFrame.Clear();
			}

		}


		/// <summary>
		/// Will get called when MPCC has an updated status an sets pin 7 RSA to high (Receiver Status Available)
		/// (Require that the ReceiverTransferStatusFlags.EnableReceiverDMA is set)
		/// </summary>
		internal void StatusAvailableFromCOM5025()
		{
			// Clear flag to make sure we dont get called again by a mistake
			regs.ReceiverTransferStatus &= ~RTSBits.StatusAvailable;


			if (dmaRegs.BurstMode)
			{
				// In burst mode we do not use the COM5025 status register
				return;
			}


			ushort RXStatus = com5025.Read((byte)RegistersByte.ReceiverStatusRegister);
			ReceiverStatusFlags RXFlags = (ReceiverStatusFlags)(RXStatus << 8);


			// Check if we have received REOM (Receiver End Of Message)
			if (RXFlags.HasFlag(ReceiverStatusFlags.REOM))
			{
#if DEBUG_DETAIL
				Log($"DMA REOM position = {dmaRegs.RX_DCB?.dma_bytes_written}, RXFlags={RXFlags}");
#endif
				// End of message.


				// Check if we have a valid buffer description ready for receive
				if (dmaRegs.RX_DCB?.Key != KeyFlags.EmptyReceiverBlock)
				{
#if DEBUG_DETAIL
					// Buffer receive is complete, but we have no place to put the data
					SetRXDMAFlag(RTSBits.ListEmpty | RTSBits.ReceiverOverrun); // was ListEmpty
					Log("Receiver overrun - No valid buffer description.");
#endif
					return;
				}


				dmaRegs.MarkBufferReceived((byte)RXStatus);

				//SetRXDMAFlag(ReceiverStatusBits.FrameEnd);
				SetRXDMAFlag(RTSBits.FrameEnd | RTSBits.BlockEnd | RTSBits.ReceiverActive | RTSBits.SyncFlagReceived);


				// Find next receiver buffer description
				dmaRegs.LoadNextRXBuffer();

				// New List Pointer makes the list wrap to the start
				if (dmaRegs.RX_DCB.Key == KeyFlags.NewListPointer)
				{
					dmaRegs.SetRXPointer(dmaRegs.RX_DCB.ListPointer, 0);

					SetReceiverState(); // Update receiver state!

					// Check if the new buffer is valid, if not we are list empty
					if (dmaRegs.RX_DCB.Key == KeyFlags.FullReceiverBlock)
					{
						// ND-12.018.01 - Page II-3-13:
						//
						// This looping in receiver list requires that driver software is fast enough to reset the list entries.
						// If the DMA processor reaches the NEW LIST POINTER and than Full Receiver Block, a LIST EMPTY interrupt is generated.
						// (See Receiver TransferStatus Register).

						SetRXDMAFlag(RTSBits.ListEmpty);

						// TODO: Analyse if we should Disable receiver? 
					}
				}

			}

			if (RXFlags.HasFlag(ReceiverStatusFlags.ROR))
			{

			}

		}

		/// <summary>
		/// Receive  the next character from the COM5025 chip
		/// If the block is ended:
		/// * Mark block received, and DMA back to ND the updated KEY
		/// * tell ND via Interrupt that the Block has ended
		/// </summary>
		internal void ReceiveByteFromCOM50250(byte data)
		{

			if (dmaRegs.BurstMode)
			{
				// In burst mode we do not use the COM5025 to transmit data, we send it directly via TCP for speed
				return;
			}

			if (dmaRegs.RX_DCB.dma_bytes_written >= dmaRegs.Parameters.MaxReceiverBlockLength)
			{
				//Buffer filled up ? Mark it as complete and find next receive buffer
				byte RXStatus = com5025.Read((byte)RegistersByte.ReceiverStatusRegister);
				dmaRegs.MarkBufferReceived(RXStatus);

				// Tell ND that the Block has ended
				SetRXDMAFlag(RTSBits.BlockEnd | RTSBits.FrameEnd | RTSBits.DataAvailable | RTSBits.ReceiverActive | RTSBits.SyncFlagReceived); // | RTSBits.StatusAvailable);
																																			   //SetRXDMAFlag(ReceiverStatusBits.FrameEnd);

				// Move to next buffer
				dmaRegs.LoadNextRXBuffer();

				// Loop around to start of list.
				if (dmaRegs.RX_DCB.Key == KeyFlags.NewListPointer)
				{
					dmaRegs.RX_ListPointer_Offset = 0;
					dmaRegs.LoadRXBuffer();

					if (dmaRegs.RX_DCB.Key == KeyFlags.FullReceiverBlock)
					{
						// ND-12.018.01 - Page II-3-13:
						//
						// This looping in receiver list requires that driver software is fast enough to reset the list entries.
						// If the DMA processor reaches the NEW LIST POINTER and than Full Receiver Block, a LIST EMPTY interrupt is generated.
						// (See Receiver TransferStatus Register).

						SetRXDMAFlag(RTSBits.ListEmpty);

						// TODO: Analyse if we should Disable receiver?
						return; // we are done here, received data will be lost
					}

				}
			}

			// Check if we have a valid buffer description
			if (dmaRegs.RX_DCB.Key == KeyFlags.EmptyReceiverBlock)
			{
				// DMA the data to ND-100 memory (via the current buffer description)
				dmaRegs.WriteNextByteDMA(data, true);
				Debug.WriteLine($"DMA RX position = {dmaRegs.RX_DCB.dma_bytes_written}, Data= 0x{data:X2}");
			}
			else
			{


				Logger.Log($"Unexpected - Receiver buffer Key is not EmptyReceiverBlock (Key={dmaRegs.RX_DCB.Key})", Logger.LogLevel.Warning);
				// Disable receiver.
				//EnableHDLCReceiver(false);

				//regs.ReceiverTransferControl &= ~(ReceiverTransferControlFlags.EnableReceiverDMA);

				// If link is stopped, this may happen
				SetRXDMAFlag(RTSBits.ReceiverOverrun | RTSBits.ListEmpty);
				return;
			}
		}


		private void Log(string logMsg)
		{
			Logger.Log($"DMAReceive: {logMsg}", Logger.LogLevel.Device);
		}


		private void SetInterruptBit(byte bit)
		{
			if (OnSetInterruptBit != null)
			{
				OnSetInterruptBit(bit);
			}
		}

		/// <summary>
		/// Tick the state machine in the receiver ?
		/// </summary>
		internal void Tick()
		{

		}
	}
}
