// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Emulated.HW.ND.CPU.NDBUS.HDLC
{

	/// <summary>
	/// DMA Module parameterbuffer
	/// </summary>
	internal class ParameterBuffer
	{
		/// <summary>
		/// PCR (PCRH/High byte and PCRL/low byte)
		///
		/// Parameter Control Reg (8 least significant bits)
		/// 
		/// </summary>
		internal int ParameterControlRegister;

		/// <summary>
		/// Sync/Address Register (8 least significant bits)
		/// </summary>
		internal int Sync_AddressRegister;

		/// <summary>
		/// Character Length (8 least significant bits)
		/// </summary>
		internal int CharacterLength;

		/// <summary>
		/// No. of bytes, first block in frame
		/// Displacement 1 is the number of free bytes reserved at the beginning of each buffer containing the start of a message(Frame).
		/// </summary>
		internal int Displacement1;

		/// <summary>
		/// No. of bytes, other blocks in frame
		/// Displacement 2 is the number of free bytes reserved at the beginning of each buffer which _do not_ contain the start of a message(Frame)
		/// </summary>
		internal int Displacement2;

		/// <summary>
		/// No. of bytes, including displacement
		/// Max.Receiver Block Length is the total number of bytes in a receiver buffer, including displacement.
		/// Long frames may be divided into blocks and stored in two or more buffers. 
		/// </summary>
		internal int MaxReceiverBlockLength;

		/// <summary>
		/// Receiver Status Register
		/// 8 least significant bits, not accumulate
		/// </summary>
		internal int ReceiverStatusReg;

		/// <summary>
		/// Transmitter Status Reg.
		/// 8 least significant bits, not accumulated
		/// </summary>
		internal int TransmitterStatusReg;


		internal int DmaBankBits;

		internal void Clear()
		{
			ParameterControlRegister = 0;
			Sync_AddressRegister = 0;
			CharacterLength = 0;
			Displacement1 = 0;
			Displacement2 = 0;
			MaxReceiverBlockLength = 0;
			ReceiverStatusReg = 0;
			TransmitterStatusReg = 0;

		}
	
	}
}
