#pragma once
#include "../utils/types.h"
#include "../drivers/disk.h"
#include "../utils/string.h"

namespace Filesystem
{
	// FAT32 filesystem
	void Initialize();
	void detectPartitions(const Disk::Device &disk, byte *bootsector);

	void formatPartition(char driveLetter);
	void listDirectoryEntries(const std::string &path);
	void removeDirectory(const std::string &path);
	void CreateDirectory(const std::string &path, const std::string &name);
}