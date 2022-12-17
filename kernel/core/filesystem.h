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

	void CreateFile(const std::string16 &path);
	bool ReadFile(const std::string16 &path, byte *&contents, ull &length);
	void WriteFile(const std::string16 &path, byte *contents, ull length);

	DirectoryIterator *GetDirectoryIterator(const std::string16 &path);
	void RemoveDirectory(const std::string16 &path);
	void CreateDirectory(const std::string16 &path, const std::string16 &name);
}