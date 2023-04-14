#include "gdt.h"
#include <iostream.h>

using namespace std;

namespace GDT
{
	class Descriptor
	{
		word size;
		word offset[4];

	public:
		inline Descriptor() : size(0) { setOffset(0); }
		inline Descriptor(qword offset, word size) : size(size) { setOffset(offset); }

		inline void setSize(word size_)
		{
			size = size_;
		}
		inline void setOffset(qword offset_)
		{
			*(qword *)offset = offset_;
		}

		inline word getSize()
		{
			return size;
		}
		inline qword getOffset()
		{
			return *(qword *)offset;
		}
	};

	SegmentDescriptor *globalDescriptorTable = (SegmentDescriptor *)0x2000;

	extern "C" void loadGDT(GDT::Descriptor &descriptor);
	extern "C" void getCurrentGDT(GDT::Descriptor &descriptor);
	extern "C" void updateSegmentRegisters();
	extern "C" void loadTSS(word gdtEntry);

	TSS *tss = (TSS *)0x38000;

	void Initialize()
	{
		tss->clear();
		tss->rsp[0] = tss->ist[0] = 0x40000;

		TSSDescriptor tssDesc(*tss);

		byte count = 0;
		globalDescriptorTable[count++] = SegmentDescriptor();			  // null descriptor
		globalDescriptorTable[count++] = CodeSegmentDescriptor(0, false); // kernel code
		globalDescriptorTable[count++] = DataSegmentDescriptor(0);		  // kernel data
		globalDescriptorTable[count++] = CodeSegmentDescriptor(3, false); // user code (ring 3)
		globalDescriptorTable[count++] = CodeSegmentDescriptor(3, true);  // compatibility user code (ring 3)
		globalDescriptorTable[count++] = DataSegmentDescriptor(3);		  // user data
		word tssEntry = count;
		globalDescriptorTable[count++] = tssDesc.low;  // 64 bit tss - low entry
		globalDescriptorTable[count++] = tssDesc.high; // 64 bit TSS - high entry

		Descriptor descriptor((qword)globalDescriptorTable, sizeof(SegmentDescriptor) * count - 1);
		loadGDT(descriptor);
		updateSegmentRegisters();
		loadTSS(tssEntry * sizeof(SegmentDescriptor));
	}
	void testGDT()
	{
		Descriptor descriptor;
		getCurrentGDT(descriptor);
		cout << "Size: " << descriptor.getSize()
			 << "\nAddress: " << (void *)descriptor.getOffset();
	}
}
