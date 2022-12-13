#include "gdt.h"
#include "../utils/iostream.h"

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

	void Initialize()
	{
		byte count = 0;
		globalDescriptorTable[count++] = SegmentDescriptor();			  // null descriptor
		globalDescriptorTable[count++] = CodeSegmentDescriptor(0, false); // code
		globalDescriptorTable[count++] = DataSegmentDescriptor(0);		  // data
		globalDescriptorTable[count++] = CodeSegmentDescriptor(0, true);

		Descriptor descriptor((qword)globalDescriptorTable, sizeof(SegmentDescriptor) * count - 1);
		loadGDT(descriptor);
		updateSegmentRegisters();
	}
	void testGDT()
	{
		Descriptor descriptor;
		getCurrentGDT(descriptor);
		cout << "Size: " << descriptor.getSize()
			 << "\nAddress: " << (void *)descriptor.getOffset();
	}
}
