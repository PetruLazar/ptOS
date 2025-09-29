#pragma once

#include "filesystem.h"

namespace Filesystem
{
    namespace ptFS
    {
        bool tryLoadPartition(Disk::Partition *&part);
        bool formatPartitionNaty(Disk::Partition *&part);
        bool formatPartition(Disk::Partition *&part);
    }
}