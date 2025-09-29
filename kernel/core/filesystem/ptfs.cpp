#include "ptfs.h"
#include "../mem.h"
#include <math.h>

using namespace Disk;
using namespace std;

namespace Filesystem
{
    namespace ptFS
    {
        static constexpr uint ptFSExpectedSignature = 'p' | ('t' << 8) | ('F' << 16) | ('S' << 24); // ASCII for "ptFS"
        static constexpr ushort ptFSExpectedMajorVersion = 0;
        static constexpr ushort ptFSExpectedMinorVersion = 1;

        static constexpr ushort minimumReservedSectors = 8;

        class ptFSHeader
        {
        public:
            char bootcode_jump[4];
            uint signature;      // check for filesystem compatibility
            ushort majorVersion; // check for filesystem compatibility
            ushort minorVersion; // check for filesystem compatibility
            ushort reservedSectors;
            ushort bootrecordBackupSector;
            byte sectorsPerCluster;
            byte reserved1_MBZ[7]; // reserved for future use
            ull partitionOffset;   // start LBA of partition
            ull partitionLength;   // in sectors
            uint clusterMapLength; // in sectors
            byte clusterMapCount;
            byte reserved2_MBZ[3]; // reserved for future use
            ull rootDirCluster;
        };

        enum class EntryType : byte
        {
            null = 0, // the directory has ended

            file = 1,
            directory = 2,

            filenamePart = 14,

            skip = 15, // unused, but followed but other used entries
        };

        class DirectoryEntry
        {
        public:
            byte reserved1_MBS : 4; // MBS does not apply if entry is unused
            byte entryType : 4;

            // attributes
            byte readOnly : 1;
            byte hidden : 1;
            byte system : 1;
            byte reserved2_MBZ : 5;

            // permissions: up to the OS for interpretation, since
            // every OS installation will have different users
            // replace with GUIDs in future versions
            word permissionCategoryId;
            uint ownerUserId;

            ull firstCluster;
        };
        class DirectoryEntryNameSegment
        {
            byte seqNumber : 4;
            byte entryType : 4; // must be "filenamePart"

            // 16 bit chars, but alignment is not preserved
            // the byte string will be constructed by iterating through all entries
            // so alignment will be restored at the end anyway
            byte nameSegment_char16[sizeof(DirectoryEntry) - 1];
        };

        static constexpr int testval = sizeof(DirectoryEntry);

        class ClusterFooter // first cluster footer also has an extension
        {
        public:
            ull consecutiveClusterCount;
            ull nextClusterInChain;
        };
        class ExtendedClusterFooter
        {
        public:
            long long creationTime;
            long long lastModified;
            ull fileSize;
            ushort hardLinkCount;
            byte reserved1_MBZ[6];
        };

        class Partition : public Filesystem::Partition
        {
        public:
            virtual const char *type() override { return "ptFS"; }

            virtual result CreateFile(std::string16 &path, byte *contents, ull length) override
            {
                return result::invalidPartition;
            }
            virtual result RemoveFile(std::string16 &path) override
            {
                return result::invalidPartition;
            }

            virtual result ReadFile(std::string16 &path, byte *&contents, ull &length) override
            {
                return result::invalidPartition;
            }
            virtual result WriteFile(std::string16 &path, byte *contents, ull length) override
            {
                return result::invalidPartition;
            }

            virtual DirectoryIterator *GetDirectoryIterator(std::string16 &path) override
            {
                return nullptr;
            }
            virtual result RemoveDirectory(std::string16 &path) override
            {
                return result::invalidPartition;
            }
            virtual result CreateDirectory(std::string16 &path) override
            {
                return result::invalidPartition;
            }
        };

        bool tryLoadPartition(Disk::Partition *&part)
        {
            // load boot record
            byte *bootRecord = new byte[512];
            ptFSHeader *header = (ptFSHeader *)bootRecord;

            // validate boot record read status and fields
            if ((Disk::result)read(part->disk, part->lbaStart, 1, bootRecord) != Disk::result::success ||
                header->signature != ptFSExpectedSignature ||
                header->majorVersion != ptFSExpectedMajorVersion ||
                header->minorVersion != ptFSExpectedMinorVersion ||
                ((word *)bootRecord)[255] != 0xaa55)
            {
                delete[] bootRecord;
                return false;
            }

            ptFS::Partition *ptr = new ptFS::Partition();
            ptr->disk = part->disk;
            ptr->letter = part->letter;
            ptr->lbaStart = part->lbaStart;
            ptr->lbaLen = part->lbaLen;

            registerPartition(ptr);

            delete part;
            part = ptr;

            delete[] bootRecord;
            return true;
        }
        bool formatPartitionNaty(Disk::Partition *&part)
        {
            byte *bootRecord = new byte[512];
            ptFSHeader *header = (ptFSHeader *)bootRecord;

            memset(bootRecord, 512, 0);

            header->signature = ptFSExpectedSignature;
            header->majorVersion = ptFSExpectedMajorVersion;
            header->minorVersion = ptFSExpectedMinorVersion;

            header->bootcode_jump[0] = 0;
            header->bootcode_jump[1] = 0;
            header->bootcode_jump[2] = 0;
            header->bootcode_jump[3] = 0;

            header->sectorsPerCluster = 2; // should be a parameter
            header->partitionOffset = part->lbaStart;
            header->partitionLength = part->lbaLen;
            header->clusterMapCount = 2;
            header->rootDirCluster = 0;

            // check if partition meets the minimum size requirements:
            // 8 reserved sectors, 1 sectors-long cluster bitmaps, and 2 clusters
            // enough for storing a single small file in the root directory of the filesystem
            if (minimumReservedSectors + 1 * header->clusterMapCount + 2 * header->sectorsPerCluster > part->lbaLen)
            {
                // partition too small
                delete[] bootRecord;
                return false;
            }

            ((word *)bootRecord)[255] = 0xaa55; // boot signature

            ull cs = 4; // cluster size of 4
            ull mc = 2;
            // cout << "cluster size: "; cin >> (uint&)cs;
            // cout << "checking formula for cluster size " << cs << "...\n";
            ull lines = 20;
            for (cs = 1; cs < 10; cs++)
                for (mc = 1; mc < 6; mc++)
                {
                    if (lines > 0)
                        cout << "Cluster size " << cs << " and map count " << mc << ":\n";
                    for (ull i = 8 + 1 * mc + 2 * cs; i < 1000000; i++) // minimum is 2 clusters + 1 sector map + 8 reserved = 17, if cluster size is 4
                    {
                        // calculations
                        ull r; // number of reserved sectors
                        ull m; // number of bitmap sectors
                        ull c; // number of clusters
                        
                        r = 8;
                        m = integerCeilDivide((i - cs) / cs, 4096);
                        c = i / cs - r - m * mc;

                        // checks
                        if (lines == 0)
                        {
                            break;
                        }
                        else if (r + m * mc + c * cs < i)
                        {
                            cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * cs << ") - missing sectors\n";
                            lines--;
                        }
                        else if (r + m * mc + c * cs > i)
                        {
                            cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * cs << ") - too many sectors\n";
                            lines--;
                        }
                        else if (m * 4096 < c)
                        {
                            cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * cs << ") - map too small\n";
                            lines--;
                        }
                        else if ((m - 1) * 4096 >= c)
                        {
                            cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * cs << ") - map too big\n";
                            lines--;
                        }
                        else if (m < 1)
                        {
                            cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * cs << ") - no map\n";
                            lines--;
                        }
                        else if (r < 8)
                        {
                            cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * cs << ") - too few reserved\n";
                            lines--;
                        }
                        else if (r - 8 >= cs + mc || (r - 8 >= cs && m * 4096 != c))
                        {
                            cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * cs << ") - too many reserved\n";
                            lines--;
                        }
                        else if (c < 2)
                        {
                            cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * cs << ") - too few clusters\n";
                            lines--;
                        }
                        else
                        {
                            // cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * cs << ") - no issue found\n";
                        }
                        if (lines == 1)
                        {
                            cout << "...\n";
                            --lines;
                            break;
                        }
                    }
                }
            delete[] bootRecord;
            return false;
        }
        bool formatPartition(Disk::Partition *&part)
        {
            byte *bootRecord = new byte[512];
            ptFSHeader *header = (ptFSHeader *)bootRecord;

            memset(bootRecord, 512, 0);

            header->signature = ptFSExpectedSignature;
            header->majorVersion = ptFSExpectedMajorVersion;
            header->minorVersion = ptFSExpectedMinorVersion;

            header->bootcode_jump[0] = 0;
            header->bootcode_jump[1] = 0;
            header->bootcode_jump[2] = 0;
            header->bootcode_jump[3] = 0;

            header->sectorsPerCluster = 2; // should be a parameter
            header->partitionOffset = part->lbaStart;
            header->partitionLength = part->lbaLen;
            header->clusterMapCount = 2;
            header->rootDirCluster = 0;

            // check if partition meets the minimum size requirements:
            // 8 reserved sectors, 1 sectors-long cluster bitmaps, and 2 clusters
            // enough for storing a single small file in the root directory of the filesystem
            if (minimumReservedSectors + 1 * header->clusterMapCount + 2 * header->sectorsPerCluster > part->lbaLen)
            {
                // partition too small
                delete[] bootRecord;
                return false;
            }

            // one sector of cluster map is 512 bytes = 4096 bits
            // therefore it can cover 4096 clusters

            // leave the minimum reserved sectors out of the calculation
            ull usablePartitionLength = header->partitionLength - minimumReservedSectors;

            // compute how many sectors of cluster bitmap are needed
            ull calculationUnit = 4096 * header->sectorsPerCluster + header->clusterMapCount;
            ull clusterMapCoverage = usablePartitionLength - (header->sectorsPerCluster + header->clusterMapCount - 1);
            ull clusterMapSectors = integerCeilDivide(clusterMapCoverage, calculationUnit);

            // compute how many sectors can fit in the partition and be covered by the map
            ull clusterCount = (usablePartitionLength - clusterMapSectors * header->clusterMapCount) / header->sectorsPerCluster;
            if (clusterCount > clusterMapSectors * 4096)
            {
                clusterCount = clusterMapSectors * 4096;
            }

            // the leftover is reseved
            header->reservedSectors = header->partitionLength - clusterMapSectors * header->clusterMapCount - clusterCount * header->sectorsPerCluster;
            header->bootrecordBackupSector = header->reservedSectors - 1; // last of the reserved sectors
            header->clusterMapLength = clusterMapSectors;

            ((word *)bootRecord)[255] = 0xaa55; // boot signature

            // ull s = 4; // cluster size of 2
            // ull mc = 2;
            // // cout << "cluster size: "; cin >> (uint&)s;
            // // cout << "checking formula for cluster size " << s << "...\n";
            // ull lines = 20;
            // for (s = 1; s < 10; s++)
            //     for (mc = 1; mc < 6; mc++)
            //     {
            //         if (lines > 0)
            //             cout << "Cluster size " << s << " and map count " << mc << ":\n";
            //         for (ull i = 8 + 1 * mc + 2 * s; i < 1000000; i++) // minimum is 2 clusters + 1 sector map + 8 reserved = 17, if cluster size is 4
            //         {
            //             // calculations
            //             ull r = 8, // 8 reserved sectors
            //                 m = integerCeilDivide(i - r - (s + mc - 1), 4096 * s + mc),
            //                 c = (i - r - m * mc) / s;
            //             if (c > m * 4096)
            //                 c = m * 4096;
            //             r = i - m * mc - c * s;
            //             // checks
            //             if (lines == 0)
            //             {
            //                 break;
            //             }
            //             else if (r + m * mc + c * s < i)
            //             {
            //                 cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * s << ") - missing sectors\n";
            //                 lines--;
            //             }
            //             else if (r + m * mc + c * s > i)
            //             {
            //                 cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * s << ") - too many sectors\n";
            //                 lines--;
            //             }
            //             else if (m * 4096 < c)
            //             {
            //                 cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * s << ") - map too small\n";
            //                 lines--;
            //             }
            //             else if ((m - 1) * 4096 >= c)
            //             {
            //                 cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * s << ") - map too big\n";
            //                 lines--;
            //             }
            //             else if (m < 1)
            //             {
            //                 cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * s << ") - no map\n";
            //                 lines--;
            //             }
            //             else if (r < 8)
            //             {
            //                 cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * s << ") - too few reserved\n";
            //                 lines--;
            //             }
            //             else if (r - 8 >= s + mc || (r - 8 >= s && m * 4096 != c))
            //             {
            //                 cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * s << ") - too many reserved\n";
            //                 lines--;
            //             }
            //             else if (c < 2)
            //             {
            //                 cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * s << ") - too few clusters\n";
            //                 lines--;
            //             }
            //             else
            //             {
            //                 // cout << i << ": " << r << "r + " << m << "m * " << mc << " + " << c << "c (" << c * s << ") - no issue found\n";
            //             }
            //             if (lines == 1)
            //             {
            //                 cout << "...\n";
            //                 --lines;
            //                 break;
            //             }
            //         }
            //     }
            // delete[] bootRecord;
            // return false;

            // write boot record and boot record backup
            write(part->disk, header->partitionOffset, 1, bootRecord);
            write(part->disk, header->partitionOffset + header->bootrecordBackupSector, 1, bootRecord);

            // write cluster map/maps
            byte *buffer = new byte[512];
            for (byte map = 0; map < header->clusterMapCount; map++)
            {
                memset(buffer, 512, 0);
                ull mapBase = header->partitionOffset + header->reservedSectors + map * header->clusterMapLength;
                for (uint mapSector = 0; mapSector < header->clusterMapLength; mapSector++)
                {
                    if ((mapSector + 1) * 4096 <= clusterCount)
                    {
                        // all bits in sector cover a cluster
                        // all bits should be 0 (already are)

                        // just write the buffer at the end of the cycle
                        // the buffer has all 0s, since it is initialized that way,
                        // and is only modified at the last map sector,
                        // so this if won't pass again
                    }
                    else if (mapSector * 4096 < clusterCount)
                    {
                        // only the first few bits in sector cover clusters
                        // compute how many used bits are needed
                        ull usedBits = clusterCount % 4096;

                        // calculate byte and bit index of the first unused bit
                        uint byteIndex = usedBits / 8,
                             bitIndex = usedBits % 8;

                        // fill bits after last used bit with 1s
                        buffer[byteIndex++] = ((1 << bitIndex) - 1) ^ 0xff;

                        // fill in the rest of the sector
                        memset(buffer + byteIndex, 512 - byteIndex, 0xff);

                        // continue to write the buffer
                    }
                    else
                    {
                        // no bits are covering clusters, should never happen
                        delete[] bootRecord;
                        delete[] buffer;
                        return false;
                    }

                    write(part->disk, mapBase + mapSector, 1, buffer);
                }
            }

            // create the root directory
            // compute which sector and which bit in the sector correspond to the root dir cluster
            ull mapSector = header->rootDirCluster / 4096;
            uint bitInSector = header->rootDirCluster % 4096;
            // simplify sector-relative bit index to byte and bit index
            uint byteIndex = bitInSector / 8;
            byte bitIndex = bitInSector % 8;
            for (byte map = 0; map < header->clusterMapCount; map++)
            {
                ull mapBase = header->partitionOffset + header->reservedSectors + map * header->clusterMapLength;
                read(part->disk, mapBase + mapSector, 1, buffer);
                buffer[byteIndex] |= 1 << bitIndex; // set the bit to mark rootDir cluster as allocated
                write(part->disk, mapBase + mapSector, 1, buffer);
            }

            // write the actual directory data
            ull clusterOffset = header->partitionOffset + header->reservedSectors + header->clusterMapCount * header->clusterMapLength;
            ull clusterSize = 512 * header->sectorsPerCluster;
            delete[] buffer;
            buffer = new byte[clusterSize];

            // fill directory data
            memset(buffer, clusterSize, 0);
            DirectoryEntry *entry = (DirectoryEntry *)buffer;
            ClusterFooter *clusterFooter = (ClusterFooter *)(buffer + clusterSize - sizeof(ClusterFooter));
            ExtendedClusterFooter *extendedClusterFooter = (ExtendedClusterFooter *)(buffer + clusterSize - sizeof(ClusterFooter) - sizeof(ExtendedClusterFooter));

            entry[0].entryType = (byte)EntryType::null; // should be already 0, but just to enforce it
            clusterFooter->consecutiveClusterCount = 0; // no more clusters
            extendedClusterFooter->creationTime = 0; // time unknown, OS has no reliable way to determine the time
            extendedClusterFooter->lastModified = 0; // time unknown, OS has no reliable way to determine the time
            extendedClusterFooter->fileSize = sizeof(DirectoryEntry); // directory is empty
            extendedClusterFooter->hardLinkCount = 1;

            // write the directory data
            write(part->disk, clusterOffset + header->rootDirCluster * clusterSize, header->sectorsPerCluster, buffer);

            delete[] bootRecord;
            delete[] buffer;
            return true;
        }
    }
}