#pragma once
#include <types.h>
#include <string.h>

namespace AML
{
	enum ExpressionType
	{
		voidType,
		integerType,
		stringType,
		bufferType,
		packageType,

		variableType,
	};
	enum ErrorType
	{
		// general errors
		noError,
		unhandledElementError,
		endOfStreamError,
		unexpectedExpressionTypeError,

		// format errors
		invalidCharacterInNameSegError,
		inconsistentBufferParametersError,
		tooManyPackageInitializersError,

		// runtime errors
		inexistentNamespaceError,
		incorrectArgCountError,
		nonRuntimeLocalUsageError,
		nonRuntimeArgUsageError,
		argOutOfRangeError,
		indexOutOfRangeError,
		noReturnStatementError,

		// non-errors
		breakLoopRetCode,
		returnFromFunctionRetCode,

		unknownError
	};

	class AMLDataObject
	{
	public:
		ExpressionType type = voidType;
		void* valuePtr = nullptr;

		inline AMLDataObject() { assign(voidType, nullptr); }
		inline AMLDataObject(ExpressionType type, void* valuePtr) { assign(type, valuePtr); }

		inline void assign(ExpressionType type, void* valuePtr)
		{
			this->type = type;
			this->valuePtr = valuePtr;
		}
		inline void assignAndDeallocate(ExpressionType type, void* valuePtr)
		{
			if (this->valuePtr != nullptr)
				deallocate();

			assign(type, valuePtr);
		}
		inline void deallocate();
		AMLDataObject clone();
		std::string to_string(const std::string& indentation = "");

		template <typename T> T val() { return *(T*)valuePtr; }
		template <typename T> T& deref() { return *(T*)valuePtr; }
	};
	class PackageObject : public std::vector<AMLDataObject>
	{
	public:
		inline ~PackageObject()
		{
			for (auto& elem : *this)
				elem.deallocate();
		}
	};

	inline void AMLDataObject::deallocate()
	{
		switch (type)
		{
		case integerType:
			delete (ull*)valuePtr;
			break;
		case stringType:
			delete (std::string*)valuePtr;
			break;
		case bufferType:
			delete (std::vector<byte>*)valuePtr;
			break;
		case packageType:
			delete (PackageObject*)valuePtr;
			break;
		case variableType: // fall through to not deleting
		default:
			// cannot delete untyped - leave the object to leak
			break;
		}

		assign(voidType, nullptr);
	}

	bool LoadDefinitionBlock(const byte* definitionBlock, ull definitionBlockLen);
	bool ExecuteMethod(const std::string& name);
	bool DisplayDefinitionBlock(const byte *definitionBlock, ull definitionBlockLen);

	bool DevTmp();
}