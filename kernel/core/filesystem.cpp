#include "filesystem.h"
#include "../utils/string.h"
#include "../drivers/disk.h"
#include "sys.h"

using namespace Disk;
using namespace std;

namespace Filesystem
{
	template <typename T>
	using Unalg = UnalignedField<T>;

	class Partition
	{
	public:
		int lbaStart, lbaLen;
		byte disk;
		char letter;

		virtual void formatPartition() = 0;
		virtual DirectoryIterator *listDirectoryEntries(const std::string &path) = 0;
		virtual void removeDirectory(const std::string &path) = 0;
		virtual void createDirectory(const std::string &path, const std::string &name) = 0;
	};

	vector<Partition *> *partitions;

	class MBRpartitionEntry
	{
	public:
		byte attributes, startHead;
		word startSecAndCyl;
		byte type, endHead;
		word endSecAndCyl;
		uint lbaStart, lbaLen;
	};

	class BiosParameterBlock
	{
	public:
		byte jumpCode[3];
		char identifier[8];
		UnalignedField<word> bytesPerSector;
		byte sectorsPerCluster;
		word nrOfReservedSectors;
		byte nrOfFATs;
		UnalignedField<word> nrOfRootDirEntries,
			sectorsInVolume;
		byte mediaDescriptorType;
		word sectorsPerFat, // unused
			sectorsPerTrack,
			headCount;
		uint nrOfHiddenSectors,
			largeSectorCount;
	};
	class ExtendedBootRecord
	{
	public:
		UnalignedField<uint> sectorsPerFAT;
		ushort flags,
			FATversion;
		UnalignedField<uint> rootDirClusterNr;
		ushort FSinfoSectNr,
			backupBootsectorSectNr;
		byte reserved[12];
		byte driveNr;
		byte reservedOrFlags;
		byte signature;
		UnalignedField<uint> volSerialNr;
		char volumeLabelString[11];
		char systemIdString[8];
		//..boot code and 0xaa55 for the rest of the sector
	};

	namespace FAT32
	{
		class FSInfoStruct
		{
		public:
			uint leadSignature;
			byte reserved[480];
			uint midSignature,
				lastFreeClusterCount,
				clusterStartHint;
			byte reserved2[12];
			uint trailSignature;
		};

		class FileAttributes
		{
			byte mask;

		public:
			enum fileAttributes : byte
			{
				readOnly = 0x01,
				hidden = 0x02,
				system = 0x04,
				volume_id = 0x08,
				directory = 0x10,
				archive = 0x20,

				longName = 0xf
			};

			FileAttributes() : mask(0) {}
			FileAttributes(byte attributes) : mask(attributes) {}

			inline bool isAny(byte attributes) { return mask & attributes; }
			inline bool isAll(byte attributes) { return (~mask & attributes) == 0; }

			inline void set(byte bitSelect, bool value)
			{
				if (value)
					mask |= bitSelect;
				else
					mask &= ~bitSelect;
			}

			inline fileAttributes get() { return (fileAttributes)mask; }
			inline void set(byte attributes) { mask = attributes; }
			inline void operator=(fileAttributes other) { mask = other; }
			inline bool operator==(fileAttributes other) { return mask == other; }
		};
		class Standard83Entry
		{
		public:
			char filename[11];
			FileAttributes attributes;
			byte reserved, creationTimeSmall;
			word createTime, creationDate,
				lastsAccesDate,
				firstClusterHigh,
				lastChangeTime, lastChangeDate,
				firstClusterLow;
			uint fileSize;

			inline string getString()
			{
				if (attributes.isAny(FileAttributes::volume_id))
				{
					string name(filename, 11);
					ull pos = 11;
					while (name[--pos] == ' ')
						name.pop_back();
					return name;
				}
				if (attributes.isAny(FileAttributes::directory))
				{
					string name(filename, 11);
					ull pos = name.firstOf(' ');
					if (pos != string::npos)
						name.resize(pos);
					return name;
				}
				// else
				string name(filename, 8);
				ull pos = name.firstOf(' ');
				if (pos != string::npos)
					name.resize(pos);
				name += '.';
				name.append(filename + 8, 3);
				return name;
			}
		};
		class LongFilenameEntry
		{
		public:
			byte order;
			Unalg<char16_t> namePart1[5];
			FileAttributes attributes;
			byte longEntryType,
				creationCheckSum;
			char16_t namePart2[6];
			word zero;
			char16_t namePart3[2];

			inline string16 getString()
			{
				string16 str;
				str.append((char16_t *)namePart1, 5);
				str.append(namePart2, 6);
				str.append(namePart3, 2);
				return str;
			}
		};
		class UnknownDirEntry
		{
		public:
			byte unused1[11];
			FileAttributes attributes;
			byte unused2[20];
		};
		union DirectoryEntry
		{
			Standard83Entry standard83entry;
			LongFilenameEntry longFileNameEntry;
			UnknownDirEntry unknown;
		};

		class DirectoryIterator : public Filesystem::DirectoryIterator
		{
			byte *directoryData;
			UnknownDirEntry *currentIterator;
			class LongNameBuffer
			{
				vector<bool> longNameOrderFlags;
				string16 *contents;

			public:
				LongNameBuffer() : contents(new string16), longNameOrderFlags(16, false) {}
				~LongNameBuffer() { delete contents; }

				void clear()
				{
					for (uint i = 0; i < 16; i++)
						longNameOrderFlags[i] = false;
					contents->erase();
				}
				void insert(const string16 &segm, byte order)
				{
					uint index = 0;
					for (byte i = 0; i < order; i++)
						if (longNameOrderFlags[i])
							index++;
					contents->insert(segm, index * 13);
				}
				string16 *getContents() { return contents; }

				bool empty() { return contents->at(0) == 0; }
			} longNameBuffer;

		public:
			inline DirectoryIterator(byte *directoryData) : directoryData(directoryData), currentIterator((UnknownDirEntry *)directoryData), longNameBuffer()
			{
				if (currentIterator != nullptr)
				{
					--currentIterator;
					advance();
				}
			}
			~DirectoryIterator() { delete[] directoryData; }

			void advance()
			{
				if (currentIterator == nullptr)
					return;
				longNameBuffer.clear();

				while (true)
				{
					++currentIterator;
					if (currentIterator->unused1[0] == 0x00)
					{
						// end of directory reached
						currentIterator = nullptr;
						return;
					}
					if (currentIterator->unused1[0] == 0xe5)
						// entry not used
						continue;
					if (currentIterator->attributes == FileAttributes::longName)
					{
						// long filename entry
						LongFilenameEntry &entry = *(LongFilenameEntry *)currentIterator;
						string16 segm = entry.getString();
						longNameBuffer.insert(segm, entry.order & 0xf);
						continue;
					}
					// standard entry
					Standard83Entry &entry = *(Standard83Entry *)currentIterator;
					if (entry.attributes.isAny(FileAttributes::volume_id))
						// volume id, not a file
						continue;
					// apply long name if long name buffer is not empty
					if (longNameBuffer.empty())
					{
						// cout << "Debug:"
						//  build the string16 name from the short name
						string16 segm;
						if (entry.attributes.isAny(FileAttributes::directory))
						{
							// this is a directory, no extension
							for (byte i = 0; i < 11; i++)
								if (entry.filename[i] != ' ')
									segm += entry.filename[i];
								else
									break;
						}
						else
						{
							// this is a file, has extension
							for (byte i = 0; i < 8; i++)
								if (entry.filename[i] != ' ')
									segm += entry.filename[i];
								else
									break;
							segm += '.';
							for (byte i = 8; i < 11; i++)
								if (entry.filename[i] != ' ')
									segm += entry.filename[i];
								else
									break;
						}
						longNameBuffer.insert(segm, 0);
					}
					return;
				}
			}
			bool finished() { return currentIterator == nullptr; }
			string16 getString() { return *longNameBuffer.getContents(); }
		};
		class Partition : public Filesystem::Partition
		{
		public:
			// FAT info
			uint dataSectorOffset,
				fatOffset,
				sectorsPerFAT,
				rootDirCluster;
			byte fatCount, sectorsPerCluster;

			virtual void formatPartition()
			{
			}
			virtual DirectoryIterator *listDirectoryEntries(const std::string &path)
			{
				// load the cluster/clusters
				ull bufferSize = 512 * sectorsPerCluster, entryLen = bufferSize / sizeof(DirectoryEntry);
				byte *buffer = new byte[bufferSize];
				Disk::accessATAdrive(accessDir::read, disk, dataSectorOffset + rootDirCluster * sectorsPerCluster, sectorsPerCluster, buffer);

				return new DirectoryIterator(buffer);
			}
			virtual void removeDirectory(const std::string &path) {}
			virtual void createDirectory(const std::string &path, const std::string &name) {}
		};

		bool tryLoadPartition(const Device &disk, MBRpartitionEntry &part)
		{
			// load the volume boot record
			byte *bootRecord = new byte[512];
			int diskNr = &disk - devices;
			BiosParameterBlock &bpm = *(BiosParameterBlock *)bootRecord;
			ExtendedBootRecord &ebr = *(ExtendedBootRecord *)(bootRecord + sizeof(BiosParameterBlock));

			// do some checks on the boot record and FAT structures
			if (accessATAdrive(accessDir::read, diskNr, part.lbaStart, 1, bootRecord) ||
				string(ebr.systemIdString, 8) != "FAT32   " ||
				!(ebr.signature == 0x28 || ebr.signature == 0x29) ||
				((word *)bootRecord)[255] != 0xaa55)
			{
				delete[] bootRecord;
				return false;
			}

			byte *fsInfoSector = new byte[512];
			FSInfoStruct &fsInfo = *(FSInfoStruct *)fsInfoSector;
			if (accessATAdrive(accessDir::read, diskNr, ebr.FSinfoSectNr + part.lbaStart, 1, fsInfoSector) ||
				fsInfo.leadSignature != 0x41615252 ||
				fsInfo.midSignature != 0x61417272 ||
				fsInfo.trailSignature != 0xaa550000)
			{
				delete[] fsInfoSector;
				delete[] bootRecord;
				return false;
			}

			// do something
			FAT32::Partition *ptr = new FAT32::Partition;

			// data about the partition
			ptr->letter = 'c' + partitions->getSize();
			ptr->disk = &disk - devices;
			ptr->lbaStart = part.lbaStart;
			ptr->lbaLen = part.lbaLen;

			// data about the partition content
			ptr->fatCount = bpm.nrOfFATs;
			ptr->fatOffset = part.lbaStart + bpm.nrOfReservedSectors;
			ptr->sectorsPerFAT = ebr.sectorsPerFAT;
			ptr->sectorsPerCluster = bpm.sectorsPerCluster;
			ptr->rootDirCluster = ebr.rootDirClusterNr;
			ptr->dataSectorOffset = ptr->fatOffset + bpm.nrOfFATs * ebr.sectorsPerFAT - 2 * bpm.sectorsPerCluster;

			partitions->push_back(ptr);

			delete[] fsInfoSector;
			delete[] bootRecord;
			return true;
		}
	}

	void Initialize()
	{
		partitions = new vector<Partition *>();
	}
	bool intersectPartition(const MBRpartitionEntry &part1, const MBRpartitionEntry &part2)
	{
		if (part1.type == 0 || part2.type == 0)
			return false;
		return part1.lbaStart < part2.lbaStart ? (part1.lbaStart + part1.lbaLen > part2.lbaStart) : (part2.lbaStart + part2.lbaLen > part1.lbaStart);
	}
	static constexpr byte fat32PartType = 0xc; // fat32 with lba
	bool tryLoadPartition(const Device &disk, MBRpartitionEntry &part)
	{
		// limit-check partition size
		if (part.lbaStart + part.lbaLen > disk.size)
			return false;

		// try load partition by type
		switch (part.type)
		{
		case fat32PartType:
			return FAT32::tryLoadPartition(disk, part);
		default:
			return false;
		}
	}
	void detectPartitions(const Device &disk, byte *bootsector)
	{
		MBRpartitionEntry *partitions = (MBRpartitionEntry *)(bootsector + 0x1be);
		// check for partition intersections, treat the disk as invalid if any are found
		for (int i = 0; i < 3; i++)
			for (int j = i + 1; j < 4; j++)
				if (intersectPartition(partitions[i], partitions[j]))
					return;

		for (int i = 0; i < 4; i++)
			tryLoadPartition(disk, partitions[i]);
	}

	DirectoryIterator *listDirectoryEntries(const string &path)
	{
		char drive = toLower(path[0]);

		for (auto &part : *partitions)
			if (part->letter == drive)
				return part->listDirectoryEntries(path);
	}
}