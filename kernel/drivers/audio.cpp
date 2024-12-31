#include "audio.h"

using namespace PCI;
using namespace std;

namespace Audio
{
	void DeviceDetected(PCIDevice &device)
	{
		// cout << "Audio device detected:\nVendor ID = 0x";
		// cout << ostream::base::hex << header->vendorId << "\nDevice ID = 0x" << header->deviceId << '\n';
		// cout << "BAR0 = " << header->bar0 << '\n';
		// cout << "BAR1 = " << header->bar1 << '\n';
		// cout << "BAR2 = " << header->bar2 << '\n';
		// cout << "BAR3 = " << header->bar3 << '\n';
		// cout << "BAR4 = " << header->bar4 << '\n';
		// cout << "BAR5 = " << header->bar5 << '\n';
		// cout << ostream::base::dec;
	}
}