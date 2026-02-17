
#define DEBUG_DETAIL
#define DEBUG_DETAIL_PLUS_DESCRIPTION


#define DMA_DEBUG
#define RX_BLAST_LOGGING



using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization.Metadata;
using System.Threading.Tasks;
using Emulated.HW.Common.Network;
using Emulated.HW.SMC.COM.COM5025;
using Emulated.Lib;

namespace Emulated.HW.ND.CPU.NDBUS.HDLC
{
	internal class DMATransmitter
	{

		/// <summary>
		/// Event raised when an HDLC frame is ready to be sent
		/// </summary>
		public event SendHDLCFrameEventHandler? OnSendHDLCFrame;

		public event SetInterruptBitEventHandler? OnSetInterruptBit;

		Registers regs;
		DMAControlBlocks dmaRegs;
		COM5025 com5025;

		string dbgRawBlockValues = "";

		public DMATransmitter(Registers r, DMAControlBlocks dr, COM5025 c)
		{
			regs = r;
			dmaRegs = dr;
			com5025 = c;
		}

		internal void Tick()
		{
			if (dmaRegs.BurstMode)
			{
				// In burst mode, we do not use the COM5025 chip, but blast out all data as fast as possible
				tick_DMASendBlastEngine();
			}
			else
			{
				// Tick DMA state machine
				tick_DMASendEngine();
			}
		}


		/// <summary>
		/// Set DMA flag, and raise an interrupt on level 12 (TX) 
		/// </summary>
		/// <param name="flag"></param>
		/// <param name="isTx"></param>
		private void SetTXDMAFlag(TTSBits flag)
		{
			string irq_reason = "";

			regs.TransmitterTransferStatus |= flag;

			if (flag.HasFlag(TTSBits.TransmissionFinished))
			{
				regs.TransmitterTransferStatus |= TTSBits.DMAModuleRequest;
				irq_reason += "TransmissionFinished ";
			}


			if (regs.TransmitterTransferControl.HasFlag(TTCBits.BlockEndIE) && regs.TransmitterTransferStatus.HasFlag(TTSBits.BlockEnd))
			{
				regs.TransmitterTransferStatus |= TTSBits.DMAModuleRequest;
				irq_reason += "BlockEnd ";
			}

			if (regs.TransmitterTransferControl.HasFlag(TTCBits.FrameEndIE) && regs.TransmitterTransferStatus.HasFlag(TTSBits.FrameEnd))
			{
				regs.TransmitterTransferStatus |= TTSBits.DMAModuleRequest;
				irq_reason += "FrameEnd ";
			}

			if (regs.TransmitterTransferControl.HasFlag(TTCBits.ListEndIE) && regs.TransmitterTransferStatus.HasFlag(TTSBits.ListEnd))
			{
				regs.TransmitterTransferStatus |= TTSBits.DMAModuleRequest;
				irq_reason += "ListEnd ";
			}

#if DMA_DEBUG
			Log("--------------------------------------------------------------------------");
			Log($"DMA SetTXDMAFlag            : {flag} ({(ushort)flag}) IRQ[{irq_reason}]");
			Log($"Transmitter TransferStatus  : {regs.TransmitterTransferStatus} ({(ushort)regs.TransmitterTransferStatus})");
			Log($"Transmitter TransferControl : {regs.TransmitterTransferControl}");
			Log("--------------------------------------------------------------------------");
#endif

			if (regs.TransmitterTransferControl.HasFlag(TTCBits.DMAModuleIE) && regs.TransmitterTransferStatus.HasFlag(TTSBits.DMAModuleRequest))
			{
				SetInterruptBit(12);
			}
		}

		private void SetInterruptBit(byte interruptBit)
		{
			if (OnSetInterruptBit != null)
			{
				OnSetInterruptBit(interruptBit);
			}
		}



		/// <summary>
		/// Read all buffers (blocks) that are available to be sent
		/// Generate HDLC frames and send them to the HDLC controller
		///
		/// Returns true if ALL buffers have been sent
		/// </summary>
		internal bool SendAllBuffers()
		{

			TTSBits tmpTSB = 0;

			if (dmaRegs.TX_ListPointer == 0) return true;

			HDLCFrame frame = new HDLCFrame();

			dmaRegs.LoadTXBuffer();

			while (true)
			{
				if (dmaRegs.TX_DCB is null) return true;

				while (dmaRegs.TX_DCB?.Key == KeyFlags.AlreadyTransmittedBlock)
				{
					//Log($"Skipping already transmitted block at offset {cur_block}");
					dmaRegs.LoadNextTXBuffer();

				}

				if (dmaRegs.TX_DCB.Key == KeyFlags.BlockToBeTransmitted)
				{
					Log($"Found block to be sent at offset {dmaRegs.TX_ListPointer_Offset}");

					if (dmaRegs.TX_DCB.HasRSOMFlag)
					{
						// Start of new frame - clear outbound buffer
						dmaRegs.OutboundBuffer.Clear();
					}

					ushort bytesToSend = (ushort)(dmaRegs.TX_DCB.ByteCount + dmaRegs.TX_DCB.Displacement);
					while (dmaRegs.TX_DCB.dma_bytes_read < bytesToSend)
					{
						// Add bytes to persistent outbound buffer
						dmaRegs.OutboundBuffer.Add(dmaRegs.ReadNextByteDMA(false));
					}

					tmpTSB |= TTSBits.BlockEnd;

					if (dmaRegs.TX_DCB.HasREOMFlag)
					{
						// Frame is complete, send it to HDLC controller
						frame = new HDLCFrame();
						frame.CreateFrame(dmaRegs.OutboundBuffer.ToArray(), (ushort)dmaRegs.OutboundBuffer.Count);

						if (OnSendHDLCFrame != null)
						{
							OnSendHDLCFrame(frame);
						}

						// Clear outbound buffer
						dmaRegs.OutboundBuffer.Clear();

						tmpTSB |= TTSBits.FrameEnd;
					}

					// Mark buffer sent
					dmaRegs.MarkBufferSent();

					// Check if there are more DCBs to process
					if (!dmaRegs.LoadNextTXBuffer())
					{
						// No more DCBs - transmission finished
						tmpTSB |= TTSBits.TransmissionFinished | TTSBits.ListEnd;
						SetTXDMAFlag(tmpTSB);
						Set_DmaEngineSenderState(DmaEngineSenderState.STOPPED);
						dmaRegs.RX_ListPointer_Offset = 0;
						return true; // All buffers have been sent
					}
					else
					{
						// More DCBs to process - set flags for current DCB and continue
						SetTXDMAFlag(tmpTSB);
						tmpTSB = 0; // Reset for next DCB
						// Continue in the loop to process next DCB
					}
				}
				else
				{
					if (dmaRegs.TX_DCB.KeyValue == 0x00)
					{
						Log($"ListEnd: End of list found at offset {dmaRegs.TX_ListPointer_Offset} KeyValue[0x{dmaRegs.TX_DCB.KeyValue:X4}]");
					}
					else
					{
						Log($"ListEnd: No more blocks to be sent at offset 0x{dmaRegs.TX_ListPointer:X6} : offset[{dmaRegs.TX_ListPointer_Offset}] KeyValue[0x{dmaRegs.TX_DCB.KeyValue:X4}]");
					}
					// Tell SINTRAN that we have successfully transmitted all buffers and we are out of work
					SetTXDMAFlag(TTSBits.TransmissionFinished | TTSBits.ListEnd);
					Set_DmaEngineSenderState(DmaEngineSenderState.STOPPED);
					dmaRegs.RX_ListPointer_Offset = 0;
					return true; // All buffers have been sent
				}
			}
		}




		/// <summary>
		/// Send the next character to the HDLC controller.
		/// If the block is ended:
		/// * Mark block sendt, and DMA back to ND the updated KEY
		/// * tell ND via Interrupt that the Block has ended
		/// </summary>
		internal void DMA_SendChar(bool isFirstChar)
		{

			if (dmaRegs.BurstMode) return; // In burst mode, we do not send char by char via COM5025 chip

			if (!regs.TransmitterTransferControl.HasFlag(TTCBits.EnableTransmitterDMA)) return; // transmitter DMA not enabled
			if (!regs.TransmitterTransferControl.HasFlag(TTCBits.TransmitterEnabled)) return; // transmitter not enabled, so this probably SYNC FRAMES


			if (dmaRegs.dmaSenderState != DmaEngineSenderState.SENDING_BLOCK) return; // Not in sending state

			// Check for unexpected KEY
			if (dmaRegs.dmaSendBlockState != DmaBlockSendState.SEND_FRAME_END)
			{
				if (dmaRegs.TX_DCB.Key != KeyFlags.BlockToBeTransmitted) //  || (regs.DMA_TX_BufferDescription.Key == KeyFlags.AlreadyTransmittedBlock))
				{
					Log($"DMA_SendChar: Unexpected DMA Key in TX: {dmaRegs.TX_DCB.Key} ({(byte)dmaRegs.TX_DCB.Key:X2})");
					Set_DmaEngineSenderState(DmaEngineSenderState.BLOCK_READY_TO_SEND);
					return;
				}
			}

			if (dmaRegs.dmaSendBlockState == DmaBlockSendState.IDLE)
			{
				// Is this the first char in the block to be transmitted?			
				if (dmaRegs.TX_DCB.dma_bytes_read == 0) //isFirstChar
				{
					Set_DmaBlockSendState(DmaBlockSendState.START_BLOCK); // wait for read buffer empty interrupt for the first sync char sent
					dbgRawBlockValues = "[ ";
					return;
				}
			}


			switch (dmaRegs.dmaSendBlockState)
			{

				case DmaBlockSendState.IDLE:
					break;
				case DmaBlockSendState.START_BLOCK:

					// The low bits of the buffer KeyValue tells is if this is the START of an HDLC frame or if its a continuation of a frame (is just a block of data to be transmitted)
					if (dmaRegs.TX_DCB.HasRSOMFlag)
					{
						// This block was a start of a new frame, so we now need to tell the HDLC chip to to switch to DATA mode
						com5025.SetTSOM(false); // Start of message	disabled								
					}

					com5025.SetTEOM(false); // Make sure TEOM is off
					Set_DmaBlockSendState(DmaBlockSendState.SEND_DATA);

					break;

				case DmaBlockSendState.SEND_DATA:
					// Since we use displacement to move start-pointer for memory read, we need to count it here so all bytes are sent!
					ushort bytesToSend = (ushort)(dmaRegs.TX_DCB.ByteCount + dmaRegs.TX_DCB.Displacement);


					// Check if we have sent all bytes in this block
					if (dmaRegs.TX_DCB.dma_bytes_read >= bytesToSend)
					{

						// Mark buffer sent
						dmaRegs.MarkBufferSent();

						// Log the sent block
						Log($"Sent block [{dmaRegs.TX_DCB.BufferAddress:X6}:{dmaRegs.TX_ListPointer_Offset}]: {dbgRawBlockValues} [RSOM:{dmaRegs.TX_DCB.HasRSOMFlag}] [REOM:{dmaRegs.TX_DCB.HasREOMFlag}]");

						dbgRawBlockValues = "";

						if (dmaRegs.TX_DCB.HasREOMFlag)
						{
							// all bytes send from this buffer, and this buffer is also end of message
							com5025.SetTEOM(); // Set transmit "End of message"

							//  Last char of the frame was sent
							Set_DmaBlockSendState(DmaBlockSendState.SEND_FRAME_END);
							break;
						}
						else
						{
							// TODO: need to fetch next buffer and start sending bytes from that buffer ?
							// Scenario is one HDLC frame that is crossing multiple blocks
							//Debug.Assert(false);

							SetTXDMAFlag(TTSBits.BlockEnd);

							bytesToSend = 0;

							if (dmaRegs.LoadNextTXBuffer())
							{
								bytesToSend = (ushort)(dmaRegs.TX_DCB.ByteCount + dmaRegs.TX_DCB.Displacement);

								// start tracking new block
								dbgRawBlockValues = "[ ";

							}
						}
					}


					// Send byte to COM5025 chip only if end of buffer is not reached
					if (dmaRegs.TX_DCB.dma_bytes_read < bytesToSend)
					{
						// Send next byte from block to the COM5020 chip
						byte value = dmaRegs.ReadNextByteDMA(false);
						//Debug.WriteLine($">> DMA TX [{regs.DMA_TX_BufferDescription.dma_bytes_read}/{regs.DMA_TX_BufferDescription.ByteCount}] 0x{value:X2}");
						com5025.Write((byte)RegistersByte.TransmitterDataRegister, (byte)value);
						dbgRawBlockValues += $"0x{value:X2} ";
					}
					else
					{
						Log($"END OF BUFFER REACHED IN DMA_Send. Bytes read from memory={dmaRegs.TX_DCB.dma_bytes_read} BytesToSend={bytesToSend}");
						Set_DmaBlockSendState(DmaBlockSendState.SEND_FRAME_END);
					}
					break;

				case DmaBlockSendState.SEND_FRAME_END: // Called after the last SYNC (char has been sent in the HDLC frame - require that TEOM as been set)

					Log($"End of frame SYNC char has been sent. Disabling transmitter");
					com5025.SetInputPinValue(SignalPinIn.TXENA, false); // Transmitter disabled.


					Set_DmaBlockSendState(DmaBlockSendState.SEND_FRAME_END);
					Set_DmaEngineSenderState(DmaEngineSenderState.FRAME_SENT);


					// Make sure TSOM and TEOM are disabled, but only if frame is complete (ie REOM flag is set)
					if (dmaRegs.TX_DCB.HasREOMFlag)
					{
						com5025.SetTSOM(false); // Start of message
						com5025.SetTEOM(false); // End of Message
					}


					//com5025.SetInputPinValue(SignalPinIn.TXENA, false); // Transmitter disabled.


					/*
					Log($"Loading new TX buffer: 0x{regs.DMA_TX_BufferDescription.ListPointer:X6}");
					regs.DMA_TX_BufferDescription = ReadNextBuffer(regs.DMA_TX_BufferDescription);

					// If next buffer is not Blocks to be transmitted, stop DMA
					if (regs.DMA_TX_BufferDescription.Key != KeyFlags.BlockToBeTransmitted)
					{
						//SetTXDMAFlag(TransmitterStatusBits.TransmissionFinished);
						SetTXDMAFlag(TransmitterStatusBits.ListEnd);
						dmaSenderState = DMA_SenderState.IDLE;
					}
					else
					{
						regs.DMA_TX_BufferDescription.Displacement = (ushort)parameterBuffer.Displacement2;
					}
					*/






					//Debug.WriteLine("==================END SEND=================");
					break;
				default:
					// Wait for DMA engine to initialize a new send block
					break;
			}

		}



		void dmaEngineProcessState_BLOCK_READY_TO_SEND()
		{

			// Do we need to find a new block to send ?
			if (dmaRegs.TX_DCB.Key != KeyFlags.BlockToBeTransmitted)
			{
				// Re-read current buffer description, maybe it has changed
				dmaRegs.LoadTXBuffer();

				// Still not a block to be transmitted, scan forward in the list
				while (dmaRegs.TX_DCB.Key != KeyFlags.BlockToBeTransmitted)
				{
					if (dmaRegs.LoadNextTXBuffer()) break; // Found a block to be transmitted?



					// Found end of list ?
					if (dmaRegs.TX_DCB.KeyValue == 0x00)
					{
						// End of list (wrap back to start, maybe we will get a new valid block later)
						dmaRegs.TX_ListPointer_Offset = 0;
						dmaRegs.LoadTXBuffer();
						return;
					}
				}
			}


			// Do we have blocks to send ?
			if (dmaRegs.TX_DCB.Key == KeyFlags.BlockToBeTransmitted)
			{
				/*
				if (regs.DMA_TX_BufferDescription.HasRSOMFlag) // might be false, then we have a frame across multiple blocks
				{
					SetTSOM(true); // Start of message
					SetTEOM(false); // Make sure TEOM is off
				}
				*/

				Set_DmaBlockSendState(DmaBlockSendState.IDLE);
				Set_DmaEngineSenderState(DmaEngineSenderState.SENDING_BLOCK);

				// Enable COM5025 module to start sending SYNC flag, which will trigger a call to  DMA_SendChar() after SYNC has been sent
				if (dmaRegs.TX_DCB.HasRSOMFlag) // might be false, then we have a frame across multiple blocks
				{
					com5025.SetTSOM(true); // Start of message					
				}

				com5025.SetTEOM(false); // Make sure TEOM is off
				return;
			}


#if _OLD_SHIT_
			// Check for end of list
			if (regs.DMA_TX_BufferDescription.KeyValue == 0x00)
			{				
				/*
				//SetTXDMAFlag(TransmitterStatusBits.TransmissionFinished);
				SetTXDMAFlag(TransmitterStatusBits.ListEnd | TransmitterStatusBits.TransmissionFinished);

				// DMA Sender is now stopped, no more blocks to send. Waiting
				Set_DmaEngineSenderState(DmaEngineSenderState.STOPPED);
				*/
				return;
			}



			if (regs.DMA_TX_BufferDescription.Key != KeyFlags.BlockToBeTransmitted)
			{
				uint nextMem = ScanNextTXBuffer(regs.DMA_TX_BufferDescription.ListPointer);

				if (nextMem > 0)
				{
					regs.DMA_TX_BufferDescription = LoadBufferDescription(nextMem);

					// Uses Displacement 1 for the first buffer and should therefor be used the first
					//TODO: Evaluate
				}
			}

			if (regs.DMA_TX_BufferDescription.Key != KeyFlags.BlockToBeTransmitted)
			{
				SetTXDMAFlag(TransmitterStatusBits.ListEnd | TransmitterStatusBits.TransmissionFinished);
				//SetTXDMAFlag(TransmitterStatusBits.Illegal|TransmitterStatusBits.TransmissionFinished);  // was listend , and blockend

				Set_DmaEngineSenderState(DmaEngineSenderState.STOPPED);
			}
#endif
		}


		/// <summary>
		/// Buffer has been sent (and it was also end of frame)
		/// </summary>
		void dmaEngineProcessState_FRAME_SENT()
		{

			Log("Starting: dmaEngineProcessState_FRAME_SENT");

			// Tell SINTRAN that we have successfully transmitted a frame			
			TTSBits tmpTSB = 0;
			if (dmaRegs.TX_DCB.HasREOMFlag)
			{
				tmpTSB |= TTSBits.FrameEnd;
			}


			// Always set block end (?)
			//if (regs.DMA_TX_BufferDescription.HasRSOMFlag)
			{
				tmpTSB |= TTSBits.BlockEnd;
			}


			// Are there more buffers to send ?
			if (dmaRegs.TX_DCB.KeyValue == 0x00)
			{
				// No more buffers!
				tmpTSB |= TTSBits.TransmissionFinished | TTSBits.ListEnd;
				//tmpTSB |= TransmitterStatusBits.ListEnd;

			}
			else
			{

				//TODO:Maybe we need a "peek" function to see if next buffer is NOT valid before we flag ListEnd

				// Find next buffer to send
				if (!dmaRegs.LoadNextTXBuffer())
				{
					tmpTSB |= TTSBits.TransmissionFinished | TTSBits.ListEnd;
					//tmpTSB |= TransmitterStatusBits.ListEnd;
				}
			}

			// Set the flags we found
			if (tmpTSB != 0)
			{
				SetTXDMAFlag(tmpTSB); // TransmitterStatusBits.FrameEnd | TransmitterStatusBits.BlockEnd); // Was FrameEnd |
			}


			// Update DMA Engine state
			if (dmaRegs.TX_DCB.KeyValue == 0x00)
			{
				Set_DmaEngineSenderState(DmaEngineSenderState.STOPPED);
				dmaRegs.TX_ListPointer_Offset = 0;
			}
			else
			{
				Set_DmaEngineSenderState(DmaEngineSenderState.BLOCK_READY_TO_SEND);
			}

			Log("Ending: dmaEngineProcessState_FRAME_SENT");
		}


		/// <summary>
		/// State engine for blasting out all data without using the COM5025 chip
		/// </summary>
		void tick_DMASendBlastEngine()
		{

			switch (dmaRegs.dmaSenderState)
			{
				case DmaEngineSenderState.STOPPED:
					break;

				case DmaEngineSenderState.BLOCK_READY_TO_SEND:
					// Wait for DMA enabled AND Transmitter enabled
					if ((regs.TransmitterTransferControl.HasFlag(TTCBits.TransmitterEnabled)) && (regs.TransmitterTransferControl.HasFlag(TTCBits.EnableTransmitterDMA)))
					{
						// Have some delays to help machine not blast everything at to high speed
						if (dmaRegs.dmaWaitTicks > 0)
						{
							dmaRegs.dmaWaitTicks--;
							if (dmaRegs.dmaWaitTicks == 0)
							{
								if (SendAllBuffers())
								{
									dmaRegs.dmaWaitTicks = -1; // disable
								}
								else
								{
									dmaRegs.dmaWaitTicks = 50; // Wait 50 cpu instructions before next block is sent (make sure IRQ has had time to run)
								}
							}
						}
					}
					break;

				case DmaEngineSenderState.SENDING_BLOCK:
					break;

				case DmaEngineSenderState.FRAME_SENT:
					break;
				default:
					break;
			}
		}


		int stopDelayTicks = 0;
		/// <summary>
		/// DMA State machine gets ticked from the main CPU loop
		/// Works together with the DMA_SendChar function that is called from the HDLC chip when it needs more data
		/// </summary>
		void tick_DMASendEngine()
		{





			switch (dmaRegs.dmaSenderState)
			{
				case DmaEngineSenderState.STOPPED:
#if _FORCE_RESTART_FROM_STOPPED_STATE
					// Every 1000 ticks, check if we have received a new buffer to send

					if (regs.DMA_TX_ListPointer > 0)
					{
						stopDelayTicks++;
						if (stopDelayTicks > 1000)
						{
							int memkey = DMARead(regs.DMA_TX_ListPointer);
							KeyFlags key = (KeyFlags)(memkey) & KeyFlags.MASK_KEY;

							if (key == KeyFlags.BlockToBeTransmitted)
							{
								regs.DMA_TX_ListPointer_Offset = 0;
								regs.DMA_TX_BufferDescription = LoadBufferDescription(regs.DMA_TX_ListPointer, regs.DMA_TX_ListPointer_Offset);

								if (regs.DMA_TX_BufferDescription.Key == KeyFlags.BlockToBeTransmitted)
								{
									Log("FORCING RE-START OF DMA SENDER");
									Set_DmaBlockSendState(DmaBlockSendState.IDLE);
									Set_DmaEngineSenderState(DmaEngineSenderState.BLOCK_READY_TO_SEND);
								}
							}
						}
					}
#endif
					break;

				case DmaEngineSenderState.BLOCK_READY_TO_SEND:
					dmaEngineProcessState_BLOCK_READY_TO_SEND();
					break;

				case DmaEngineSenderState.SENDING_BLOCK:
					break;

				case DmaEngineSenderState.FRAME_SENT:
					dmaEngineProcessState_FRAME_SENT();
					break;
				default:
					break;
			}
		}


		/// <summary>
		/// Set the state of the DMA block sender (State withing a HDLC block of bytes being sent)
		/// </summary>
		/// <param name="state"></param>
		void Set_DmaBlockSendState(DmaBlockSendState state)
		{
			Log($"Set_DmaBlockSendState: {state}");
			dmaRegs.dmaSendBlockState = state;
		}




		/// <summary>
		/// Set the state of the DMA sender engine
		/// </summary>
		/// <param name="state"></param>
		void Set_DmaEngineSenderState(DmaEngineSenderState state)
		{
			Log($"Set_DmaEngineSenderState: {state}");
			dmaRegs.dmaSenderState = state;

			if (state == DmaEngineSenderState.BLOCK_READY_TO_SEND)
			{
				dmaRegs.dmaWaitTicks = 500; // Wait 500 cpu instructions before sender starts
			}

		}


		private void Log(string logMsg)
		{
			Logger.Log($"DMATransmit: {logMsg}", Logger.LogLevel.Device);
		}
		internal void SetSenderState(DmaEngineSenderState engineState)
		{
			Set_DmaEngineSenderState(engineState);
		}

	}
}
