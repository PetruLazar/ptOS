#pragma once
#include <types.h>

namespace AML
{
	bool LoadDefinitionBlock(const byte* definitionBlock, ull definitionBlockLen);
	bool DisplayDefinitionBlock(const byte *definitionBlock, ull definitionBlockLen);

	bool DevTmp();
}