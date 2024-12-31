#include "filesystem.h"
#include "fat32.h"

// #include "sys.h"
#include "mem.h"

using namespace Disk;
using namespace std;

namespace Filesystem
{
	template <typename T>
	using Unalg = UnalignedField<T>;

	vector<Partition *> *partitions;

	void Initialize()
	{
		partitions = new vector<Partition *>();
	}
	void CleanUp()
	{
		// each partition will be deleted by the disk driver
		delete partitions;
	}
	static constexpr byte fat32PartType = 0xc; // fat32 with lba
	bool tryLoadPartition(Disk::Partition *&part)
	{
		// try load partition by type
		if (FAT32::tryLoadPartition(part))
			return true;
		return false;
	}
	void detectPartitions(StorageDevice *disk)
	{
		for (auto &part : disk->partitions)
			tryLoadPartition(part);
	}

	void registerPartition(Partition *part)
	{
		partitions->push_back(part);
	}
	Partition *getPartition(char letter)
	{
		for (auto &part : *partitions)
			if (part->letter == letter)
				return part;
		return nullptr;
	}

	result CreateFile(const string16 &path, byte *contents, ull length)
	{
		if (path.length() > 1 && path[1] != ':')
			// path does not contain a drive letter
			return result::invalidPath;

		auto part = getPartition(toLower(path[0]));
		if (part == nullptr)
			return result::invalidPartition;

		string16 path_copy(path.data() + 2);
		return part->CreateFile(path_copy, contents, length);
	}
	result RemoveFile(const string16 &path)
	{
		if (path.length() > 1 && path[1] != ':')
			return result::invalidPath;

		auto part = getPartition(toLower(path[0]));
		if (part == nullptr)
			return result::invalidPartition;

		string16 path_copy(path.data() + 2);
		return part->RemoveFile(path_copy);
	}

	result ReadFile(const string16 &path, byte *&contents, ull &length)
	{
		if (path.length() > 1 && path[1] != ':')
			return result::invalidPath;

		auto part = getPartition(toLower(path[0]));
		if (part == nullptr)
			return result::invalidPartition;

		string16 path_copy(path.data() + 2);
		return part->ReadFile(path_copy, contents, length);
	}
	result WriteFile(const string16 &path, byte *contents, ull length)
	{
		if (path.length() > 1 && path[1] != ':')
			return result::invalidPath;

		auto part = getPartition(toLower(path[0]));
		if (part == nullptr)
			return result::invalidPartition;

		string16 path_copy(path.data() + 2);
		return part->WriteFile(path_copy, contents, length);
	}

	DirectoryIterator *GetDirectoryIterator(const string16 &path)
	{
		if (path.length() > 1 && path[1] != ':')
			return nullptr;

		auto part = getPartition(toLower(path[0]));
		if (part == nullptr)
			return nullptr;

		string16 path_copy(path.data() + 2);
		return part->GetDirectoryIterator(path_copy);
	}
	result RemoveDirectory(const string16 &path)
	{
		if (path.length() > 1 && path[1] != ':')
			return result::invalidPath;

		auto part = getPartition(toLower(path[0]));
		if (part == nullptr)
			return result::invalidPartition;

		string16 path_copy(path.data() + 2);
		return part->RemoveDirectory(path_copy);
	}
	result CreateDirectory(const string16 &path)
	{
		if (path.length() > 1 && path[1] != ':')
			return result::invalidPath;

		auto part = getPartition(toLower(path[0]));
		if (part == nullptr)
			return result::invalidPartition;

		string16 path_copy(path.data() + 2);
		return part->CreateDirectory(path_copy);
	}

	result Move(const string16 &src, const string16 &dest)
	{
		return (result)-1;
	}
	result Copy(const string16 &src, const string16 &dest)
	{
		return (result)-1;
	}

	string partitionList()
	{
		string ret;
		for (auto &part : *partitions)
		{
			ret += toUpper(part->letter);
		}
		return ret;
	}
	void displayPartitions()
	{
		cout << "List of " << partitions->getSize() << ":\n";
		for (auto &part : *partitions)
		{
			cout << "\n\t" << toUpper(part->letter) << ": " << part->lbaLen << " sectors at " << part->lbaStart << ".\n";
		}
	}
}