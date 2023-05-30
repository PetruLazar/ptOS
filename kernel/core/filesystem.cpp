#include "filesystem.h"
#include "sys.h"
#include "mem.h"

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
		StorageDevice *disk;
		char letter;

		// virtual void formatPartition() = 0;

		virtual result CreateFile(string16 &path, byte *contents, ull length) = 0;
		virtual result RemoveFile(string16 &path) = 0;

		virtual result ReadFile(string16 &path, byte *&contents, ull &length) = 0;
		virtual result WriteFile(string16 &path, byte *contents, ull length) = 0;

		virtual DirectoryIterator *GetDirectoryIterator(string16 &path) = 0;
		virtual result RemoveDirectory(string16 &path) = 0;
		virtual result CreateDirectory(string16 &path) = 0;

		virtual void CleanUp() = 0;
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
		static constexpr int maxShortFilenameSuffix = 100000; // the max value of a suffix for short filename entrie that have a corresponding long name, value not included

		extern "C" byte FATchecksum(char shortName[11]);
		inline ull shortFilenameLen(char shortFilename[11])
		{
			for (ull i = 0; i < 8; i++)
				if (shortFilename[i] == ' ')
					return i;
			return 8;
		}
		string shortFilenameAsString(char shortFilename[11])
		{
			string filename;
			for (int i = 0; i < 8; i++)
			{
				if (shortFilename[i] == ' ')
					break;

				filename += toLower((char)shortFilename[i]);
			}
			if (shortFilename[8] != ' ')
			{
				// has extension
				filename += '.';
				for (int i = 8; i < 11; i++)
				{
					if (shortFilename[i] == ' ')
						break;

					filename += toLower((char)shortFilename[i]);
				}
			}
			return filename;
		}
		string16 shortFilenameAsString16(char shortFilename[11])
		{
			string16 filename;
			for (int i = 0; i < 8; i++)
			{
				if (shortFilename[i] == ' ')
					break;

				filename += toLower((char)shortFilename[i]);
			}
			if (shortFilename[8] != ' ')
			{
				// has extension
				filename += '.';
				for (int i = 8; i < 11; i++)
				{
					if (shortFilename[i] == ' ')
						break;

					filename += toLower((char)shortFilename[i]);
				}
			}
			return filename;
		}
		bool needsLongFilename(const string16 &filename)
		{
			ull len = filename.length();

			ull dot = filename.lastOf(u'.');
			if (dot == string16::npos)
				dot = len;

			if (dot > 8 || len - dot > 4) // filename or extension too long
				return true;

			for (ull i = 0; i < len; i++)
			{
				char16_t ch = filename[i];
				if (ch & 0xff00 ||				// filename contains wide chars
					ch == u' ' ||				// filename contains spaces
					(ch >= u'A' && ch <= u'Z')) // filename contains uppercase letters
					return true;
			}
			return false;
		}
		// returns wheather a long filename has to be used
		void suffixShortFilename(char shortFilename[11], int suffix, ull shortFilenameLen)
		{
			char str[16];
			lltos(str, suffix);
			ull suffixLen = strlen(str);
			{
				ull shortFilenameLenLim = 7 - suffixLen;
				if (shortFilenameLen > shortFilenameLenLim)
					shortFilenameLen = shortFilenameLenLim;
			}
			shortFilename[shortFilenameLen++] = '~';
			for (ull i = 0; i < suffixLen; i++)
				shortFilename[shortFilenameLen + i] = str[i];
		}
		bool getShortFilename(const string16 &filename, char shortFilename[11])
		{
			bool needsLfn = needsLongFilename(filename);
			ull len = filename.length();

			ull dot = filename.lastOf(u'.');
			if (dot == string16::npos)
				dot = len;

			int shorti = 0, shortlim = 8;
			for (ull i = 0; i < dot; i++) // copy chars from the filename
			{
				char16_t wch = filename[i];
				if (wch != ' ' &&				 // if not space
					(unsigned short)wch <= 0xff) // if not wide char
				{
					shortFilename[shorti++] = toUpper((char)wch); // copy
					if (shorti == shortlim)
						break; // if short filename is full, break
				}
			}

			// pad with spaces
			while (shorti < 8)
				shortFilename[shorti++] = ' ';

			shortlim = 11;
			for (ull i = dot + 1; i < len; i++) // copy chars from the extension
			{
				char16_t wch = filename[i];
				if (wch != ' ' &&				 // if not space
					(unsigned short)wch <= 0xff) // if not wide char
				{
					shortFilename[shorti++] = toUpper((char)wch); // copy
					if (shorti == shortlim)
						break; // if short filename is full, break
				}
			}

			// pad with spaces
			while (shorti < 11)
				shortFilename[shorti++] = ' ';

			return needsLfn;
		}

		class FSInfoStruct
		{
		public:
			uint leadSignature;
			byte reserved[480];
			uint midSignature,
				freeClusterCount,
				freeClusterStartHint;
			byte reserved2[12];
			uint trailSignature;
		};

		class FileAttributes
		{
			byte mask;

		public:
			enum fileAttributes : byte
			{
				none = 0x0,

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
				return shortFilenameAsString(filename);
			}
			inline void setString(const char newFilename[11])
			{
				for (int i = 0; i < 11; i++)
					filename[i] = newFilename[i];
			}
			inline uint getFirstCluster()
			{
				return firstClusterHigh << 16 | firstClusterLow;
			}
			inline void setFirstCluster(uint firstCluster)
			{
				firstClusterHigh = firstCluster >> 16;
				firstClusterLow = firstCluster & 0xffff;
			}
		};
		class LongFilenameEntry
		{
			inline char16_t getChar(ull index)
			{
				switch (index)
				{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
					return namePart1[index];
				default:
					return namePart2[index - 5];
				case 11:
				case 12:
					return namePart3[index - 11];
				}
			}
			inline void setChar(ull index, char16_t value)
			{
				switch (index)
				{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
					namePart1[index] = value;
					break;
				default:
					namePart2[index - 5] = value;
					break;
				case 11:
				case 12:
					namePart3[index - 11] = value;
				}
			}

		public:
			byte order;
			Unalg<char16_t> namePart1[5];
			FileAttributes attributes;
			byte longEntryType,
				shortNameChecksum;
			char16_t namePart2[6];
			word zero;
			char16_t namePart3[2];

			inline string16 getString()
			{
				string16 str;
				for (ull i = 0; i < 13; i++)
				{
					char16_t ch = getChar(i);
					if (ch == 0)
						return str;
					else
						str += ch;
				}
				return str;
			}
			inline void setString(const string16 &name, int offset)
			{
				ull i = 0;
				ull len = name.length() + 1;
				while (i < 13 && offset < len) // copy as much as possible from the string
					setChar(i++, name[offset++]);
				while (i < 13) // pad with 0xffff if the entry is not full
					setChar(i++, 0xffff);
			}
		};
		class UnknownDirEntry
		{
		public:
			byte unused1[11];
			FileAttributes attributes;
			byte unused2[20];

			inline bool isUnused() { return unused1[0] == 0xe5; }
			inline void setUnused() { unused1[0] = 0xe5; }
			inline bool isEnd() { return unused1[0] == 0x00; }
		};
		union DirectoryEntry
		{
			Standard83Entry standard83entry;
			LongFilenameEntry longFileNameEntry;
			UnknownDirEntry unknown;
		};

		class Partition;
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
			Partition *part;
			ull length;
			uint startCluster;

			bool addClusters(ull count);

			UnknownDirEntry *findFree(ull count)
			{
				if (directoryData == nullptr)
					return nullptr;

				// find unused entries
				ull current = 0;
				UnknownDirEntry *i;
				for (i = (UnknownDirEntry *)directoryData; !i->isEnd(); i++)
					if (i->isUnused())
					{
						if (++current == count)
							return ++i - current;
					}
					else
						current = 0;

				// try to allocate after directory data has ended
				ull offset = (byte *)i - directoryData;
				if (offset + (count + 1) * sizeof(UnknownDirEntry) > length)
				{
					// not enough space, increase directory length by a cluster
					if (!addClusters(1))
						return nullptr; // no free clusters
					i = (UnknownDirEntry *)(directoryData + offset);
				}
				// return the entries at the old end
				auto newEnd = i + count;
				newEnd->unused1[0] = 0x00; // mark the new end
				return i;
			}
			bool checkShortFilename(const char shortFilename[11])
			{
				auto it = new DirectoryIterator(directoryData, part, startCluster, length);
				while (!it->finished())
				{
					// compare short filename
					bool eq = true;
					for (int i = 0; i < 11; i++)
						if (shortFilename[i] != it->getStdEntry()->filename[i])
						{
							eq = false;
							break;
						}
					if (eq)
					{
						it->directoryData = nullptr; // so that deallocating the object will not erase the directoryData, still needed by this object
						delete it;
						return true;
					}

					it->advance();
				}
				it->directoryData = nullptr;
				delete it;
				return false;
			}
			bool checkFilename(const string16 &filename)
			{
				auto it = new DirectoryIterator(directoryData, part, startCluster, length);
				while (!it->finished())
				{
					if (filename == it->getString())
					{
						it->directoryData = nullptr;
						delete it;
						return true;
					}

					it->advance();
				}
				it->directoryData = nullptr;
				delete it;
				return false;
			}

			Standard83Entry *createShortEntry(const char shortFilename[11])
			{
				// find space for the std entry
				auto entry = findFree(1);
				if (entry == nullptr)
					return nullptr;
				auto stdEntry = (Standard83Entry *)entry;

				// put the short filename
				stdEntry->setString(shortFilename);
				stdEntry->reserved = 0;
				stdEntry->attributes.set(FileAttributes::none);

				// return the entry
				return stdEntry;
			}

		public:
			inline DirectoryIterator(byte *directoryData, Partition *part, uint startCluster, ull length)
				: directoryData(directoryData), currentIterator((UnknownDirEntry *)directoryData),
				  longNameBuffer(), part(part), startCluster(startCluster), length(length)
			{
				if (currentIterator)
				{
					--currentIterator;
					advance();
				}
			}
			inline DirectoryIterator(Partition *part, uint startCluster)
				: directoryData(nullptr), currentIterator(nullptr),
				  longNameBuffer(), part(part), startCluster(startCluster)
			{
				undo();
				if (currentIterator)
				{
					--currentIterator;
					advance();
				}
			}
			~DirectoryIterator()
			{
				if (directoryData)
					delete[] directoryData;
			}

			inline result flush();
			// re-read the directory from disk, undoing all the changes since the last flush
			inline void undo();

			void advance() override
			{
				if (finished())
					return;
				longNameBuffer.clear();

				while (true)
				{
					++currentIterator;
					if (currentIterator->isEnd())
					{
						// end of directory reached
						return;
					}
					if (currentIterator->isUnused())
						// entry not used
						continue;
					ushort checksum;
					if (currentIterator->attributes == FileAttributes::longName)
					{
						// long filename entry
						LongFilenameEntry &entry = *(LongFilenameEntry *)currentIterator;

						if (longNameBuffer.empty())
							checksum = entry.shortNameChecksum;
						else if (checksum != 0xffff && checksum != entry.shortNameChecksum)
							checksum = 0xffff;

						string16 segm = entry.getString();
						longNameBuffer.insert(segm, entry.order & 0xf);
						continue;
					}
					// standard entry
					Standard83Entry &entry = *(Standard83Entry *)currentIterator;
					if (entry.attributes.isAny(FileAttributes::volume_id))
						// volume id, not a file
						continue;

					bool useShortFilename = false;
					if (longNameBuffer.empty()) // no long filename
						useShortFilename = true;
					else if (checksum == 0xffff || checksum != FATchecksum(entry.filename)) // invalid checksum
					{
						longNameBuffer.clear();
						useShortFilename = true;
					}

					if (useShortFilename)
						//  build the string16 name from the short name
						longNameBuffer.insert(shortFilenameAsString16(entry.filename), 0);

					return;
				}
			}
			void advance(uint count) override
			{
				for (uint i = 0; i < count; i++)
					advance();
			}
			void advanceTo(const string16 &entryName) override
			{
				while (!finished() && getString() != entryName)
					advance();
			}
			bool finished() override { return currentIterator == nullptr || (currentIterator->isEnd() && (byte *)currentIterator >= directoryData); }
			void reset() override
			{
				currentIterator = (UnknownDirEntry *)directoryData;
				if (currentIterator != nullptr)
				{
					--currentIterator;
					advance();
				}
			}

			string16 getString() override { return *longNameBuffer.getContents(); }
			Standard83Entry *getStdEntry() { return (Standard83Entry *)currentIterator; }
			ull getSize() override { return getStdEntry()->fileSize; }

			bool isDirectory() override { return getStdEntry()->attributes.isAny(FileAttributes::directory); }
			bool isFile() override { return !getStdEntry()->attributes.isAny(FileAttributes::directory | FileAttributes::volume_id); }

			result createEntry(const string16 &entryName, Standard83Entry *&entry)
			{
				// ensure the name is unique
				if (checkFilename(entryName))
				{
					return result::nameNotUnique;
				}

				// get the short filename corresponding to the desired filename
				char shortFilename[11];
				bool needsLfn = getShortFilename(entryName, shortFilename);

				if (needsLfn)
				{
					// the entry requires a long filename
					// suffix the filename with a number to make it unique, or return nullptr if not possible
					ull sfnlen = shortFilenameLen(shortFilename);
					int suffix = 1;
					while (true)
					{
						suffixShortFilename(shortFilename, suffix, sfnlen);
						if (!checkShortFilename(shortFilename))
							break;
						suffix++;
						if (suffix == maxShortFilenameSuffix)
							return result::nameNotUnique;
					}

					byte checksum = FATchecksum(shortFilename);

					// find a space long enough for the entry and the long ilename
					ull lfnCount = (entryName.length() - 1) / 13 + 1; // limit lfnCount to 0xf
					if (lfnCount > 0xf)
						return result::nameTooLong;
					auto entries = findFree(lfnCount + 1);
					if (entries == nullptr)
						return result::partitionFull;

					// fill the long filename
					for (ull i = lfnCount - 1; i < lfnCount; i--)
					{
						LongFilenameEntry *lfEntry = (LongFilenameEntry *)entries++;
						lfEntry->attributes.set(FileAttributes::longName);
						lfEntry->longEntryType = 0;
						lfEntry->shortNameChecksum = checksum;
						lfEntry->zero = 0;
						byte order = i + 1;
						lfEntry->order = ((order == lfnCount) ? (0x40 | order) : order);
						lfEntry->setString(entryName, 13 * i);
					}
					// std entry
					Standard83Entry *stdEntry = (Standard83Entry *)entries;
					stdEntry->setString(shortFilename);
					stdEntry->reserved = 0;
					stdEntry->attributes = FileAttributes::none;
					// return the entry
					entry = stdEntry;
					return result::success;
				}
				else
				{
					entry = createShortEntry(shortFilename);
					return result::success;
				}
			}
			void markUnused()
			{
				if (finished())
					return;

				auto preserveIterator = currentIterator--;
				preserveIterator->setUnused();
				while ((byte *)currentIterator >= directoryData && !currentIterator->isUnused() && currentIterator->attributes == FileAttributes::longName)
					(currentIterator--)->setUnused();
				currentIterator = preserveIterator;
				// to do: move the entries that follow the deleted one, and maybe free up a cluster
				advance();
			}

			friend Partition;
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
			ull fsInfoSect;
			FSInfoStruct *fsInfo;

		private:
			void flushFSInfo()
			{
				Disk::result res = disk->access(accessDir::write, fsInfoSect, 1, (byte *)fsInfo);
				if (res != Disk::result::success)
				{
					cout << "Error writing to disk: " << Disk::resultAsString(res) << "\n";
				}
			}

		public:
			uint AllocateCluster()
			{
				// decrease freeClusterCount
				// update fats
				// update freeClusterStart
				if (fsInfo->freeClusterCount == 0)
					return 0;
				else
					fsInfo->freeClusterCount--;
				uint cluster = fsInfo->freeClusterStartHint;
				while (getFatEntry(cluster))
					cluster++;
				fsInfo->freeClusterStartHint = cluster;
				return cluster;
			}
			void DeallocateCluster(uint clusterNr)
			{
				// increase freeClusterCount
				// update fats
				// update freeClusterStart
				updateFatEntry(clusterNr, 0);
				fsInfo->freeClusterCount++;
				if (clusterNr < fsInfo->freeClusterStartHint)
					fsInfo->freeClusterStartHint = clusterNr;
				flushFSInfo();
			}

			// fat table info
		private:
			uint cachedFATSectorNr = -1;
			uint *cachedFATSector = nullptr;
			static constexpr ull FATentriesPerSector = 512 / sizeof(uint);

			/// return the index within the loaded sector
			uint loadFATSector(uint index)
			{
				ull sectorNr = index / FATentriesPerSector;
				uint relIndex = index % FATentriesPerSector;
				if (sectorNr == cachedFATSectorNr)
					return relIndex;
				if (cachedFATSector != nullptr)
				{
					// maybe flush it?
					delete[] cachedFATSector;
				}
				cachedFATSector = (uint *)new byte[512];
				Disk::result res = disk->access(accessDir::read, fatOffset + sectorNr, 1, (byte *)cachedFATSector);
				if (res != Disk::result::success)
				{
					delete[] cachedFATSector;
					cachedFATSectorNr = -1;
					cachedFATSector = nullptr;
					return 0;
				}
				cachedFATSectorNr = sectorNr;
				return relIndex;
			}
			void writeCachedSector()
			{
				if (cachedFATSector)
					for (uint i = 0; i < fatCount; i++)
						disk->access(accessDir::write, fatOffset + i * sectorsPerFAT + cachedFATSectorNr, 1, (byte *)cachedFATSector);
			}

		public:
			void CleanUp() override
			{
				delete[] fsInfo;
				if (cachedFATSector)
					delete[] cachedFATSector;
			}

			uint getFatEntry(uint index)
			{
				index = loadFATSector(index);
				return cachedFATSector[index] & 0x0fffffff; // ignore the highest 4 bits
			}
			void updateFatEntry(uint index, uint value)
			{
				index = loadFATSector(index);
				cachedFATSector[index] = cachedFATSector[index] & 0xf0000000 | value & 0x0fffffff; // leave the highest 4 bits unchanged
				writeCachedSector();
			}

			static constexpr uint freeCluster = 0,
								  lastCluster = 0x0FFFFFF8,
								  badCluster = 0x0FFFFFF7;

			ull ClusterToLba(uint cluster)
			{
				return dataSectorOffset + cluster * sectorsPerCluster;
			}

			// virtual void formatPartition() {}

			uint GetClusterChainLength(uint cluster)
			{
				uint clusterLen = 0;
				for (; cluster < lastCluster; cluster = getFatEntry(cluster))
					clusterLen++;
				return clusterLen;
			}
			void ReadClusterChain(uint startCluster, byte *&buffer, ull &length)
			{
				// actually read all the clusters
				length = 512 * sectorsPerCluster * GetClusterChainLength(startCluster);
				buffer = (byte *)Memory::Allocate(length, 0x1000);
				// buffer = new byte[length];
				byte *current = buffer;
				for (uint next = startCluster; next < lastCluster; next = getFatEntry(next))
				{
					disk->access(accessDir::read, ClusterToLba(next), sectorsPerCluster, current);
					current += 512 * sectorsPerCluster;
				}
				// cout << "Read " << clusterLen << " clusters\n";
			}
			void WriteCluster(uint cluster, byte *buffer, ull length)
			{
				ull clusterLen = 512 * sectorsPerCluster;
				if (length < clusterLen)
				{
					// buffer smaller than a cluster, pad
					byte *padded = new byte[clusterLen];
					memcpy(padded, buffer, length);
					Disk::result res = disk->access(accessDir::write, ClusterToLba(cluster), sectorsPerCluster, padded);
					if (res != Disk::result::success)
					{
						cout << "Error writing to cluster " << cluster << ": " << Disk::resultAsString(res) << '\n';
					}
					delete[] padded;
					return;
				}
				Disk::result res = disk->access(accessDir::write, ClusterToLba(cluster), sectorsPerCluster, buffer);
				if (res != Disk::result::success)
				{
					cout << "Error writing to cluster " << cluster << ": " << Disk::resultAsString(res) << '\n';
				}
			}
			result WriteClusterChain(uint &startCluster, byte *buffer, ull length)
			{
				// cout << "Writing cluster chain (" << length << " bytes), starting at cluster " << startCluster << '\n';
				ull clusterLen = 512 * sectorsPerCluster;
				uint clusterChainLen = startCluster < 2 ? 0 : GetClusterChainLength(startCluster),
					 neededClusterChainLen = length == 0 ? 0 : (length - 1) / clusterLen + 1;
				// cout << "Cluster chain length: " << clusterChainLen << "\nNeeded cluster chain length: " << neededClusterChainLen << '\n';
				if (clusterChainLen < neededClusterChainLen)
				{
					// cluster chain not long enough, allocate missing clusters and write
					uint cluster = startCluster, prevCluster = 0;
					neededClusterChainLen -= clusterChainLen;
					if (neededClusterChainLen > fsInfo->freeClusterCount)
						return result::partitionFull;
					bool isFirstCluster = true;
					while (clusterChainLen > 0)
					{
						WriteCluster(cluster, buffer, length);
						buffer += clusterLen;
						length -= clusterLen;
						prevCluster = cluster;
						cluster = getFatEntry(cluster);
						clusterChainLen--;
						isFirstCluster = false;
					}
					while (neededClusterChainLen > 0)
					{
						cluster = AllocateCluster();
						if (isFirstCluster)
						{
							startCluster = cluster;
							isFirstCluster = false;
						}
						WriteCluster(cluster, buffer, length);
						buffer += clusterLen;
						length -= clusterLen;
						updateFatEntry(prevCluster, cluster);
						prevCluster = cluster;
						neededClusterChainLen--;
					}
					if (cluster)
						updateFatEntry(cluster, lastCluster);
					return result::success;
				}
				else
				{
					// cluster chain already long enough, write and deallocate surplus
					uint cluster = startCluster, prevCluster = 0;
					while (neededClusterChainLen > 0)
					{
						WriteCluster(cluster, buffer, length);
						buffer += clusterLen;
						length -= clusterLen;
						prevCluster = cluster;
						cluster = getFatEntry(cluster);
						neededClusterChainLen--;
					}
					if (startCluster == 0)
						return result::success;
					updateFatEntry(prevCluster, lastCluster);
					while (cluster < lastCluster)
					{
						prevCluster = cluster;
						cluster = getFatEntry(cluster);
						DeallocateCluster(prevCluster);
						updateFatEntry(prevCluster, freeCluster);
					}
					return result::success;
				}
			}

			inline void separateParentpathFromFilepath(string16 &path, string16 &filename)
			{
				ull slash = path.lastOf('/');
				if (slash == string16::npos)
				{
					// no slash, all is filename
					filename = path;
					path.erase();
					return;
				}
				ull filenameLen = path.length() - ++slash;
				filename.assign(path.data() + slash, filenameLen);
				path.erase(slash - 1, filenameLen + 1);
			}
			inline void getTopDirectoryPath(string16 &path, string16 &top)
			{
				ull slash = path.firstOf('/');
				if (slash == string16::npos)
				{
					top.assign(path);
					path.erase();
				}
				else
				{
					top.assign(path.data(), slash);
					path.erase(0, slash + 1);
				}
			}
			inline ull diskSizeFromByteSize(ull byteLength)
			{
				ull sectorLen = 512 * sectorsPerCluster;
				ull rem = byteLength % sectorLen;
				return rem == 0 ? byteLength : (byteLength - rem + sectorLen);
			}

			result CreateFile(string16 &path, byte *contents, ull length) override
			{
				string16 filename;
				separateParentpathFromFilepath(path, filename);
				if (path.length() == 0)
					path = u"/";

				auto it = GetDirectoryIterator(path); // to do: if the directory is read-only, do not remove file
				if (it == nullptr)
					return result::invalidPath;
				Standard83Entry *entry;
				result res = it->createEntry(filename, entry);
				if (res != result::success)
				{
					// could not add new entry
					delete it;
					return res;
				}
				res = it->flush();
				if (res != result::success)
				{
					delete it;
					return res;
				}
				if (contents != nullptr && length > 0)
				{
					uint startCluster = 0;
					result res = WriteClusterChain(startCluster, contents, length);
					if (res != result::success)
					{
						// failed to write, leave the entry in the directory though
						delete it;
						return res;
					}
					entry->setFirstCluster(startCluster);
					it->flush();
				}
				delete it;
				return result::success;
			}
			result RemoveFile(string16 &path) override
			{
				string16 filename;
				separateParentpathFromFilepath(path, filename);
				if (path.length() == 0)
					path = u"/";

				auto it = GetDirectoryIterator(path); // to do: if the directory is read-only, do not remove file
				if (it == nullptr)
					return result::invalidPath;
				it->advanceTo(filename);
				if (it->finished())
				{
					// file not found
					delete it;
					return result::fileDoesNotExist;
				}
				Standard83Entry *entry = it->getStdEntry();
				// check that the entry is not a directory or volume id
				if (entry->attributes.isAny(FileAttributes::directory | FileAttributes::volume_id))
				{
					delete it;
					return result::notAFile;
				}
				//  deallocate all clusters
				uint startCluster = it->getStdEntry()->getFirstCluster();
				WriteClusterChain(startCluster, nullptr, 0);

				// deletion from directory
				it->markUnused();
				it->flush();
				delete it;
				return result::success;
			}

			result ReadFile(string16 &path, byte *&contents, ull &length) override
			{
				// separate parent directory path from filename
				string16 filename;
				separateParentpathFromFilepath(path, filename);
				if (path.length() == 0)
					path = u"/";

				// traverse the directory to find the file
				auto it = GetDirectoryIterator(path);
				if (it == nullptr)
					return result::invalidPath;
				it->advanceTo(filename);
				if (it->finished())
				{
					// file not found
					delete it;
					return result::fileDoesNotExist;
				}
				auto entry = it->getStdEntry();

				// check that the entry is a file
				if (entry->attributes.isAny(FileAttributes::directory | FileAttributes::volume_id))
				{
					delete it;
					return result::notAFile;
				}

				uint firstCluster = entry->getFirstCluster();
				if (firstCluster > 1)
					ReadClusterChain(firstCluster, contents, length);
				else
					contents = nullptr;
				length = it->getStdEntry()->fileSize;
				delete it;
				return result::success;
			}
			result WriteFile(string16 &path, byte *contents, ull length) override
			{
				string16 filename;
				separateParentpathFromFilepath(path, filename);
				if (path.length() == 0)
					path = u"/";

				auto it = GetDirectoryIterator(path);
				if (it == nullptr)
					return result::invalidPath;
				it->advanceTo(filename);

				if (it->finished())
				{
					// file not found
					// to-do: file not found, create it
					delete it;
					return result::fileDoesNotExist;
				}
				auto entry = it->getStdEntry();
				// make sure the entry is a file
				if (entry->attributes.isAny(FileAttributes::directory | FileAttributes::fileAttributes::volume_id | FileAttributes::readOnly))
				{
					delete it;
					return result::notAFile;
				}

				uint firstCluster = entry->getFirstCluster();
				result res = WriteClusterChain(firstCluster, contents, length);
				entry->setFirstCluster(firstCluster);
				if (res == result::success)
				{
					entry->fileSize = length;
					it->flush();
				}
				delete it;
				return res;
			}

			DirectoryIterator *GetDirectoryIterator(string16 &path) override
			{
				if (path[0] != u'/')
				{
					return nullptr;
				}

				path.erase(0); // erase the slash at the beginning
				if (path[path.length() - 1] == u'/')
					path.pop_back(); // erase the slash at the end if there is one

				// analyse path
				string16 part;
				uint cluster = rootDirCluster;
				while (path.length() > 0)
				{
					getTopDirectoryPath(path, part);

					// cout << "{ ";
					DirectoryIterator iterator(this, cluster);
					// cout << "}\n";
					// DisplyMemoryBlock((byte *)iterator.directoryData, 0x80);
					// if (!*(byte *)iterator.currentIterator)
					// {
					// 	cout << '\n';
					// 	iterator.currentIterator = (UnknownDirEntry *)iterator.directoryData;
					// 	iterator.currentIterator--;
					// 	iterator.advance();
					// 	DisplyMemoryBlock((byte *)iterator.currentIterator, 0x80);
					// }
					iterator.advanceTo(part);
					if (iterator.finished())
					{
						cout << "exited\n";
						return nullptr;
					}
					Standard83Entry *entry = iterator.getStdEntry();
					if (!entry->attributes.isAny(FileAttributes::directory))
						return nullptr;
					cluster = entry->getFirstCluster();
				}
				return new DirectoryIterator(this, cluster);
			}
			result RemoveDirectory(std::string16 &path) override
			{
				if (path[path.length() - 1] == '/')
					path.pop_back();

				string16 dirname;
				separateParentpathFromFilepath(path, dirname);
				if (dirname == u"." || dirname == u"..")
					return result::deletionNotAllowed; // cannot delete "." and ".."
				if (path.length() == 0)
					path = u"/";

				auto it = GetDirectoryIterator(path);
				if (it == nullptr)
					return result::invalidPath;

				it->advanceTo(dirname);
				if (it->finished())
				{
					// file not found
					delete it;
					return result::directoryDoesNotExist;
				}
				auto entry = it->getStdEntry();
				// check if the entry is a directory and if the directory is empty
				if (!entry->attributes.isAny(FileAttributes::directory))
				{
					delete it;
					return result::notADirectory;
				}
				uint startCluster = entry->getFirstCluster();
				DirectoryIterator subDir(this, startCluster);
				while (!subDir.finished())
				{
					string16 subEntry = subDir.getString();
					if (subEntry != u"." && subEntry != u"..")
					{
						delete it;
						return result::directoryNotEmpty;
					}
					subDir.advance();
				}

				// remove the directory
				WriteClusterChain(startCluster, nullptr, 0);
				it->markUnused();
				it->flush();
				delete it;
				return result::success;
			}
			result CreateDirectory(std::string16 &path) override
			{
				// the last char could be a slash for directories
				if (path[path.length() - 1] == '/')
					path.pop_back();

				string16 dirname;
				separateParentpathFromFilepath(path, dirname);
				if (path.length() == 0)
					path = u"/";

				auto it = GetDirectoryIterator(path);
				if (it == nullptr) // check that the iterator is valid
					return result::invalidPath;
				Standard83Entry *entry;
				result res = it->createEntry(dirname, entry);
				if (res != result::success)
				{
					// could not add new entry
					delete it;
					return res;
				}
				res = it->flush();
				if (res != result::success)
				{
					// could not add new entry
					delete it;
					return res;
				}
				entry->attributes = FileAttributes::directory;
				entry->fileSize = 0;

				// allocate a cluster for the directory data
				ull dirDataLen = 3 * sizeof(UnknownDirEntry);
				UnknownDirEntry *dirData = new UnknownDirEntry[3];
				dirData[0].unused1[0] = 0;
				uint startCluster = 0;
				res = WriteClusterChain(startCluster, (byte *)dirData, dirDataLen);
				if (res != result::success)
				{
					// could not write directory data: delete the added entry and cleanup allocated data
					delete[] dirData;
					it->reset();
					it->advanceTo(dirname);
					it->markUnused();
					it->flush();
					delete it;
					return res;
				}
				// create the initial directory entries ("." and "..")
				DirectoryIterator subDir((byte *)dirData, this, startCluster, dirDataLen);

				auto itself = subDir.createShortEntry(".          ");
				itself->attributes = FileAttributes::directory;
				itself->setFirstCluster(startCluster);
				itself->fileSize = 0;

				auto parent = subDir.createShortEntry("..         ");
				parent->attributes = FileAttributes::directory;
				parent->setFirstCluster(it->startCluster);
				parent->fileSize = 0;

				entry->setFirstCluster(startCluster);
				subDir.flush();
				it->flush();
				delete it;
				return result::success;
			}
		};

		inline result DirectoryIterator::flush() { return part->WriteClusterChain(startCluster, directoryData, length); }
		inline void DirectoryIterator::undo()
		{
			ull offset = (byte *)currentIterator - directoryData;
			if (directoryData)
				delete[] directoryData;
			part->ReadClusterChain(startCluster, directoryData, length);
			currentIterator = (UnknownDirEntry *)(directoryData + offset);
		}
		inline bool DirectoryIterator::addClusters(ull count)
		{
			if (part->fsInfo->freeClusterCount < count)
				return false;

			ull newLength = length + 512 * part->sectorsPerCluster * count,
				currentIteratorOffset = (byte *)currentIterator - directoryData;
			byte *newDirectoryData = new byte[newLength];
			memcpy(newDirectoryData, directoryData, length);
			delete[] directoryData;
			directoryData = newDirectoryData;
			currentIterator = (UnknownDirEntry *)(directoryData + currentIteratorOffset);
			length = newLength;
			return true;
		}

		bool tryLoadPartition(StorageDevice *disk, MBRpartitionEntry &part)
		{
			// load the volume boot record
			byte *bootRecord = new byte[512];
			BiosParameterBlock &bpm = *(BiosParameterBlock *)bootRecord;
			ExtendedBootRecord &ebr = *(ExtendedBootRecord *)(bootRecord + sizeof(BiosParameterBlock));

			// do some checks on the boot record and FAT structures
			if (disk->access(accessDir::read, part.lbaStart, 1, bootRecord) != Disk::result::success ||
				string(ebr.systemIdString, 8) != "FAT32   " ||
				!(ebr.signature == 0x28 || ebr.signature == 0x29) ||
				((word *)bootRecord)[255] != 0xaa55)
			{
				delete[] bootRecord;
				return false;
			}

			byte *fsInfoSector = new byte[512];
			FSInfoStruct &fsInfo = *(FSInfoStruct *)fsInfoSector;
			if (disk->access(accessDir::read, ebr.FSinfoSectNr + part.lbaStart, 1, fsInfoSector) != Disk::result::success ||
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
			ptr->disk = disk;
			ptr->lbaStart = part.lbaStart;
			ptr->lbaLen = part.lbaLen;

			// data about the partition content
			ptr->fatCount = bpm.nrOfFATs;
			ptr->fatOffset = part.lbaStart + bpm.nrOfReservedSectors;
			ptr->sectorsPerFAT = ebr.sectorsPerFAT;
			ptr->sectorsPerCluster = bpm.sectorsPerCluster;
			ptr->rootDirCluster = ebr.rootDirClusterNr;
			ptr->dataSectorOffset = ptr->fatOffset + bpm.nrOfFATs * ebr.sectorsPerFAT - 2 * bpm.sectorsPerCluster;

			ptr->fsInfoSect = part.lbaStart + ebr.FSinfoSectNr;
			ptr->fsInfo = (FSInfoStruct *)fsInfoSector;

			partitions->push_back(ptr);

			delete[] bootRecord;
			return true;
		}
	}

	void Initialize()
	{
		partitions = new vector<Partition *>();
	}
	void CleanUp()
	{
		// nothing for now
		for (auto part : *partitions)
		{
			part->CleanUp();
			delete part;
		}
		delete partitions;
	}
	bool intersectPartition(const MBRpartitionEntry &part1, const MBRpartitionEntry &part2)
	{
		if (part1.type == 0 || part2.type == 0)
			return false;
		return part1.lbaStart < part2.lbaStart ? (part1.lbaStart + part1.lbaLen > part2.lbaStart) : (part2.lbaStart + part2.lbaLen > part1.lbaStart);
	}
	static constexpr byte fat32PartType = 0xc; // fat32 with lba
	bool tryLoadPartition(StorageDevice *disk, MBRpartitionEntry &part)
	{
		// limit-check partition size
		if (part.lbaStart + part.lbaLen > disk->getSize())
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
	void detectPartitions(StorageDevice *disk, byte *bootsector)
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