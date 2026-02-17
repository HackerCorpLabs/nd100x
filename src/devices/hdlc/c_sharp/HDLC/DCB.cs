using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Emulated.HW.ND.CPU.NDBUS.HDLC
{

	/// <summary>
	/// In memory transmit and receive buffers
	///
	/// DMA Control Block (DCB)
	/// </summary>
	internal class DCB
	{
		/// <summary>
		/// Where in memory was this buffer description loaded from
		/// </summary>
		public uint BufferAddress { get; set; } = 0;

		/// <summary>
		/// For this Buffer description, what is the offset (in blocks) from the start list pointer
		/// </summary>
		public ushort OffsetFromLP { get; set; } = 0;

		// KEY
		public ushort KeyValue { get; set; } = 0;

		public KeyFlags Key => (KeyFlags)(KeyValue) & KeyFlags.MASK_KEY;

		/// <summary>
		/// Has Receiver Start of Message flag
		/// </summary>
		public bool HasRSOMFlag => ((KeyFlags)(KeyValue)).HasFlag(KeyFlags.RCOST_RSOM);

		/// <summary>
		/// Has Receiver End of Message flag
		/// </summary>
		public bool HasREOMFlag => ((KeyFlags)(KeyValue)).HasFlag(KeyFlags.RCOST_REOM);

		public ushort DataFlowCost => (ushort)((KeyFlags)KeyValue & KeyFlags.MASK_DATAFLOW_COST);

		/// <summary>
		/// Byte count.
		/// Number of information bytes
		/// </summary>
		public ushort ByteCount { get; set; } = 0;

		/// <summary>
		/// Data Address
		/// </summary>
		public ushort MostAddress { get; set; } = 0;
		public ushort LeastAddress { get; set; } = 0;

		/// <summary>
		/// Originally 18 bit memory address.. Lets be a bit more open to potentially bigger memories like 24bit
		/// </summary>
		public uint DataMemoryAddress => (uint)(MostAddress & 0x00FF) << 16 | LeastAddress;

		// Displacement 
		public ushort Displacement { get; set; } = 0;
		public uint ListPointer { get; internal set; } = 0;



		// Helper fields for DMA transfer
		internal uint dmaAddress; // Must be set to the Memory address we should read from
		internal int dma_bytes_read;
		internal int dma_bytes_written;

		/// <summary>
		/// Last DMA read from ND. Will be -1 if its not read
		/// </summary>
		internal int dma_read_data;
	}

}
