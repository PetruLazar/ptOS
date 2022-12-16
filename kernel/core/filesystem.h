#pragma once
#include "../utils/types.h"
#include "../drivers/disk.h"
#include "../utils/string.h"

namespace Filesystem
{
	class DirectoryIterator
	{
	public:
		virtual ~DirectoryIterator(){};

		virtual void advance() = 0;
		virtual bool finished() = 0;
		virtual std::string16 getString() = 0;
	};

	// FAT32 filesystem
	void Initialize();
	void detectPartitions(const Disk::Device &disk, byte *bootsector);

	void formatPartition(char driveLetter);
	DirectoryIterator *listDirectoryEntries(const std::string &path);
	void removeDirectory(const std::string &path);
	void CreateDirectory(const std::string &path, const std::string &name);
}