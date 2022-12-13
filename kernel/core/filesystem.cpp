#include "filesystem.h"
#include "../utils/string.h"
#include "../drivers/disk.h"
#include "../utils/string.h"
#include "sys.h"

using namespace Disk;
using namespace std;

namespace Filesystem
{
	// part chs start - chs end
	//  00 21 00 - 0e 38 01
	//	sec 0x20 - sec 0x799 (maybe it has to be 0x800?)
	template <typename T>
	using Unalg = UnalignedField<T>;

	class Partition
	{
	public:
		int lbaStart, lbaLen;
		byte disk;
		char letter;

		virtual void formatPartition() = 0;
		virtual void listDirectoryEntries(const std::string &path) = 0;
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

		//...
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
			inline bool isReadOnly() { return mask & readOnly; }
			inline bool isHidden() { return mask & hidden; }
			inline bool isSystemFile() { return mask & system; }
			inline bool isVolumeId() { return mask & volume_id; }
			inline bool isDirectory() { return mask & directory; }
			inline bool isArchive() { return mask & archive; }
			inline bool isLongFilename() { return mask & longName; }

			inline void setReadOnly(bool value)
			{
				if (value)
					mask |= readOnly;
				else
					mask &= ~readOnly;
			}
			inline void setHidden(bool value)
			{
				if (value)
					mask |= hidden;
				else
					mask &= ~hidden;
			}
			inline void setSystemFile(bool value)
			{
				if (value)
					mask |= system;
				else
					mask &= ~system;
			}
			inline void setVolumeId(bool value)
			{
				if (value)
					mask |= volume_id;
				else
					mask &= ~volume_id;
			}
			inline void setDirectory(bool value)
			{
				if (value)
					mask |= directory;
				else
					mask &= ~directory;
			}
			inline void setArchive(bool value)
			{
				if (value)
					mask |= archive;
				else
					mask &= ~archive;
			}
			inline void setLongFilename(bool value)
			{
				if (value)
					mask |= longName;
				else
					mask &= ~longName;
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
				if (attributes.isVolumeId())
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
		/*constexpr ull testConst = sizeof(DirectoryEntry),
					  testConst2 = sizeof(LongFilenameEntry);*/

		class Partition : public Filesystem::Partition
		{
		public:
			// FAT info
			uint dataSectorOffset,
				fatOffset,
				sectorsPerFAT,
				rootDirCluster;
			byte fatCount, sectorsPerCluster;

			virtual void formatPartition() {}
			virtual void listDirectoryEntries(const std::string &path)
			{
				// load the cluster/clusters
				ull bufferSize = 512 * sectorsPerCluster, entryLen = bufferSize / sizeof(DirectoryEntry);
				byte *buffer = new byte[bufferSize];
				Disk::accessATAdrive(accessDir::read, disk, dataSectorOffset + rootDirCluster * sectorsPerCluster, sectorsPerCluster, buffer);

				// examine them
				DirectoryEntry *entryBuffer = (DirectoryEntry *)buffer;
				uint i;
				// ull fileCount = 0;
				string16 longNameBuffer;
				bool longNameFlags[16];
				for (i = 0; i < 16; i++)
					longNameFlags[i] = false;
				i = 0;
				while (i < entryLen)
				{
					DirectoryEntry *rawEntry = entryBuffer + i;
					if (rawEntry->unknown.unused1[0] == 0x00) // no more files or directory
					{
						// cout << "File count: " << fileCount << '\n';
						delete[] buffer;
						return;
					}
					else if (rawEntry->unknown.unused1[0] != 0xe5)
					{
						// do something
						if (rawEntry->unknown.attributes == FileAttributes::longName)
						{
							// long filename
							LongFilenameEntry &entry = rawEntry->longFileNameEntry;
							string16 str = entry.getString();
							byte order = entry.order & 0xf, // if entry.order & 0x40, this is the last entry
								insertAt = 0;
							for (byte k = 0; k < order; k++)
								if (longNameFlags[k])
									insertAt++;
							longNameBuffer.insert(str, insertAt * 13);
						}
						else
						{
							// regular entry
							Standard83Entry &entry = rawEntry->standard83entry;
							// fileCount++;
							if (entry.attributes == FileAttributes::volume_id)
							{
								cout << "Volume id: \"" << entry.getString() << "\"\n";
							}
							else
							{
								if (entry.attributes.isDirectory())
								{
									cout << (entry.attributes.isHidden() ? "Hidden directory: " : "Directory: ");
									if (longNameBuffer.length() != 0)
									{
										cout << longNameBuffer;
										longNameBuffer.erase();
									}
									else
										cout << entry.getString();
									cout << " (" << ostream::base::bin << (uint)entry.attributes.get() << ostream::base::dec << ")\n";
								}
								else
								{
									cout << (entry.attributes.isHidden() ? "Hidden file: " : "File: ");
									if (longNameBuffer.length() != 0)
									{
										cout << longNameBuffer;
										longNameBuffer.erase();
									}
									else
										cout << entry.getString();
									cout << " (" << ostream::base::bin << (uint)entry.attributes.get() << ostream::base::dec << ")\n";
								}
							}
						}
					}
					i++;
				}

				cout << "Directory bigger than a cluster: Multi-cluster files are not implemented yet!\n";
				System::blueScreen();
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

	void listDirectoryEntries(const string &path)
	{
		char drive = toLower(path[0]);

		for (auto &part : *partitions)
			if (part->letter == drive)
				part->listDirectoryEntries(path);
	}
}