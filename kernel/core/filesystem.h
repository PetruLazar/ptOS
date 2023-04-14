#pragma once
#include <types.h>
#include "../drivers/disk.h"
#include "../utils/string.h"

namespace Filesystem
{
	enum class result
	{
		success,
		fileDoesNotExist,
		directoryDoesNotExist,
		invalidPath,
		invalidPartition,
		notADirectory,
		notAFile,
		directoryNotEmpty,
		deletionNotAllowed,
		partitionFull,
		fileIsReadOnly,
		nameNotUnique,
		nameTooLong
	};

	inline std::string resultAsString(result res)
	{
		switch (res)
		{
		case result::success:
			return "success";
		case result::fileDoesNotExist:
			return "file does not exist";
		case result::directoryDoesNotExist:
			return "directory does not exist";
		case result::invalidPath:
			return "invalid path";
		case result::invalidPartition:
			return "invalid partition";
		case result::notADirectory:
			return "not a directory";
		case result::notAFile:
			return "not a file";
		case result::directoryNotEmpty:
			return "directory not empty";
		case result::deletionNotAllowed:
			return "deletion not allowed";
		case result::partitionFull:
			return "partition full";
		case result::fileIsReadOnly:
			return "file is read only";
		case result::nameNotUnique:
			return "name not unique";
		case result::nameTooLong:
			return "name too long";
		}
		return "unknown";
	}

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

	void Initialize();
	void CleanUp();
	void detectPartitions(const Disk::Device &disk, byte *bootsector);

	void formatPartition(char driveLetter);

	result CreateFile(const std::string16 &path, byte *contents = nullptr, ull length = 0);
	result RemoveFile(const std::string16 &path);

	result ReadFile(const std::string16 &path, byte *&contents, ull &length);
	result WriteFile(const std::string16 &path, byte *contents, ull length);

	DirectoryIterator *GetDirectoryIterator(const std::string16 &path);
	result RemoveDirectory(const std::string16 &path);
	result CreateDirectory(const std::string16 &path);

	result Move(const std::string16 &src, const std::string16 &dest);
	result Copy(const std::string16 &src, const std::string16 &dest);
}