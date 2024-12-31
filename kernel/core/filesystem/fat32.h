#pragma once

#include "filesystem.h"

namespace Filesystem
{
	namespace FAT32
	{
		bool tryLoadPartition(Disk::Partition *&part);
	}
}