#include "disk.h"
#include "ide.h"

using namespace PCI;
using namespace std;

namespace Disk
{
	vector<StorageDevice *> *devices;

	void Initialize()
	{
		devices = new vector<StorageDevice *>;

		IDE::Initialize();
	}
	void CleanUp()
	{
		IDE::CleanUp();

		for (auto &device : *devices)
			delete device;
		delete devices;
	}
	void ControllerDetected(DeviceHeader *header)
	{
		switch ((MassStorageController)header->subclass)
		{
		case MassStorageController::IDEController:
			IDE::ControllerDetected(header);
			break;
		default:
			// unsupported device
			break;
		}
	}
}