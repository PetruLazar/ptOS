#pragma once
#include <types.h>
#include <string.h>

namespace AML
{
	bool LoadDefinitionBlock(const byte* definitionBlock, ull definitionBlockLen);
	bool ExecuteMethod(const std::string& name);
	bool DisplayDefinitionBlock(const byte *definitionBlock, ull definitionBlockLen);

	bool DevTmp();
}