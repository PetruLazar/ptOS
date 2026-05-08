#pragma once
#include "types.h"
#include "../acpi.h"
#include <iostream.h>

#define STRINGIFY_HELPER(arg) #arg
#define STRINGIFY(arg) STRINGIFY_HELPER(arg)

#define AMLAssert(cond, errType, location) if (!(cond))		\
{		\
	ctx.stackTraceEntry(#location, STRINGIFY(__LINE__));		\
	return ctx.logError(errType);		\
}

#define AMLAssertPassthrough(func_call, location) if(!(func_call))		\
{		\
	ctx.stackTraceEntry(#location, STRINGIFY(__LINE__));		\
	return false;		\
}

#define AMLLoopAssertPassthrough(func_call, location) {		\
	if(!(func_call))		\
	{		\
		ErrorType result = ctx.lastError;		\
		if (result == ErrorType::breakLoopRetCode)		\
		{		\
			ctx.stackTrace = "\n";		\
			ctx.lastError = ErrorType::noError;		\
			break;		\
		}		\
		ctx.stackTraceEntry(#location, STRINGIFY(__LINE__));		\
		return false;		\
	}		\
}

#define arraysize(arr) (sizeof(arr) / sizeof(arr[0]))

namespace AML
{
	class ACPIDevice : public ACPI::ACPINamedObject
	{
	public:
		ACPIDevice() : ACPINamedObject(deviceType) { }

		virtual void DisplayContents(std::string& indentation) override
		{
			std::cout << indentation << "Device (" << GetSimpleName() << ") {\n";
			indentation += "  ";
			for (auto elem : children) elem->DisplayContents(indentation);
			indentation.erase(indentation.length() - 2, 2);
			std::cout << indentation << "}\n";
		}
	};
	class ACPIMethod : public ACPI::ACPINamedObject
	{
	private:
		virtual ACPINamedObject* getChild(const std::string& simpleName, ScopeType desiredType = anyType) override { return nullptr; }
		virtual bool addChild(const std::string& simpleName, ACPINamedObject* obj)  override { return false; }

	public:
		union Flags
		{
			byte raw;
			struct Fields
			{
				byte argCount : 3;
				byte serialized : 1;
				byte syncLevel : 4;
			} fields;
		} flags;
		const byte* methodStart;
		ull methodLen;

		ACPIMethod() : ACPINamedObject(methodType) { }

		virtual void DisplayContents(std::string &indentation) override
		{
			std::cout << indentation << "Method (" << GetSimpleName() << ", " << flags.fields.argCount << " args, " << (flags.fields.serialized ? "S" : "nS") << ", Sync " << flags.fields.syncLevel << ")\n";
		}
	};

	class AMLContext
	{
	public:
		const byte *byteStream;
		ull length;

		AMLContext(const byte* byteStream, ull length, bool display = false)
		{
			// derive data from the input data
			this->byteStream = byteStream;
			this->length = length;
			this->runtime = false;

			if (display) output_stream = &std::cout;
			else output_stream = &std::nullout;

			this->currentScope = ACPI::GetRootNamespace();
		}
		AMLContext(ACPIMethod* method, const std::vector<AMLDataObject> &args)
		{
			byteStream = method->methodStart;
			length = method->methodLen;
			runtime = true;

			output_stream = &std::nullout;
			currentScope = method; // maybe get parent instead?

			if (method->flags.fields.argCount != args.getSize())
			{
				lastError = incorrectArgCountError;
				return;
			}

			this->args.assign(args);
			locals.resize(8);
		}

		~AMLContext()
		{
			returnValueObj.assignAndDeallocate(voidType, nullptr);
			for (auto& elem : args)
				elem.deallocate();
			for (auto& elem : locals)
				elem.deallocate();
		}

		// context data
		bool runtime;
		std::vector<AMLDataObject> args;
		std::vector<AMLDataObject> locals;

		ACPI::ACPINamedObject* currentScope;

		// sub-context handling
		std::vector<const byte*> subContextStream;
		std::vector<ull> subContextLength;
		void createSubcontext()
		{
			subContextStream.push_back(byteStream);
			subContextLength.push_back(length);
		}
		void limitSubcontext(ull newLength)
		{
			const byte* parentByteStream = subContextStream[subContextStream.getSize() - 1];
			length = newLength - (byteStream - parentByteStream);
		}
		void popSubContext()
		{
			const byte* newByteStream = byteStream;
			byteStream = subContextStream.pop_back();
			length = subContextLength.pop_back();

			ull diffLength = newByteStream - byteStream;
			byteStream += diffLength;
			length -= diffLength;
		}
		bool revertStream(ull count)
		{
			byteStream -= count;
			length += count;
			return true;
		}
		bool advanceStream(ull count)
		{
			if (length < count)
				return false;

			byteStream += count;
			length -= count;
			return true;
		}

		inline bool check(ull len)
		{
			return length >= len;
		}
		template <typename tn> tn peek()
		{
			return *(tn*)byteStream;
		}
		template <typename tn> tn pop()
		{
			tn val = *(tn*)byteStream;
			byteStream += sizeof(tn);
			length -= sizeof(tn);
			return val;
		}
		template <typename tn> bool assertPop(tn val)
		{
			if (!check(sizeof(tn)))
				return false;

			if (peek<tn>() != val)
				return false;

			pop<tn>();
			return true;
		}

		// error handling
		ErrorType lastError = noError;
		std::string stackTrace = "\n";
		void stackTraceEntry(const char* func, const char* line)
		{
			stackTrace = stackTrace + "  at " + func + ", line " + line + "\n";
		}
		bool logError(ErrorType error)
		{
			lastError = error;
			return false;
		}
		const char* lastErrorAsString()
		{
			static const char* const errorTypeToString[] =
			{
				"No error",
				"Unhandled grammar element",
				"End of stream reached at unexpected time",
				"Unexpected expression type",

				"Invalid character while parsing NameSeg",
				"Inconsistent buffer parameters detected",
				"Too many package initializers",

				"Inexistent namespace referenced",
				"Method called with incorrect number or arguments",
				"LocalObj used in non-runtime context",
				"ArgObj used in non-runtime context",
				"ArgObj out of range",
				"Index out of range error",
				"Function did not execute a return statement",

				"Non-error", // breakLoopError
				"Non-error", // returnFromFunctionError

				"Unkown Error",
			};
			return errorTypeToString[(uint)lastError];
		}

		// expression result
		AMLDataObject returnValueObj;
		template <typename tn> bool returnValue(ExpressionType type, tn* val)
		{
			returnValueObj.assignAndDeallocate(type, val);
			return true;
		}
		template <typename tn> bool getReturnByValue(ExpressionType type, tn* ptr)
		{
			// no point in checking for nullptr;
			// if return value is consumed, return type becomes voidType, which should never be requested
			if (returnValueObj.type != type)
				return false;

			*ptr = *(tn*)returnValueObj.valuePtr;
			return true;
		}
		template <typename tn> bool getReturnByReference(ExpressionType type, tn* &ptr)
		{
			if (returnValueObj.type != type)
				return false;

			ptr = (tn*)returnValueObj.valuePtr;
			return true;
		}
		void takeReturnValueOwnership()
		{
			// skip deletion
			returnValueObj.assign(voidType, nullptr);
		}

		// debug ctx info
		std::ostream* output_stream;
		std::string tabulation = "";
		void indent() { tabulation += "  "; }
		void outdent() { if (tabulation.length() >= 2) tabulation.resize(tabulation.length() - 2); }
		std::ostream& debug_info(bool skipIndentation = false) { return skipIndentation ? *output_stream : *output_stream << tabulation; }
	};
}