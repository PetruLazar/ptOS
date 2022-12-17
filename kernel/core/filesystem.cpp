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

		// virtual void formatPartition() = 0;

		virtual void CreateFile(string16 &path) = 0;
		virtual bool ReadFile(string16 &path, byte *&contents, ull &length) = 0;
		virtual void WriteFile(string16 &path, byte *contents, ull length) = 0;

		virtual DirectoryIterator *GetDirectoryIterator(string16 &path) = 0;
		virtual void RemoveDirectory(string16 &path) = 0;
		virtual void CreateDirectory(string16 &path, string16 &name) = 0;
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
				freeClusterCount,
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
			inline uint getFirstCluster()
			{
				return firstClusterHigh << 16 | firstClusterLow;
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
			Standard83Entry *getEntry() { return (Standard83Entry *)currentIterator; }
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

			// fsInfo
			uint freeClusterCount;
			uint freeClusterStart;
			uint AllocateCluster()
			{
				// decrease freeClusterCount
				// update fats
				// update freeClusterStart
			}
			void DeallocateCluster(uint clusterNr)
			{
				// increase freeClusterCount
				// update fats
				// update freeClusterStart
			}

			uint *fat;

			static constexpr uint freeCluster = 0,
								  lastCluster = 0x0FFFFFF8,
								  badCluster = 0x0FFFFFF7;

			ull ClusterToLba(uint cluster)
			{
				return dataSectorOffset + cluster * sectorsPerCluster;
			}

			// virtual void formatPartition() {}

			void ReadClusterChain(uint startCluster, byte *&buffer, ull &length)
			{
				// read a single cluster, multi-cluster reading not implemented yet
				/*length = 512 * sectorsPerCluster;
				buffer = new byte[length];
				if (byte err = accessATAdrive(accessDir::read, disk, ClusterToLba(startCluster), sectorsPerCluster, buffer))
				{
					displayError(disk, err);
					delete[] buffer;
					buffer = nullptr;
				}
				// DisplyMemoryBlock(buffer, 256);
				// System::pause(false);
				return;*/

				// determine how many clusters the file/directory occupies
				ull clusterLen = 1;
				uint next = fat[startCluster];
				while (next < lastCluster)
				{
					next = fat[next];
					clusterLen++;
				}

				// actually read all the clusters
				length = 512 * sectorsPerCluster * clusterLen;
				buffer = new byte[length];
				byte *current = buffer;
				for (next = startCluster; next < lastCluster; next = fat[next])
				{
					accessATAdrive(accessDir::read, disk, ClusterToLba(next), sectorsPerCluster, current);
					current += 512 * sectorsPerCluster;
				}
				// cout << "Read " << clusterLen << " clusters\n";
			}

			void CreateFile(string16 &path) override {}
			bool ReadFile(string16 &path, byte *&contents, ull &length) override
			{
				// find the last separator in the path
				ull slash = path.lastOf('/');
				if (slash == string16::npos)
					return false;

				// separate parent directory path from filename
				string16 dirPath(path.data(), slash + 1);
				path.erase(0, slash + 1);

				// traverse the directory to find the file
				auto it = GetDirectoryIterator(dirPath);
				while (!it->finished())
				{
					if (it->getString() == path)
					{
						// file found
						auto entry = it->getEntry();
						ReadClusterChain(entry->getFirstCluster(), contents, length);
						length = it->getEntry()->fileSize;
						delete it;
						return true;
					}
					it->advance();
				}

				delete it;
				return false;
			}
			void WriteFile(string16 &path, byte *contents, ull length) override {}

			DirectoryIterator *GetDirectoryIterator(string16 &path) override
			{
				if (path[0] != '/')
				{
					return nullptr;
				}

				path.erase(0); // erase the slash at the beginning
				if (path[path.length() - 1] == '/')
					path.pop_back(); // erase the slash at the end if there is one

				// analyse path
				string16 part;
				ull bufferSize;
				byte *buffer;
				ReadClusterChain(rootDirCluster, buffer, bufferSize);
				while (path.length() > 0)
				{
					ull slash = path.firstOf('/');
					if (slash != string16::npos)
					{
						// slash found
						part.assign(path.data(), slash);
						path.erase(0, slash + 1);
					}
					else
					{
						// slash not found
						part.assign(path);
						path.erase();
					}

					DirectoryIterator iterator(buffer);
					bool cont = false;
					while (!iterator.finished())
					{
						if (iterator.getString() == part)
						{
							ReadClusterChain(iterator.getEntry()->getFirstCluster(), buffer, bufferSize);
							cont = true;
							break;
						}
						iterator.advance();
					}
					if (cont)
						continue;
					return nullptr;
				}

				// load the cluster/clusters

				return new DirectoryIterator(buffer);
			}
			void RemoveDirectory(std::string16 &path) override {}
			virtual void CreateDirectory(std::string16 &path, std::string16 &name) override {}
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
			cout << "Last known free cluster count: " << fsInfo.freeClusterCount << "\n"
				 << "Hint for first free cluster: " << fsInfo.clusterStartHint << '\n';
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

			ptr->freeClusterCount = fsInfo.freeClusterCount;
			ptr->freeClusterStart = fsInfo.clusterStartHint;

			// load the whole fat in memory (just one for now)
			byte *buffer = new byte[512 * ptr->sectorsPerFAT];
			// cout << "Sectors per FAT: " << ptr->sectorsPerFAT << ", drive " << ptr->disk << '\n';
			ull fatSectorsRead = 0;
			uint sectorsToRead = ptr->sectorsPerFAT;
			while (sectorsToRead > 0xff)
			{
				if (byte err = accessATAdrive(accessDir::read, ptr->disk, ptr->fatOffset + fatSectorsRead, 0xff, buffer + fatSectorsRead * 512))
				{
					Disk::displayError(ptr->disk, err);
					delete[] fsInfoSector;
					delete[] bootRecord;
					return false;
				}

				sectorsToRead -= 0xff;
				fatSectorsRead += 0xff;
			}
			if (byte err = accessATAdrive(accessDir::read, ptr->disk, ptr->fatOffset + fatSectorsRead, sectorsToRead, buffer + fatSectorsRead * 512))
			{
				Disk::displayError(ptr->disk, err);
				delete[] fsInfoSector;
				delete[] bootRecord;
				return false;
			}
			ptr->fat = (uint *)buffer;

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

	bool ReadFile(const string16 &path, byte *&contents, ull &length)
	{
		string16 path_copy = path;
		if (path_copy.length() > 1 && path_copy[1] != ':')
		{
			// path does not contain a drive letter
			return false;
		}
		char drive = toLower(path_copy[0]);
		path_copy.erase(0, 2);

		for (auto &part : *partitions)
			if (part->letter == drive)
				return part->ReadFile(path_copy, contents, length);
		return false;
	}

	DirectoryIterator *GetDirectoryIterator(const string16 &path)
	{
		string16 path_copy = path;
		if (path_copy.length() > 1 && path_copy[1] != ':')
		{
			// path does not contain a drive letter
			return nullptr;
		}
		char drive = toLower(path_copy[0]);
		path_copy.erase(0, 2);

		for (auto &part : *partitions)
			if (part->letter == drive)
				return part->GetDirectoryIterator(path_copy);
		return nullptr;
	}
}