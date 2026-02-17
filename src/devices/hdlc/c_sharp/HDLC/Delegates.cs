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
	/// Delegate for HDLC frame send event
	/// </summary>
	/// <param name="frame">The HDLC frame to be sent</param>
	public delegate void SendHDLCFrameEventHandler(HDLCFrame frame);


	/// <summary>
	/// Delegat for "Set interrupt bit "
	/// </summary>
	/// <param name="bit"></param>
	public delegate void SetInterruptBitEventHandler (byte bit);


}
