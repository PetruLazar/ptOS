#include "disk.h"

#include "ide.h"
#include "ahci.h"

using namespace PCI;
using namespace std;

namespace Disk
{
	vector<StorageDevice *> *devices;

	void Initialize()
	{
		devices = new vector<StorageDevice *>;

		IDE::Initialize();
		AHCI::Initialize();
	}
	void CleanUp()
	{
		IDE::CleanUp();
		AHCI::CleanUp();

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
		case MassStorageController::SATAcontroller:
			AHCI::ControllerDetected(header);
			break;
		default:
			// unsupported device
			break;
		}
	}
}