#include "aml.h"
#include "amlcommon.h"
#include "grammar.h"
#include "../../../core/filesystem/filesystem.h" // temporarily
#include <iostream.h>
#include <stringstream.h>

using namespace std;

namespace AML
{
	AMLDataObject AMLDataObject::clone()
	{
		AMLDataObject retVal;
		retVal.type = type;
		switch (type)
		{
			case integerType:
				retVal.valuePtr = new ull(this->val<ull>());
				break;
			case stringType:
				retVal.valuePtr = new string(this->deref<string>());
				break;
			case bufferType:
				retVal.valuePtr = new vector<byte>(this->deref<vector<byte>>());
				break;
			case packageType:
			{
				
				PackageObject* clonedObject = new PackageObject();
				for (auto& elem : this->deref<PackageObject>())
				{
					clonedObject->push_back(elem.clone());
				}
				retVal.valuePtr = clonedObject;
				break;
			}
			case variableType:
				retVal.valuePtr = this->valuePtr;
				break;
			default:
				break;
		}

		return retVal;
	}
	string AMLDataObject::to_string(const string& indentation)
	{
		stringstream stream;
		stream << ostream::base::hex;
		switch (type)
		{
			case voidType:
				stream << "void\n";
				break;
			case integerType:
				stream << "0x" << val<ull>() << '\n';
				break;
			case stringType:
				stream << '"' << deref<string>() << "\"\n";
				break;
			case bufferType:
			{
				vector<byte>& bufferPtr = deref<vector<byte>>();
				stream << "buffer(" << bufferPtr.getSize() << ") {\n";
				for (auto& elem : bufferPtr)
				{
					stream << indentation << "  0x" << elem << '\n';
				}
				stream << indentation << "}\n";
				break;
			}
			case packageType:
			{
				PackageObject& pkgRef = deref<PackageObject>();
				stream << "package(" << pkgRef.getSize() << ") {\n";
				ull len = pkgRef.getSize();
				for (ull i = 0; i < len; i++)
				{
					stream << indentation << "  [" << i << "] " << pkgRef[i].to_string(indentation + "  ");
				}
				stream << indentation << "}\n";
				break;
			}
			case variableType:
				stream << "ref to " << deref<AMLDataObject>().to_string(indentation + "  ");
				break;
			default:
				cout << "unknown\n";
				break;
		}

		return stream.getBuffer();
	}

	bool AssignValToVarUtil(AMLContext &ctx, Operation handler, ull val)
	{
		AMLDataObject* varptr;
		AMLAssertPassthrough(handler(ctx), AssignValToVarUtil);
		AMLAssert(ctx.getReturnByReference(variableType, varptr), unknownError, AssignValToVarUtil);
		varptr->assignAndDeallocate(integerType, new ull(val));

		return true;
	}
	bool AssignToVarUtil(AMLContext &ctx, Operation handler, AMLDataObject obj)
	{
		AMLDataObject* varptr;
		if (handler(ctx) == false)
		{
			obj.deallocate();
			return false;
		}
		if (ctx.returnValueObj.type != variableType)
		{
			obj.deallocate();
			return ctx.logError(unknownError);
		}
		varptr = (AMLDataObject*)ctx.returnValueObj.valuePtr;
		varptr->assignAndDeallocate(obj.type, obj.valuePtr);

		return true;
	}

	bool LoadDefinitionBlock(const byte *definitionBlock, ull definitionBlockLen)
	{
		AMLContext ctx(definitionBlock, definitionBlockLen, false);

		bool result = Grammar::TermList::Parse(ctx);
		if (result == false)
		{
			cout << ctx.lastErrorAsString() << " in table " << (void*)definitionBlock << ctx.stackTrace << "executing:\n";
			DisplayMemoryBlock(ctx.byteStream, 32);
		}

		return result;
	}
	bool ExecuteMethod(const string& name)
	{
		ACPIMethod *method = (ACPIMethod*)ACPI::GetRootNamespace()->get(name, ACPI::ACPINamedObject::methodType);

		if (method == nullptr)
		{
			cout << "Method not found.\n";
			return false;
		}

		AMLContext ctx(method, vector<AMLDataObject>());

		if (ctx.lastError == incorrectArgCountError)
		{
			cout << ctx.lastErrorAsString() << " in method " << (void*)method->methodStart << ".\n";
			return false;
		}

		bool result = Grammar::TermList::Parse(ctx);

		// cout << "Final values:\n" << ostream::base::hex;
		// string indentation;
		// for (ull i = 0; i < ctx.args.getSize(); i++)
		// {
		// 	cout << "Arg" << i << " = ";
		// 	ctx.args[i].display(indentation);
		// }
		// for (ull i = 0; i < ctx.locals.getSize(); i++)
		// {
		// 	cout << "Local" << i << " = ";
		// 	ctx.locals[i].display(indentation);
		// }
		// cout << ostream::base::dec;

		if (result == false && ctx.lastError == returnFromFunctionRetCode)
		{
			result = true;
			ctx.lastError = noError;
			ctx.stackTrace = "\n";
		}
		else
		{
			result = false;
			if (ctx.lastError == noError)
				ctx.lastError = noReturnStatementError;
		}

		if (result == false)
		{
			// there was an error in the function execution
			cout << ctx.lastErrorAsString() << " in method " << (void*)method->methodStart << ctx.stackTrace << "executing:\n";
			DisplayMemoryBlock(ctx.byteStream, 32);
		}
		else
		{
			// no error
			string retVal = ctx.returnValueObj.to_string();
			Filesystem::result res = Filesystem::WriteFile(u"e:/ptos/info/tmp.txt", (byte*)retVal.data(), retVal.length());
			cout << retVal << '\n';
			if (res != Filesystem::result::success)
			{
				cout << "File write failed: " << Filesystem::resultAsString(res) << '\n';
			}
			else
			{
				cout << "File write finished.\n";
			}
		}

		return result;
	}
	bool DisplayDefinitionBlock(const byte *definitionBlock, ull definitionBlockLen)
	{
		AMLContext ctx(definitionBlock, definitionBlockLen, true);

		// cout << (void*)ctx.byteStreamPos << '\n';
		// DisplayMemoryBlock(ctx.byteStreamPos, 0x100);

		ctx.debug_info() << "DefinitionBlock {\n";
		ctx.indent();
		bool result = Grammar::TermList::Parse(ctx);

		if (result == false)
		{
			cout << ctx.lastErrorAsString() << " in table " << (void*)definitionBlock << ctx.stackTrace << "executing:\n";
			DisplayMemoryBlock(ctx.byteStream, 32);
			return false;
		}

		ctx.outdent();
		ctx.debug_info() << "}\n";
		return true;
	}

	bool DevTmp()
	{
		return true;
	}
}