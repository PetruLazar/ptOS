#include "aml.h"

#include "../../core/filesystem/filesystem.h" // temporarily
#include <iostream.h>
#include "../../core/mem.h"

#define AMLAssert(cond, errType) if (!(cond)) { return ctx.logError(errType); }
#define AMLAssertPassthrough(cond) if(!(cond)) { return false; }
#define arraysize(arr) (sizeof(arr) / sizeof(arr[0]))

using namespace std;

namespace AML
{
	enum ExpressionType
	{
		voidType,
		integerType,
		stringType,
		bufferType,
		packageType,
	};
	enum ErrorType
	{
		noError,
		unhandledElementError,
		endOfStreamError,

		invalidCharacterInNameSegError,
		unexpectedExpressionTypeError,
		inconsistentBufferParametersError,
		inconsistentPackageParametersError,

		unknownError
	};

	class AMLContext
	{
	public:
		const byte *byteStream;
		ull length;

		AMLContext(const byte* byteStream, ull length, bool runtime, bool display = false)
		{
			this->byteStream = byteStream;
			this->length = length;

			this->lastError = ErrorType::noError;

			this->expressionReturnValue = nullptr;
			this->expressionReturnType = voidType;

			this->runtime = runtime;
			if (display) output_stream = &cout;
			else output_stream = &nullout;
		}
		~AMLContext()
		{
			consumeReturnByReference();
		}

		// context data
		bool runtime;

		// sub-context handling
		vector<const byte*> subContextStream;
		vector<ull> subContextLength;
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
		ErrorType lastError;
		bool logError(ErrorType error)
		{
			if (expressionReturnValue != nullptr)
				consumeReturnByReference();
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

				"Invalid character while parsing NameSeg",
				"Unexpected expression type",
				"Inconsistent buffer parameters detected",
				"Inconsistent package parameters detected",

				"Unkown Error",
			};
			return errorTypeToString[(uint)lastError];
		}

		// expression result
		ExpressionType expressionReturnType;
		void* expressionReturnValue;
		template <typename tn> bool returnValue(ExpressionType type, tn* val)
		{
			if (expressionReturnValue != nullptr)
				consumeReturnByReference();
			expressionReturnType = type;
			expressionReturnValue = val;
			return true;
		}
		template <typename tn> bool consumeReturnByValue(ExpressionType type, tn* ptr)
		{
			// no point in checking for nullptr;
			// if return value is consumed, return type becomes voidType, which should never be requested
			if (expressionReturnType != type)
				return false;

			*ptr = *(tn*)expressionReturnValue;

			consumeReturnByReference();
			return true;
		}
		template <typename tn> bool getReturnByReference(ExpressionType type, tn* &ptr)
		{
			if (expressionReturnType != type)
				return false;

			ptr = (tn*)expressionReturnValue;
			return true;
		}
		void takeReturnReferenceOwnership()
		{
			// skip deletion
			expressionReturnValue = nullptr;
			expressionReturnType = voidType;
		}
		void consumeReturnByReference();

		// debug ctx info
		ostream* output_stream;
		string tabulation = "";
		void indent() { tabulation += "  "; }
		void outdent() { if (tabulation.length() >= 2) tabulation.resize(tabulation.length() - 2); }
		ostream& debug_info(bool skipIndentation = false) { return skipIndentation ? *output_stream : *output_stream << tabulation; }
	};
	typedef bool (*Operation)(AMLContext &ctx);

	class OpCodeHandler
	{
	public:
		ull opCode;
		Operation handler;
	};

	class PackageElement
	{
	public:
		PackageElement() { assign(voidType, nullptr); }
		PackageElement(AMLContext &ctx) { assign(ctx); }
		PackageElement(ExpressionType type, void* valuePtr) { assign(type, valuePtr); }
		void assign(AMLContext &ctx) { assign(ctx.expressionReturnType, ctx.expressionReturnValue); }
		void assign(ExpressionType type, void* valuePtr)
		{
			if (this->valuePtr != nullptr)
				deallocate();

			this->type = type;
			this->valuePtr = valuePtr;
		}
		void deallocate()
		{
			switch (type)
			{
			case integerType:
				delete (ull*)valuePtr;
				break;
			case stringType:
				delete (string*)valuePtr;
				break;
			default:
				// skip deletion
				break;
			}
			type = voidType;
			valuePtr = nullptr;
		}

		template <typename T> T val() { return *(T*)valuePtr; }
		template <typename T> T& deref() { return *(T*)valuePtr; }

		ExpressionType type = voidType;
		void* valuePtr = nullptr;
	};
	class PackageObject : public vector<PackageElement>
	{
	public:
		~PackageObject()
		{
			for (auto& elem : *this)
				elem.deallocate();
		}
	};

	void AMLContext::consumeReturnByReference()
	{
		switch (expressionReturnType)
		{
		case integerType:
			delete (ull*)expressionReturnValue;
			break;
		case stringType:
			delete (string*)expressionReturnValue;
			break;
		case bufferType:
			delete (vector<byte>*)expressionReturnValue;
			break;
		case packageType:
			delete (PackageObject*)expressionReturnValue;
			break;
		default:
			// cannot delete untyped - leave the object to leak
			break;
		}
		expressionReturnValue = nullptr;
		expressionReturnType = voidType;
	}

	namespace Grammar
	{
		namespace TermList { bool Parse(AMLContext &ctx); }
		namespace TermObj { bool Parse(AMLContext &ctx); }
		namespace TermArg { bool Parse(AMLContext &ctx); }
		namespace DefAlias { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x06; }
		namespace DefName { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x08; }
		namespace DefScope { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x10; }
		namespace DefOpRegion { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x805b; } // extended opcodes have the bytes reversed because a single LE read is used
		namespace PkgLength { bool Parse(AMLContext &ctx); }
		namespace NameString { bool Parse(AMLContext &ctx); }
		namespace NameSeg { bool Parse(AMLContext &ctx); }
		namespace DefCreateBitField { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x8d; }
		namespace DefCreateByteField { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x8c; }
		namespace DefCreateDWordField { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x8a; }
		namespace DefCreateQWordField { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x8f; }
		namespace DefCreateWordField { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x8b; }
		namespace DefBreak { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0xa5; }
		namespace DefBreakPoint { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0xcc; }
		namespace DefContinue { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x9f; }
		namespace DefFatal { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x325b; }
		namespace DefIfElse { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0xa0; }
		namespace DefNoop { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0xa3; }
		namespace DefNotify { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x86; }
		namespace DefRelease { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x275b; }
		namespace DefReset { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x265b; }
		namespace DefReturn { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0xa4; }
		namespace DefSignal { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x245b; }
		namespace DefSleep { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x225b; }
		namespace DefStall { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x215b; }
		namespace DefWhile { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0xa2; }
		namespace DefBuffer { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x11; }
		namespace DefPackage { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x12; }
		namespace DefVarPackage { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x13; }
		namespace DefStore { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x70; }
		namespace DefRefOf { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x71; }
		namespace DefAdd { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x72; }
		namespace DefConcat { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x73; }
		namespace DefSubtract { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x74; }
		namespace DefIncrement { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x75; }
		namespace DefDecrement { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x76; }
		namespace DefMultiply { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x77; }
		namespace DefDivide { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x78; }
		namespace DefShiftLeft { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x79; }
		namespace DefShiftRight { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x7A; }
		namespace DefAnd { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x7B; }
		namespace DefNand { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x7C; }
		namespace DefOr { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x7D; }
		namespace DefNor { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x7E; }
		namespace DefXor { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x7F; }
		namespace DefNot { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x80; }
		namespace DefFindSetLeftBit { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x81; }
		namespace DefFindSetRightBit { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x82; }
		namespace DefDerefOf { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x83; }
		namespace DefConcatRes { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x84; }
		namespace DefMod { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x85; }
		namespace DefSizeOf { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x87; }
		namespace DefIndex { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x88; }
		namespace DefMatch { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x89; }
		namespace DefObjectType { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x8E; }
		namespace DefLand { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x90; }
		namespace DefLor { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x91; }
		namespace DefLnot { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x92; }
		namespace DefLequal { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x93; }
		namespace DefLgreater { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x94; }
		namespace DefLless { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x95; }
		namespace DefToBuffer { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x96; }
		namespace DefToDecimalString { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x97; }
		namespace DefToHexString { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x98; }
		namespace DefToInteger { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x99; }
		namespace DefToString { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x9C; }
		namespace DefCopyObject { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x9D; }
		namespace DefMid { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x9E; }
		namespace DefCondRefOf { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x125b; }
		namespace DefLoadTable { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x1F5b; }
		namespace DefAcquire { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x235b; }
		namespace DefWait { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x255b; }
		namespace DefFromBCD { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x285b; }
		namespace DefToBCD { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x295b; }
		namespace DefTimer { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x335b; }
		namespace StringObj { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x0d; }
		namespace RevisionOp { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x305b; }
		namespace ExpressionOpcode { bool Parse(AMLContext &ctx); }
		namespace NameSeg { bool Parse(AMLContext &ctx); }
		namespace NameString { bool Parse(AMLContext &ctx); }
		namespace DefBankField { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x875b; }
		namespace DefDataRegion { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x885b; }
		namespace DefExternal { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x15; }
		namespace DefPowerRes { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x845b; }
		namespace DefThermalZone { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x855b; }
		namespace DefMethod { bool Parse(AMLContext &ctx); static constexpr byte opCode = 0x14; }
		namespace DefField { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x815b; }
		namespace DefDevice { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x825b; }
		namespace DefProcessor { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x835b; }
		namespace DefCreateField { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x135b; }
		namespace FieldList { bool Parse(AMLContext &ctx); }
		namespace FieldElement { bool Parse(AMLContext &ctx); }
		namespace NamedField { bool Parse(AMLContext &ctx); }
		namespace ReservedField { bool Parse(AMLContext &ctx); }
		namespace AccessField { bool Parse(AMLContext &ctx); }
		namespace ExtendedAccessField { bool Parse(AMLContext &ctx); }
		namespace ConnectField { bool Parse(AMLContext &ctx); }
		namespace DataRefObject { bool Parse(AMLContext &ctx); }
		namespace PackageElementList { bool Parse(AMLContext &ctx); }
		namespace PackageElement { bool Parse(AMLContext &ctx); }
		namespace DefMutex { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x015b; }

		namespace ImmediateConst
		{
			static constexpr byte bytePrefix = 0x0a;
			static constexpr byte wordPrefix = 0x0b;
			static constexpr byte dwordPrefix = 0x0c;
			static constexpr byte qwordPrefix = 0x0e;
			bool Parse(AMLContext &ctx);
		}
		namespace ConstObj
		{
			static constexpr byte zeroOp = 0x00;
			static constexpr byte oneOp = 0x01;
			static constexpr byte onesOp = 0xff;
			bool Parse(AMLContext &ctx);
		}
		namespace DataObject
		{
			bool OpCodeMatches(byte opCode);
			bool OpCodeMatches(word extOpCode);
			bool Parse(AMLContext &ctx);
		}
		namespace LocalObj
		{
			static constexpr byte opCodeBase = 0x60;
			static constexpr byte opCodeCount = 8;
			inline bool OpcodeMatches(byte opcode) { return opcode >= opCodeBase && opcode < opCodeBase + opCodeCount; }
			bool Parse(AMLContext &ctx);
		}
		namespace ArgObj
		{
			static constexpr byte opCodeBase = 0x68;
			static constexpr byte opCodeCount = 7;
			inline bool OpcodeMatches(byte opcode) { return opcode >= opCodeBase && opcode < opCodeBase + opCodeCount; }
			bool Parse(AMLContext &ctx);
		}
	}

	namespace Grammar
	{
		namespace TermList
		{
			bool Parse(AMLContext &ctx)
			{
				while (ctx.length > 0)
				{
					AMLAssertPassthrough(TermObj::Parse(ctx));
				}

				return true;
			}
		}
		namespace TermObj
		{
			static constexpr OpCodeHandler possibleOpCodes[] =
			{
				// NameSpaceModifierObj
				{ DefAlias::opCode, DefAlias::Parse },
				{ DefName::opCode, DefName::Parse },
				{ DefScope::opCode, DefScope::Parse },

				// NamedObj
				{ DefCreateBitField::opCode, DefCreateBitField::Parse },
				{ DefCreateByteField::opCode, DefCreateByteField::Parse },
				{ DefCreateDWordField::opCode, DefCreateDWordField::Parse },
				{ DefCreateQWordField::opCode, DefCreateQWordField::Parse },
				{ DefCreateWordField::opCode, DefCreateWordField::Parse },
				{ DefExternal::opCode, DefExternal::Parse },
				{ DefMethod::opCode, DefMethod::Parse },

				// StatementOpcode
				{ DefBreak::opCode, DefBreak::Parse },
				{ DefBreakPoint::opCode, DefBreakPoint::Parse },
				{ DefContinue::opCode, DefContinue::Parse },
				{ DefIfElse::opCode, DefIfElse::Parse },
				{ DefNoop::opCode, DefNoop::Parse },
				{ DefNotify::opCode, DefNotify::Parse },
				{ DefReturn::opCode, DefReturn::Parse },
				{ DefWhile::opCode, DefWhile::Parse },
			};
			static constexpr OpCodeHandler possibleExtOpCodes[] =
			{
				// NamedObj
				{ DefOpRegion::extOpCode, DefOpRegion::Parse },
				{ DefCreateField::extOpCode, DefCreateField::Parse },
				{ DefPowerRes::extOpCode, DefPowerRes::Parse },
				{ DefThermalZone::extOpCode, DefThermalZone::Parse },
				{ DefField::extOpCode, DefField::Parse },
				{ DefDevice::extOpCode, DefDevice::Parse },
				{ DefProcessor::extOpCode, DefProcessor::Parse },
				{ DefBankField::extOpCode, DefBankField::Parse },
				{ DefDataRegion::extOpCode, DefDataRegion::Parse },
				{ DefMutex::extOpCode, DefMutex::Parse },

				// StatementOpcode
				{ DefFatal::extOpCode, DefFatal::Parse },
				{ DefRelease::extOpCode, DefRelease::Parse },
				{ DefReset::extOpCode, DefReset::Parse },
				{ DefSignal::extOpCode, DefSignal::Parse },
				{ DefSleep::extOpCode, DefSleep::Parse },
				{ DefStall::extOpCode, DefStall::Parse },
			};

			bool Parse(AMLContext &ctx)
			{
				// try basic opcodes
				if (ctx.check(1))
				{
					byte opCode = ctx.peek<byte>();
					for (auto& opcodeHandler : possibleOpCodes)
					{
						if (opcodeHandler.opCode == opCode) return opcodeHandler.handler(ctx);
					}
				}

				// try extended op codes
				if (ctx.check(2))
				{
					word extOpCode = ctx.peek<word>();
					for (auto& OpCodeHandler : possibleExtOpCodes)
					{
						if (OpCodeHandler.opCode == extOpCode) return OpCodeHandler.handler(ctx);
					}
				}

				// no opCode match => ExpressionOpcode
				return ExpressionOpcode::Parse(ctx);
			}
		}
		namespace TermArg
		{
			bool Parse(AMLContext &ctx)
			{
				// check if LocalObj
				if (ctx.check(1) && LocalObj::OpcodeMatches(ctx.peek<byte>()))
					return LocalObj::Parse(ctx);

				// check if ArgObj
				if (ctx.check(1) && ArgObj::OpcodeMatches(ctx.peek<byte>()))
					return ArgObj::Parse(ctx);

				// check if DataObject
				if ((ctx.check(1) && DataObject::OpCodeMatches(ctx.peek<byte>())) ||
					(ctx.check(2) && DataObject::OpCodeMatches(ctx.peek<word>())))
					return DataObject::Parse(ctx);

				// fallback to ExpressionOpcode
				return ExpressionOpcode::Parse(ctx);
			}
		}
		namespace DefAlias
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefName
		{
			bool Parse(AMLContext &ctx)
			{
				// NameOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError);

				// NameString
				string *tmpname, name;
				AMLAssertPassthrough(NameString::Parse(ctx));
				AMLAssert(ctx.getReturnByReference(stringType, tmpname), unknownError);
				name = *tmpname;
				ctx.consumeReturnByReference();

				// DataRefObject
				AMLAssertPassthrough(DataRefObject::Parse(ctx));

				switch (ctx.expressionReturnType)
				{
					case integerType:
					{
						ull data;
						AMLAssert(ctx.consumeReturnByValue(integerType, &data), unknownError);
						ctx.debug_info() << "Name (" << name << ", " << data << ")\n";
						break;
					}
					case stringType:
					{
						string* val;
						AMLAssert(ctx.getReturnByReference(stringType, val), unknownError);
						ctx.debug_info() << "Name (" << name << ", \"" << *val << "\")\n";
						ctx.consumeReturnByReference();
						break;
					}
					case bufferType:
					{
						vector<byte> *buffer;
						AMLAssert(ctx.getReturnByReference(bufferType, buffer), unknownError);
						ctx.debug_info() << "Name (" << name << ") Buffer(" << buffer->getSize() << ") {\n";
						ctx.indent();
						for (byte b : *buffer)
						{
							ctx.debug_info() << ostream::base::hex << "0x" << b << ",\n" << ostream::base::dec;
						}
						ctx.outdent();
						ctx.debug_info() << "}\n";
						ctx.consumeReturnByReference();
						break;
					}
					case packageType:
					{
						PackageObject *package;
						AMLAssert(ctx.getReturnByReference(packageType, package), unknownError);
						ctx.debug_info() << "Name (" << name << ") Package(" << package->getSize() << ") {\n";
						ctx.indent();
						for (auto& elem : *package)
						{
							switch (elem.type)
							{
							case integerType:
								ctx.debug_info() << ostream::base::hex << "0x" << elem.val<ull>() << ",\n" << ostream::base::dec;
								break;
							case stringType:
								ctx.debug_info() << '\"' << elem.deref<string>() << "\",\n";
								break;
							default:
								return ctx.logError(unexpectedExpressionTypeError);
							}
						}
						ctx.outdent();
						ctx.debug_info() << "}\n";
						ctx.consumeReturnByReference();
						break;
					}
					default:
						return ctx.logError(unexpectedExpressionTypeError);
				}

				return true;
			}
		}
		namespace DefScope
		{
			bool Parse(AMLContext &ctx)
			{
				ull pkgLength;

				// consume opcode
				AMLAssert(ctx.assertPop<byte>(DefScope::opCode), unknownError);

				// enter sub-context
				ctx.createSubcontext();

				// pkgLength
				AMLAssertPassthrough(PkgLength::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &pkgLength), unknownError);
				ctx.limitSubcontext(pkgLength);

				// NameString
				string* name;
				AMLAssertPassthrough(NameString::Parse(ctx));
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError);

				ctx.debug_info() << "Scope (" << *name << ") {\n";
				ctx.indent();
				ctx.consumeReturnByReference();

				// TermList
				AMLAssertPassthrough(TermList::Parse(ctx));

				ctx.outdent();
				ctx.debug_info() << "}\n";
				ctx.popSubContext();

				return true;
			}
		}
		namespace DefBankField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefCreateBitField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefCreateByteField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefCreateDWordField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefCreateField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefCreateQWordField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefCreateWordField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefDataRegion
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefExternal
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefOpRegion
		{
			bool Parse(AMLContext &ctx)
			{
				string* tmpname;

				string name;
				byte regionSpace;
				ull regionOffset;
				ull regionLen;

				// DefOpRegion := NameString RegionSpace RegionOffset RegionLen
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError);

				// NameString
				AMLAssertPassthrough(NameString::Parse(ctx));
				AMLAssert(ctx.getReturnByReference(stringType, tmpname), unknownError);
				name = *tmpname;
				ctx.consumeReturnByReference();

				// RegionSpace
				AMLAssert(ctx.check(1), endOfStreamError);
				regionSpace = ctx.pop<byte>();

				// RegionOffset => Integer
				AMLAssertPassthrough(TermArg::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &regionOffset), unknownError);

				// RegionLen => Integer
				AMLAssertPassthrough(TermArg::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &regionLen), unexpectedExpressionTypeError);

				ctx.debug_info() << "OpRegion (" << name << ", " << (uint)regionSpace << ", " << regionOffset << ", " << regionLen << ")\n";

				return true;
			}
		}
		namespace DefPowerRes
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefThermalZone
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefMethod
		{
			bool Parse(AMLContext &ctx)
			{
				// MethodOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError);

				ctx.createSubcontext();

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &pkgLength), unknownError);
				ctx.limitSubcontext(pkgLength);

				// NameString
				string *name;
				AMLAssertPassthrough(NameString::Parse(ctx));
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError);

				// MethodFlags
				AMLAssert(ctx.check(1), endOfStreamError);
				byte methodFlags = ctx.pop<byte>();

				ctx.debug_info() << "Method (" << *name << ", " << (uint)methodFlags << ") {\n";
				ctx.consumeReturnByReference();
				ctx.indent();

				// TermList
				// skip for now...
				ctx.debug_info() << ctx.length << " bytes...\n";
				ctx.advanceStream(ctx.length);

				ctx.popSubContext();
				ctx.outdent();
				ctx.debug_info() << "}\n";
				return true;
			}
		}
		namespace DefField
		{
			bool Parse(AMLContext &ctx)
			{
				// FieldOp
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError);

				// enter sub-context
				ctx.createSubcontext();

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &pkgLength), unknownError);
				ctx.limitSubcontext(pkgLength);

				// NameString
				string *name;
				AMLAssertPassthrough(NameString::Parse(ctx));
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError);

				// FieldFlags
				AMLAssert(ctx.check(1), endOfStreamError);
				byte fieldFlags = ctx.pop<byte>();

				ctx.debug_info() << "Field (" << *name << ", " << (uint)fieldFlags << ") {\n";
				ctx.indent();
				ctx.consumeReturnByReference();

				// FieldList
				AMLAssertPassthrough(FieldList::Parse(ctx));

				ctx.outdent();
				ctx.debug_info() << "}\n";
				ctx.popSubContext();

				return true;
			}
		}
		namespace DefDevice
		{
			bool Parse(AMLContext &ctx)
			{
				// DeviceOp
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError);

				ctx.createSubcontext();

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &pkgLength), unknownError);
				ctx.limitSubcontext(pkgLength);

				// NameString
				string *name;
				AMLAssertPassthrough(NameString::Parse(ctx));
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError);

				ctx.debug_info() << "Device (" << *name << ") {\n";
				ctx.consumeReturnByReference();
				ctx.indent();

				// TermList
				AMLAssertPassthrough(TermList::Parse(ctx));

				ctx.outdent();
				ctx.debug_info() << "}\n";
				ctx.popSubContext();

				return true;
			}
		}
		namespace DefProcessor
		{
			bool Parse(AMLContext &ctx)
			{
				// deprecated, parse as little data as possible and skip object
				// ProcessorOp
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError);

				// PkgLength
				ull pkgLength;
				ctx.createSubcontext();
				AMLAssertPassthrough(PkgLength::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &pkgLength), unknownError);
				ctx.limitSubcontext(pkgLength);

				ctx.debug_info() << "Processor {\n";
				ctx.indent();
				ctx.debug_info() << "Skipping " << ctx.length << " bytes...\n";
				ctx.outdent();
				ctx.debug_info() << "}\n";

				// NameString ProcID PblkAddr PblkLen TermList
				// skip all other info
				ctx.advanceStream(ctx.length);

				ctx.popSubContext();
				return true;
			}
		}
		namespace PkgLength
		{
			bool Parse(AMLContext &ctx)
			{
				uint value = 0;
				AMLAssert(ctx.check(1), endOfStreamError);
				byte leadByte = ctx.pop<byte>();
				uint additionalBytes = leadByte >> 6;

				AMLAssert(ctx.check(additionalBytes), endOfStreamError);
				
				if (additionalBytes)
				{
					int bitshift = 4;
					value = leadByte & 0x0f;
					do
					{
						value |= ctx.pop<byte>() << bitshift;
						bitshift += 8;
						additionalBytes--;
					} while (additionalBytes);
				}
				else
				{
					value = leadByte & 0x3f;
				}

				return ctx.returnValue(integerType, new ull(value));
			}
		}
		namespace NameString
		{
			static constexpr byte rootchar = '\\';
			static constexpr byte prefixChar = '^';

			static constexpr byte nullChar = 0x00;
			static constexpr byte dualNamePrefix = 0x2e;
			static constexpr byte multiNamePrefix = 0x2f;

			bool AppendNameSegs(AMLContext &ctx, string& result, uint segCoung)
			{
				string *nameSegVal;

				for (uint i = 0; i < segCoung; i++)
				{
					AMLAssertPassthrough(NameSeg::Parse(ctx));
					AMLAssert(ctx.getReturnByReference(stringType, nameSegVal), unknownError);
					result.append(*nameSegVal);
					ctx.consumeReturnByReference();
				}
				return true;
			}
			bool Parse(AMLContext &ctx)
			{
				string retVal;
				string *nameSegVal;

				if (ctx.assertPop<byte>(rootchar))
				{
					retVal.push_back(rootchar);
				}
				else while (ctx.assertPop<byte>(prefixChar))
				{
					retVal.push_back(prefixChar);
				}

				if (ctx.assertPop<byte>(dualNamePrefix))
				{
					// DualNamePath := DualNamePrefix NameSeg NameSeg
					AMLAssertPassthrough(AppendNameSegs(ctx, retVal, 2));
				}
				else if (ctx.assertPop<byte>(multiNamePrefix))
				{
					// MultiNamePath := MultiNamePrefix SegCount NameSeg(SegCount)
					AMLAssert(ctx.check(1), endOfStreamError);
					byte segCount = ctx.pop<byte>();
					AMLAssertPassthrough(AppendNameSegs(ctx, retVal, segCount));
				}
				else if (ctx.assertPop<byte>(nullChar))
				{
					// NullName
					// nothing else to append
				}
				else
				{
					// NameSeg
					AMLAssertPassthrough(AppendNameSegs(ctx, retVal, 1));
				}

				return ctx.returnValue(stringType, new string(retVal));
			}
		}
		namespace NameSeg
		{
			inline bool IsLeadNameChar(byte val) { return (val >= 'A' && val <= 'Z') || val == '_'; }
			inline bool IsNameChar(byte val) { return IsLeadNameChar(val) || (val >= '0' && val <= '9'); }

			bool Parse(AMLContext &ctx)
			{
				AMLAssert(ctx.check(4), endOfStreamError);

				AMLAssert(IsLeadNameChar(ctx.byteStream[0]), invalidCharacterInNameSegError);
				for (uint i = 1; i < 4; i++)
				{
					AMLAssert(IsNameChar(ctx.byteStream[i]), invalidCharacterInNameSegError);
				}

				string *retVal = new string((char*)ctx.byteStream, 4);
				ctx.advanceStream(4);
				return ctx.returnValue(stringType, retVal);
			}
		}
		namespace LocalObj
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace ArgObj
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace ExpressionOpcode
		{
			static constexpr OpCodeHandler possibleOpCodes[] =
			{
				{ DefBuffer::opCode, DefBuffer::Parse },
				{ DefPackage::opCode, DefPackage::Parse },
				{ DefVarPackage::opCode, DefVarPackage::Parse },
				{ DefStore::opCode, DefStore::Parse },
				{ DefRefOf::opCode, DefRefOf::Parse },
				{ DefAdd::opCode, DefAdd::Parse },
				{ DefConcat::opCode, DefConcat::Parse },
				{ DefSubtract::opCode, DefSubtract::Parse },
				{ DefIncrement::opCode, DefIncrement::Parse },
				{ DefDecrement::opCode, DefDecrement::Parse },
				{ DefMultiply::opCode, DefMultiply::Parse },
				{ DefDivide::opCode, DefDivide::Parse },
				{ DefShiftLeft::opCode, DefShiftLeft::Parse },
				{ DefShiftRight::opCode, DefShiftRight::Parse },
				{ DefAnd::opCode, DefAnd::Parse },
				{ DefNand::opCode, DefNand::Parse },
				{ DefOr::opCode, DefOr::Parse },
				{ DefNor::opCode, DefNor::Parse },
				{ DefXor::opCode, DefXor::Parse },
				{ DefNot::opCode, DefNot::Parse },
				{ DefFindSetLeftBit::opCode, DefFindSetLeftBit::Parse },
				{ DefFindSetRightBit::opCode, DefFindSetRightBit::Parse },
				{ DefDerefOf::opCode, DefDerefOf::Parse },
				{ DefConcatRes::opCode, DefConcatRes::Parse },
				{ DefMod::opCode, DefMod::Parse },
				{ DefSizeOf::opCode, DefSizeOf::Parse },
				{ DefIndex::opCode, DefIndex::Parse },
				{ DefMatch::opCode, DefMatch::Parse },
				{ DefObjectType::opCode, DefObjectType::Parse },
				{ DefLand::opCode, DefLand::Parse },
				{ DefLor::opCode, DefLor::Parse },
				{ DefLnot::opCode, DefLnot::Parse },
				{ DefLequal::opCode, DefLequal::Parse },
				{ DefLgreater::opCode, DefLgreater::Parse },
				{ DefLless::opCode, DefLless::Parse },
				{ DefToBuffer::opCode, DefToBuffer::Parse },
				{ DefToDecimalString::opCode, DefToDecimalString::Parse },
				{ DefToHexString::opCode, DefToHexString::Parse },
				{ DefToInteger::opCode, DefToInteger::Parse },
				{ DefToString::opCode, DefToString::Parse },
				{ DefCopyObject::opCode, DefCopyObject::Parse },
				{ DefMid::opCode, DefMid::Parse },
			};
			static constexpr OpCodeHandler possibleExtOpCodes[] =
			{
				{ DefCondRefOf::extOpCode, DefCondRefOf::Parse },
				{ DefLoadTable::extOpCode, DefLoadTable::Parse },
				{ DefAcquire::extOpCode, DefAcquire::Parse },
				{ DefWait::extOpCode, DefWait::Parse },
				{ DefFromBCD::extOpCode, DefFromBCD::Parse },
				{ DefToBCD::extOpCode, DefToBCD::Parse },
				{ DefTimer::extOpCode, DefTimer::Parse },
			};
			bool Parse(AMLContext &ctx)
			{
				// no opcode match => MethodInvocation
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DataObject
		{
			static constexpr OpCodeHandler possibleOpCodes[] =
			{
				{ DefPackage::opCode, DefPackage::Parse },
				{ DefVarPackage::opCode, DefVarPackage::Parse },
				{ ImmediateConst::bytePrefix, ImmediateConst::Parse },
				{ ImmediateConst::wordPrefix, ImmediateConst::Parse },
				{ ImmediateConst::dwordPrefix, ImmediateConst::Parse },
				{ ImmediateConst::qwordPrefix, ImmediateConst::Parse },
				{ StringObj::opCode, StringObj::Parse },
				{ ConstObj::zeroOp, ConstObj::Parse },
				{ ConstObj::oneOp, ConstObj::Parse },
				{ ConstObj::onesOp, ConstObj::Parse },
				{ DefBuffer::opCode, DefBuffer::Parse },
			};
			static constexpr OpCodeHandler possibleExtOpCodes[] =
			{
				{ RevisionOp::extOpCode, RevisionOp::Parse },
			};

			bool OpCodeMatches(byte opCode)
			{
				for (auto& opcodeHandler : possibleOpCodes)
				{
					if (opcodeHandler.opCode == opCode) return true;
				}
				return false;
			}
			bool OpCodeMatches(word extOpCode)
			{
				for (auto& opcodeHandler : possibleExtOpCodes)
				{
					if (opcodeHandler.opCode == extOpCode) return true;
				}
				return false;
			}
			bool Parse(AMLContext &ctx)
			{
				// try basic opcodes
				if (ctx.check(1))
				{
					byte opCode = ctx.peek<byte>();
					for (auto& opcodeHandler : possibleOpCodes)
					{
						if (opcodeHandler.opCode == opCode) return opcodeHandler.handler(ctx);
					}
				}

				// try extended op codes
				if (ctx.check(2))
				{
					word extOpCode = ctx.peek<word>();
					for (auto& OpCodeHandler : possibleExtOpCodes)
					{
						if (OpCodeHandler.opCode == extOpCode) return OpCodeHandler.handler(ctx);
					}
				}

				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefBreak
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefBreakPoint
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefContinue
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefFatal
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefIfElse
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefNoop
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefNotify
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefRelease
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefReset
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefReturn
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefSignal
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefSleep
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefStall
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefWhile
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefBuffer
		{
			bool Parse(AMLContext &ctx)
			{
				// BufferOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError);

				// PkgLength
				ull pkgLength;
				ctx.createSubcontext();
				AMLAssertPassthrough(PkgLength::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &pkgLength), unknownError);
				ctx.limitSubcontext(pkgLength);

				// BufferSize
				ull bufferSize;
				AMLAssertPassthrough(TermArg::Parse(ctx))
				AMLAssert(ctx.consumeReturnByValue(integerType, &bufferSize), unexpectedExpressionTypeError);

				// ByteList
				AMLAssert(ctx.length == bufferSize, inconsistentBufferParametersError);
				vector<byte> *buffer = new vector<byte>(ctx.byteStream, bufferSize);
				ctx.advanceStream(bufferSize);

				ctx.popSubContext();
				return ctx.returnValue(bufferType, buffer);
			}
		}
		namespace DefPackage
		{
			bool Parse(AMLContext &ctx)
			{
				// PackageOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError);

				// PkgLength
				ull pkgLength;
				ctx.createSubcontext();
				AMLAssertPassthrough(PkgLength::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &pkgLength), unknownError);
				ctx.limitSubcontext(pkgLength);

				// NumElements
				byte numElements;
				AMLAssert(ctx.check(1), endOfStreamError);
				numElements = ctx.pop<byte>();

				// PackageElementList
				AMLAssertPassthrough(PackageElementList::Parse(ctx));
				PackageObject *package;
				AMLAssert(ctx.getReturnByReference(packageType, package), unknownError);
				AMLAssert(package->getSize() == numElements && ctx.length == 0, inconsistentPackageParametersError);
				ctx.popSubContext();
				return true; // return the PackageObject
			}
		}
		namespace DefVarPackage
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefStore
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefRefOf
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefAdd
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefConcat
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefSubtract
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefIncrement
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefDecrement
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefMultiply
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefDivide
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefShiftLeft
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefShiftRight
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefAnd
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefNand
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefOr
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefNor
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefXor
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefNot
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefFindSetLeftBit
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefFindSetRightBit
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefDerefOf
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefConcatRes
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefMod
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefSizeOf
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefIndex
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefMatch
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefObjectType
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefLand
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefLor
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefLnot
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefLequal
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefLgreater
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefLless
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefToBuffer
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefToDecimalString
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefToHexString
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefToInteger
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefToString
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefCopyObject
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefMid
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefCondRefOf
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefLoadTable
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefAcquire
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefWait
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefFromBCD
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefToBCD
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefTimer
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace ImmediateConst
		{
			bool Parse(AMLContext &ctx)
			{
				if (ctx.assertPop<byte>(bytePrefix))
				{
					AMLAssert(ctx.check(1), endOfStreamError);
					return ctx.returnValue(integerType, new ull(ctx.pop<byte>()));
				}
				if (ctx.assertPop<byte>(wordPrefix))
				{
					AMLAssert(ctx.check(2), endOfStreamError);
					return ctx.returnValue(integerType, new ull(ctx.pop<word>()));
				}
				if (ctx.assertPop<byte>(dwordPrefix))
				{
					AMLAssert(ctx.check(4), endOfStreamError);
					return ctx.returnValue(integerType, new ull(ctx.pop<dword>()));
				}
				if (ctx.assertPop<byte>(qwordPrefix))
				{
					AMLAssert(ctx.check(8), endOfStreamError);
					return ctx.returnValue(integerType, new ull(ctx.pop<qword>()));
				}

				return ctx.logError(unknownError);
			}
		}
		namespace StringObj
		{
			inline bool isAsciiChar(byte v) { return v >= 0x01 && v <= 0x7f; }
			bool Parse(AMLContext &ctx)
			{
				// StringPrefix
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError);

				// AsciiCharList
				string *str = new string;
				while (ctx.check(1) && isAsciiChar(ctx.peek<byte>()))
				{
					str->push_back(ctx.pop<char>());
				}

				// NullChar
				ctx.returnValue(stringType, str);
				AMLAssert(ctx.assertPop(NameString::nullChar), endOfStreamError);
				
				return true;
			}
		}
		namespace ConstObj
		{
			bool Parse(AMLContext &ctx)
			{
				byte retVal;
				if (ctx.assertPop<byte>(zeroOp))
				{
					retVal = zeroOp;
				}
				else if (ctx.assertPop<byte>(oneOp))
				{
					retVal = oneOp;
				}
				else if (ctx.assertPop<byte>(onesOp))
				{
					retVal = onesOp;
				}
				else
				{
					return ctx.logError(unknownError);
				}
				return ctx.returnValue(integerType, new ull(retVal));
			}
		}
		namespace RevisionOp
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace FieldList
		{
			bool Parse(AMLContext &ctx)
			{
				while (ctx.length > 0)
				{
					AMLAssertPassthrough(FieldElement::Parse(ctx));
				}

				return true;
			}
		}
		namespace FieldElement
		{
			static constexpr Operation possibleOpCodes[] =
			{
				/* 00 */ ReservedField::Parse,
				/* 01 */ AccessField::Parse,
				/* 02 */ ConnectField::Parse,
				/* 03 */ ExtendedAccessField::Parse,
			};
			bool Parse(AMLContext &ctx)
			{
				if (ctx.check(1))
				{
					byte opCode = ctx.peek<byte>();
					if (opCode < arraysize(possibleOpCodes))
						return possibleOpCodes[opCode](ctx);
				}

				return NamedField::Parse(ctx);
			}
		}
		namespace NamedField
		{
			bool Parse(AMLContext &ctx)
			{
				// NameSeg
				string* name;
				AMLAssertPassthrough(NameSeg::Parse(ctx));
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError);
				ctx.debug_info() << "NamedField (" << *name << ", ";

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &pkgLength), unknownError);
				ctx.debug_info(true) << pkgLength << ")\n";

				return true;
			}
		}
		namespace ReservedField
		{
			bool Parse(AMLContext &ctx)
			{
				// 0x00
				AMLAssert(ctx.assertPop<byte>(0x00), unknownError);

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx));
				AMLAssert(ctx.consumeReturnByValue(integerType, &pkgLength), unknownError);

				ctx.debug_info() << "ReservedField (" << pkgLength << ")\n";

				return true;
			}
		}
		namespace AccessField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace ExtendedAccessField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace ConnectField
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DataRefObject
		{
			bool Parse(AMLContext &ctx)
			{
				// try DataObject
				if ((ctx.check(1) && DataObject::OpCodeMatches(ctx.peek<byte>())) ||
					(ctx.check(2) && DataObject::OpCodeMatches(ctx.peek<word>())))
					return DataObject::Parse(ctx);

				// ObjectReference
				return ctx.logError(unhandledElementError);
			}
		}
		namespace PackageElementList
		{
			bool Parse(AMLContext &ctx)
			{
				PackageObject *package = new PackageObject();

				while (ctx.length > 0)
				{
					if (PackageElement::Parse(ctx) == false)
					{
						delete package;
						return false;
					}
					package->push_back(AML::PackageElement(ctx));
					ctx.takeReturnReferenceOwnership();
				}

				return ctx.returnValue(packageType, package);
			}
		}
		namespace PackageElement
		{
			bool Parse(AMLContext &ctx)
			{
				// PackageElement := DataRefObject | NameString
				// DataRefObject := DataObject | ObjectReference(N/A)

				bool isDataObject = (ctx.check(1) && DataObject::OpCodeMatches(ctx.peek<byte>())) || (ctx.check(2) && DataObject::OpCodeMatches(ctx.peek<word>()));

				if (isDataObject)
				{
					AMLAssertPassthrough(DataObject::Parse(ctx));
				}
				else
				{
					AMLAssertPassthrough(NameString::Parse(ctx));
				}

				AMLAssert(ctx.expressionReturnType == integerType || ctx.expressionReturnType == stringType, unexpectedExpressionTypeError);
				return true; // simply leave the return value unchanged
			}
		}
		namespace DefMutex
		{
			bool Parse(AMLContext &ctx)
			{
				// MutexOp
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError);

				// NameString
				string* name;
				AMLAssertPassthrough(NameString::Parse(ctx));
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError);

				// SyncFlags
				byte syncFlags;
				AMLAssert(ctx.check(1), endOfStreamError);
				syncFlags = ctx.pop<byte>();

				ctx.debug_info() << "Mutex(" << *name << ", " << syncFlags << "),\n";
				ctx.consumeReturnByReference();
				return true;
			}
		}
	}

	bool LoadDefinitionBlock(const byte *definitionBlock, ull definitionBlockLen)
	{
		AMLContext ctx(definitionBlock, definitionBlockLen, false);

		bool result = Grammar::TermList::Parse(ctx);
		if (result == false)
		{
			cout << ctx.lastErrorAsString() << " in table " << (void*)definitionBlock << " at: \n";
			DisplayMemoryBlock(ctx.byteStream, 32);
		}

		return result;
	}
	bool DisplayDefinitionBlock(const byte *definitionBlock, ull definitionBlockLen)
	{
		AMLContext ctx(definitionBlock, definitionBlockLen, false, true);

		// cout << (void*)ctx.byteStreamPos << '\n';
		// DisplayMemoryBlock(ctx.byteStreamPos, 0x100);

		ctx.debug_info() << "DefinitionBlock {\n";
		ctx.indent();
		bool result = Grammar::TermList::Parse(ctx);

		if (result == false)
		{
			cout << ctx.lastErrorAsString() << " in table " << (void*)definitionBlock << " at: \n";
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