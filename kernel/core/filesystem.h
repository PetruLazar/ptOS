#pragma once
#include "../../libc/types.h"
#include "../drivers/disk.h"
#include "../utils/string.h"

namespace Filesystem
{
	enum class result
	{
		success,
		fileDoesNotExist,
		invalidPath,
		notADirectory,
		notAFile,
		directoryNotEmpty,
		partitionFull,
		fileIsReadOnly,
	};

	class DirectoryIterator
	{
	public:
		virtual ~DirectoryIterator(){};

		virtual void reset() = 0;
		virtual void advance() = 0;
		virtual void advance(uint count) = 0;
		virtual void advanceTo(const std::string16 &entryName) = 0;
		virtual bool finished() = 0;

		virtual std::string16 getString() = 0;
		virtual ull getSize() = 0;

		virtual bool isDirectory() = 0;
		virtual bool isFile() = 0;
	};

	// FAT32 filesystem
	void Initialize();
	void CleanUp();
	void detectPartitions(const Disk::Device &disk, byte *bootsector);

	void formatPartition(char driveLetter);

	bool CreateFile(const std::string16 &path, byte *contents = nullptr, ull length = 0);
	bool RemoveFile(const std::string16 &path);

	bool ReadFile(const std::string16 &path, byte *&contents, ull &length);
	bool WriteFile(const std::string16 &path, byte *contents, ull length);

	DirectoryIterator *GetDirectoryIterator(const std::string16 &path);
	bool RemoveDirectory(const std::string16 &path);
	bool CreateDirectory(const std::string16 &path);

	bool Move(const std::string16 &src, const std::string16 &dest);
	bool Copy(const std::string16 &src, const std::string16 &dest);
}