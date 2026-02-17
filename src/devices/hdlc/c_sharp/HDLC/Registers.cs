// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Emulated.HW.Common.Network;

namespace Emulated.HW.ND.CPU.NDBUS.HDLC
{
	/// <summary>
	/// Registers used by the HDLC controller card
	/// </summary>
	internal class Registers
	{
		public Registers()
		{
			SetTH1(TH1Wheel.TW7_38400); // Default value for TX speed
		}

		// Has this controller DMA enabled ?
		internal bool DMAEnabled = false;

		// Set TX clock speed
		internal TH1Wheel TH1 = TH1Wheel.TW7_38400;

		/// <summary>
		/// Number of CPU ticks pr HDLC tx clock
		/// </summary>
		internal int cpu_ticks_pr_tx = 1041; //38400 bps?

		/// <summary>
		/// CPu tick counter for Tick() logic
		/// </summary>
		internal int cpuTicks = 0;



		/// <summary>
		/// Set thumbwheel TH1 on the HDLC controller card.
		/// It controls TX clock/transmit speed
		/// </summary>
		/// Ref: https://www.ndwiki.org/wiki/3023
		/// <param name="th"></param>
		public void SetTH1(TH1Wheel th)
		{
			int bps = 38400; // default value
			TH1 = th;
			switch (TH1)
			{
				case TH1Wheel.TW0_307200:
					bps = 307200;
					break;
				case TH1Wheel.TW1_invalid:
					break;
				case TH1Wheel.TW2_invalid:
					break;
				case TH1Wheel.TW3_1200:
					bps = 1200;
					break;
				case TH1Wheel.TW4_invalid:
					break;
				case TH1Wheel.TW5_invalid:
					break;
				case TH1Wheel.TW6_9600:
					bps = 9600;
					break;
				case TH1Wheel.TW7_38400:
					bps = 38400;
					break;
				case TH1Wheel.TW8_153600:
					bps = 153600;
					break;
				case TH1Wheel.TW9_76800:
					bps = 76800;
					break;
				case TH1Wheel.TW10_invalid:
					break;
				case TH1Wheel.TW11_19200:
					bps = 19200;
					break;
				case TH1Wheel.TW12_invalid:
					break;
				case TH1Wheel.TW13_4800:
					bps = 4800;
					break;
				case TH1Wheel.TW14_2400:
					bps = 2400;
					break;
				default:
					break;
			}

			// assume CPU ticks is 40MHZ
			cpu_ticks_pr_tx = 40_000_000 / bps;
		}



		#region CONTROL OF THE DMA Address  IOX <DEVICE NO.> + 15
		internal ushort DMAAddressLO; // 16 least significant bits of DMA
		#endregion

		#region CONTROL OF THE DMA PROCESSOR  IOX <DEVICE NO.> + 17
		internal DMACommands DMACommand;
		internal int DMABankBits; //bits for BANK select

		/// <summary>
		/// Calculated 18 bits memory address
		/// </summary>
		public uint DMA_Address
		{
			get
			{
				return (uint)(DMAAddressLO | (DMABankBits << 16));
			}
			set
			{
				DMAAddressLO = (ushort)(value & 0xFFFF);
				DMABankBits = (ushort)(value >> 16);
			}
		}


		/// <summary>
		/// IOX GP+10 (READ) 
		/// Receiver Transfer Status (RRTS)
		/// </summary>
		public RTSBits ReceiverTransferStatus { get; internal set; }

		/// <summary>
		/// Register to hold changes in the signal from the modem for the RECEIVER part of HDCL controller
		/// </summary>
		public RTSBits RX_ModemFlags { get; internal set; }

		/// <summary>
		/// Mask used to detect interrupt on signal changes
		/// </summary>
		public readonly RTSBits RX_ModemFlagsMask = (RTSBits.SignalDetector | RTSBits.RingIndicator | RTSBits.DataSetReady);

		/// <summary>
		/// IOX GP+11 (WRITE) 
		/// Receiver Transfer Control (WRTC)
		/// </summary>
		public RTCBits ReceiverTransferControl { get; internal set; }


		/// <summary>
		/// IOX GP+12 
		/// Transmitter Transfer Status (RTTS)
		/// </summary>
		public TTSBits TransmitterTransferStatus { get; internal set; }

		/// <summary>
		/// Register to hold changes in the signal from the modem for the TRANSMITTER part of HDCL controller
		/// </summary>
		public TTSBits TX_ModemFlags { get; internal set; }

		/// <summary>
		/// What are the bits that we care about in the TX_ModemFlags
		/// </summary>
		public readonly TTSBits TX_ModemFlagsMask = (TTSBits.ReadyForSending);

		/// <summary>
		/// IOX GP+13 (WRITE)
		/// Transmitter Transfer Control (WTTC)
		/// </summary>
		public TTCBits TransmitterTransferControl { get; internal set; }

		/// <summary>
		/// Are we in maintenance mode (loopback)
		/// </summary>
		public bool Maintenance { get; internal set; }



		/// <summary>
		/// Clear register
		/// Set deviceClear if this is called from DeviceClear()
		/// </summary>
		/// <param name="deviceClear"></param>
		internal void Clear(bool deviceClear = false)
		{
			DMACommand = DMACommands.DEVICE_CLEAR;
			DMABankBits = 0;
			ReceiverTransferStatus = 0;

			if (deviceClear)
			{
				ReceiverTransferControl &= RTCBits.MASK_CLEAR_DEVICE_CLEAR;
				TransmitterTransferStatus &= TTSBits.MASK_CLEAR_DEVICE_CLEAR;
				TransmitterTransferControl &= TTCBits.MASK_CLEAR_DEVICE_CLEAR; ;
			}
			else
			{
				// MASTER CLEAR 
				ReceiverTransferControl = 0;
				TransmitterTransferStatus = 0;
				TransmitterTransferControl = 0;
				//RX_ModemFlags = 0;
				//TX_ModemFlags = 0;

				Maintenance = false;
			}	
		}

		#endregion

	}
}
