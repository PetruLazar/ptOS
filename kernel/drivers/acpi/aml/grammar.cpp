#include "grammar.h"

using namespace std;

namespace AML
{
	namespace Grammar
	{
		namespace TermList
		{
			bool Parse(AMLContext &ctx)
			{
				while (ctx.length > 0)
				{
					AMLAssertPassthrough(TermObj::Parse(ctx), TermList::Parse);
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
				// try extended op codes
				if (ctx.check(2))
				{
					word extOpCode = ctx.peek<word>();
					for (auto& OpCodeHandler : possibleExtOpCodes)
					{
						if (OpCodeHandler.opCode == extOpCode) return OpCodeHandler.handler(ctx);
					}
				}

				// try basic opcodes
				if (ctx.check(1))
				{
					byte opCode = ctx.peek<byte>();
					for (auto& OpCodeHandler : possibleOpCodes)
					{
						if (OpCodeHandler.opCode == opCode) return OpCodeHandler.handler(ctx);
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
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefName::Parse);

				// NameString
				string *tmpname, name;
				AMLAssertPassthrough(NameString::Parse(ctx), DefName::Parse);
				AMLAssert(ctx.getReturnByReference(stringType, tmpname), unknownError, DefName::Parse);
				name = *tmpname;

				// DataRefObject
				AMLAssertPassthrough(DataRefObject::Parse(ctx), DefName::Parse);

				switch (ctx.returnValueObj.type)
				{
					case integerType:
					{
						ull data;
						AMLAssert(ctx.getReturnByValue(integerType, &data), unknownError, DefName::Parse);
						ctx.debug_info() << "Name (" << name << ", " << data << ")\n";
						break;
					}
					case stringType:
					{
						string* val;
						AMLAssert(ctx.getReturnByReference(stringType, val), unknownError, DefName::Parse);
						ctx.debug_info() << "Name (" << name << ", \"" << *val << "\")\n";
						break;
					}
					case bufferType:
					{
						vector<byte> *buffer;
						AMLAssert(ctx.getReturnByReference(bufferType, buffer), unknownError, DefName::Parse);
						ctx.debug_info() << "Name (" << name << ") Buffer(" << buffer->getSize() << ") {\n";
						ctx.indent();
						for (byte b : *buffer)
						{
							ctx.debug_info() << ostream::base::hex << "0x" << b << ",\n" << ostream::base::dec;
						}
						ctx.outdent();
						ctx.debug_info() << "}\n";
						break;
					}
					case packageType:
					{
						PackageObject *package;
						AMLAssert(ctx.getReturnByReference(packageType, package), unknownError, DefName::Parse);
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
				AMLAssert(ctx.assertPop<byte>(DefScope::opCode), unknownError, DefName::Parse);

				// enter sub-context
				ctx.createSubcontext();

				// pkgLength
				AMLAssertPassthrough(PkgLength::Parse(ctx), DefName::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefName::Parse);
				ctx.limitSubcontext(pkgLength);

				// NameString
				string* name;
				AMLAssertPassthrough(NameString::Parse(ctx), DefName::Parse);
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError, DefName::Parse);

				ACPI::ACPINamedObject* oldScope = ctx.currentScope;
				ctx.currentScope = ctx.currentScope->get(*name);
				AMLAssert(ctx.currentScope != nullptr, inexistentNamespaceError, DefName::Parse);

				ctx.debug_info() << "Scope (" << *name << ") {\n";
				ctx.indent();

				// TermList
				AMLAssertPassthrough(TermList::Parse(ctx), DefName::Parse);

				ctx.outdent();
				ctx.debug_info() << "}\n";
				ctx.popSubContext();

				ctx.currentScope = oldScope;

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
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError, DefOpRegion::Parse);

				// NameString
				AMLAssertPassthrough(NameString::Parse(ctx), DefOpRegion::Parse);
				AMLAssert(ctx.getReturnByReference(stringType, tmpname), unknownError, DefOpRegion::Parse);
				name = *tmpname;

				// RegionSpace
				AMLAssert(ctx.check(1), endOfStreamError, DefOpRegion::Parse);
				regionSpace = ctx.pop<byte>();

				// RegionOffset => Integer
				AMLAssertPassthrough(TermArg::Parse(ctx), DefOpRegion::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &regionOffset), unknownError, DefOpRegion::Parse);

				// RegionLen => Integer
				AMLAssertPassthrough(TermArg::Parse(ctx), DefOpRegion::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &regionLen), unexpectedExpressionTypeError, DefOpRegion::Parse);

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
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefMethod::Parse);

				ctx.createSubcontext();

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx), DefMethod::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefMethod::Parse);
				ctx.limitSubcontext(pkgLength);

				// NameString
				string *name;
				AMLAssertPassthrough(NameString::Parse(ctx), DefMethod::Parse);
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError, DefMethod::Parse);

				// MethodFlags
				AMLAssert(ctx.check(1), endOfStreamError, DefMethod::Parse);
				byte methodFlags = ctx.pop<byte>();

				ACPIMethod* method = new ACPIMethod();
				if (!ctx.currentScope->add(*name, method))
				{
					delete method;
					return ctx.logError(inexistentNamespaceError);
				}

				method->flags.raw = methodFlags;
				method->methodStart = ctx.byteStream;
				method->methodLen = ctx.length;

				ctx.debug_info() << "Method (" << *name << ", " << (uint)methodFlags << ") {\n";
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
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError, DefField::Parse);

				// enter sub-context
				ctx.createSubcontext();

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx), DefField::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefField::Parse);
				ctx.limitSubcontext(pkgLength);

				// NameString
				string *name;
				AMLAssertPassthrough(NameString::Parse(ctx), DefField::Parse);
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError, DefField::Parse);

				// FieldFlags
				AMLAssert(ctx.check(1), endOfStreamError, DefField::Parse);
				byte fieldFlags = ctx.pop<byte>();

				ctx.debug_info() << "Field (" << *name << ", " << (uint)fieldFlags << ") {\n";
				ctx.indent();

				// FieldList
				AMLAssertPassthrough(FieldList::Parse(ctx), DefField::Parse);

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
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError, DefDevice::Parse);

				ctx.createSubcontext();

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx), DefDevice::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefDevice::Parse);
				ctx.limitSubcontext(pkgLength);

				// NameString
				string *name;
				AMLAssertPassthrough(NameString::Parse(ctx), DefDevice::Parse);
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError, DefDevice::Parse);

				ACPIDevice* device = new ACPIDevice();
				if (!ctx.currentScope->add(*name, device))
				{
					delete device;
					return ctx.logError(inexistentNamespaceError);
				}
				ACPI::ACPINamedObject* oldScope = ctx.currentScope;
				ctx.currentScope = device;

				ctx.debug_info() << "Device (" << *name << ") {\n";
				ctx.indent();

				// TermList
				AMLAssertPassthrough(TermList::Parse(ctx), DefDevice::Parse);

				ctx.outdent();
				ctx.debug_info() << "}\n";
				ctx.popSubContext();

				ctx.currentScope = oldScope;

				return true;
			}
		}
		namespace DefProcessor
		{
			bool Parse(AMLContext &ctx)
			{
				// deprecated, parse as little data as possible and skip object
				// ProcessorOp
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError, DefProcessor::Parse);

				// PkgLength
				ull pkgLength;
				ctx.createSubcontext();
				AMLAssertPassthrough(PkgLength::Parse(ctx), DefProcessor::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefProcessor::Parse);
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
				AMLAssert(ctx.check(1), endOfStreamError, PkgLength::Parse);
				byte leadByte = ctx.pop<byte>();
				uint additionalBytes = leadByte >> 6;

				AMLAssert(ctx.check(additionalBytes), endOfStreamError, PkgLength::Parse);
				
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
					AMLAssertPassthrough(NameSeg::Parse(ctx), NameString::AppendNameSegs);
					AMLAssert(ctx.getReturnByReference(stringType, nameSegVal), unknownError, NameString::AppendNameSegs);
					result.append(*nameSegVal);
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
					AMLAssertPassthrough(AppendNameSegs(ctx, retVal, 2), NameString::Parse);
				}
				else if (ctx.assertPop<byte>(multiNamePrefix))
				{
					// MultiNamePath := MultiNamePrefix SegCount NameSeg(SegCount)
					AMLAssert(ctx.check(1), endOfStreamError, NameString::Parse);
					byte segCount = ctx.pop<byte>();
					AMLAssertPassthrough(AppendNameSegs(ctx, retVal, segCount), NameString::Parse);
				}
				else if (ctx.assertPop<byte>(nullChar))
				{
					// NullName
					// nothing else to append
				}
				else
				{
					// NameSeg
					AMLAssertPassthrough(AppendNameSegs(ctx, retVal, 1), NameString::Parse);
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
				AMLAssert(ctx.check(4), endOfStreamError, NameSeg::Parse);

				AMLAssert(IsLeadNameChar(ctx.byteStream[0]), invalidCharacterInNameSegError, NameSeg::Parse);
				for (uint i = 1; i < 4; i++)
				{
					AMLAssert(IsNameChar(ctx.byteStream[i]), invalidCharacterInNameSegError, NameSeg::Parse);
				}

				string *retVal = new string((char*)ctx.byteStream, 4);
				ctx.advanceStream(4);
				return ctx.returnValue(stringType, retVal);
			}
		}
		namespace LocalObj
		{
			bool ParseAsSimpleName(AMLContext &ctx)
			{
				AMLAssert(ctx.check(1), unknownError, LocalObj::ParseAsSimpleName);
				AMLAssert(ctx.runtime == true, nonRuntimeLocalUsageError, LocalObj::ParseAsSimpleName);

				byte opCode = ctx.pop<byte>();
				byte localIdx = opCode - opCodeBase;
				AMLAssert(localIdx < ctx.locals.getSize(), unknownError, LocalObj::ParseAsSimpleName);

				return ctx.returnValue(variableType, &ctx.locals[localIdx]);
			}
			bool Parse(AMLContext &ctx)
			{
				AMLAssertPassthrough(ParseAsSimpleName(ctx), LocalObj::Parse);
				AMLDataObject* obj;
				AMLAssert(ctx.getReturnByReference(variableType, obj), unknownError, LocalObj::Parse);
				AMLDataObject clonedObj = obj->clone();
				return ctx.returnValue(clonedObj.type, clonedObj.valuePtr);
			}
		}
		namespace ArgObj
		{
			bool ParseAsSimpleName(AMLContext &ctx)
			{
				AMLAssert(ctx.check(1), unknownError, ArgObj::ParseAsSimpleName);
				AMLAssert(ctx.runtime == true, nonRuntimeArgUsageError, ArgObj::ParseAsSimpleName);

				byte opCode = ctx.pop<byte>();
				byte argIdx = opCode - opCodeBase;
				AMLAssert(argIdx < ctx.args.getSize(), argOutOfRangeError, ArgObj::ParseAsSimpleName);

				return ctx.returnValue(variableType, &ctx.args[argIdx]);
			}
			bool Parse(AMLContext &ctx)
			{
				AMLAssertPassthrough(ParseAsSimpleName(ctx), ArgObj::Parse);
				AMLDataObject* obj;
				AMLAssert(ctx.getReturnByReference(variableType, obj), unknownError, ArgObj::Parse);
				AMLDataObject clonedObj = obj->clone();
				return ctx.returnValue(clonedObj.type, clonedObj.valuePtr);
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
				// try extended op codes
				if (ctx.check(2))
				{
					word extOpCode = ctx.peek<word>();
					for (auto& OpCodeHandler : possibleExtOpCodes)
					{
						if (OpCodeHandler.opCode == extOpCode) return OpCodeHandler.handler(ctx);
					}
				}

				// try basic opcodes
				if (ctx.check(1))
				{
					byte opCode = ctx.peek<byte>();
					for (auto& opcodeHandler : possibleOpCodes)
					{
						if (opcodeHandler.opCode == opCode) return opcodeHandler.handler(ctx);
					}
				}

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
				// IfOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefIfElse::Parse);

				// PkgLength
				ull pkgLength;
				ctx.createSubcontext();
				AMLAssertPassthrough(PkgLength::Parse(ctx), DefIfElse::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefIfElse::Parse);
				ctx.limitSubcontext(pkgLength);

				// Predicate
				ull predicate;
				Operand::Evaluate(ctx, predicate);

				if (predicate != 0)
				{
					// TermList
					AMLAssertPassthrough(TermList::Parse(ctx), DefIfElse::Parse);

					ctx.popSubContext();

					// DefElse
					AMLAssertPassthrough(DefElse::Skip(ctx), DefIfElse::Parse);
				}
				else
				{
					// TermList
					ctx.advanceStream(ctx.length);

					ctx.popSubContext();

					// DefElse
					AMLAssertPassthrough(DefElse::Parse(ctx), DefIfElse::Parse);
				}

				return true;
			}
		}
		namespace DefElse
		{
			bool Parse(AMLContext &ctx)
			{
				// Nothing | <ElseOp PkgLength TermList>
				if (ctx.assertPop<byte>(opCode))
				{
					// PkgLength
					ull pkgLength;
					ctx.createSubcontext();
					AMLAssertPassthrough(PkgLength::Parse(ctx), DefElse::Parse);
					AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefElse::Parse);
					ctx.limitSubcontext(pkgLength);

					// TermList
					AMLAssertPassthrough(TermList::Parse(ctx), DefElse::Parse);

					ctx.popSubContext();
				}

				return true;
			}
			bool Skip(AMLContext &ctx)
			{
				// Nothing | <ElseOp PkgLength TermList>
				if (ctx.assertPop<byte>(opCode))
				{
					// PkgLength
					ull pkgLength;
					ctx.createSubcontext();
					AMLAssertPassthrough(PkgLength::Parse(ctx), DefElse::Parse);
					AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefElse::Parse);
					ctx.limitSubcontext(pkgLength);

					// TermList
					ctx.advanceStream(ctx.length);

					ctx.popSubContext();
				}

				return true;
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
				// ReturnOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefReturn::Parse);

				// ArgObject
				AMLAssertPassthrough(TermArg::Parse(ctx), DefReturn::Parse);

				return ctx.logError(returnFromFunctionRetCode);
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
				// WhileOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefWhile::Parse);

				// PkgLength
				ull pkgLength;
				ctx.createSubcontext();
				AMLAssertPassthrough(PkgLength::Parse(ctx), DefWhile::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefWhile::Parse);
				ctx.limitSubcontext(pkgLength);

				const byte* loopPosition = ctx.byteStream;
				while (true)
				{
					// Predicate
					ull predicate;
					AMLAssertPassthrough(TermArg::Parse(ctx), DefWhile::Parse);
					AMLAssert(ctx.getReturnByValue(integerType, &predicate), unexpectedExpressionTypeError, DefWhile::Parse);
					if (predicate == 0)
					{
						ctx.advanceStream(ctx.length);
						break;
					}

					// TermList
					AMLLoopAssertPassthrough(TermList::Parse(ctx), DefWhile::Parse);

					// loop back to evaluating the predicate
					ctx.revertStream(ctx.byteStream - loopPosition);
				}

				ctx.popSubContext();
				return true;
			}
		}
		namespace DefBuffer
		{
			bool Parse(AMLContext &ctx)
			{
				// BufferOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefBuffer::Parse);

				// PkgLength
				ull pkgLength;
				ctx.createSubcontext();
				AMLAssertPassthrough(PkgLength::Parse(ctx), DefBuffer::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefBuffer::Parse);
				ctx.limitSubcontext(pkgLength);

				// BufferSize
				ull bufferSize;
				AMLAssertPassthrough(TermArg::Parse(ctx), DefBuffer::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &bufferSize), unexpectedExpressionTypeError, DefBuffer::Parse);

				// ByteList
				AMLAssert(ctx.length == bufferSize, inconsistentBufferParametersError, DefBuffer::Parse);
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
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefPackage::Parse);

				// PkgLength
				ull pkgLength;
				ctx.createSubcontext();
				AMLAssertPassthrough(PkgLength::Parse(ctx), DefPackage::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, DefPackage::Parse);
				ctx.limitSubcontext(pkgLength);

				// NumElements
				byte numElements;
				AMLAssert(ctx.check(1), endOfStreamError, DefPackage::Parse);
				numElements = ctx.pop<byte>();

				PackageObject *package = new PackageObject();
				package->resize(numElements);

				// PackageElementList
				ull idx = 0;
				while (ctx.length > 0)
				{
					if (PackageElement::Parse(ctx) == false)
					{
						delete package;
						return false;
					}
					if (idx >= numElements)
					{
						delete package;
						return ctx.logError(tooManyPackageInitializersError);
					}
					package->at(idx++).assignAndDeallocate(ctx.returnValueObj.type, ctx.returnValueObj.valuePtr);
					ctx.takeReturnValueOwnership();
				}

				ctx.popSubContext();
				return ctx.returnValue(packageType, package);
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
				// StoreOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefStore::Parse);

				// TermArg
				AMLDataObject obj;
				AMLAssertPassthrough(TermArg::Parse(ctx), DefStore::Parse);
				obj = ctx.returnValueObj;
				ctx.takeReturnValueOwnership();

				// SuperName
				return AssignToVarUtil(ctx, SuperName::Parse, obj);
			}
		}
		namespace DefAdd
		{
			bool Parse(AMLContext &ctx)
			{
				// AddOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefAdd::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefAdd::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefAdd::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefAdd::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefAdd::Parse);

				return ctx.returnValue(integerType, new ull(op1 + op2));
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
				// SubtractOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefSubtract::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefSubtract::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefSubtract::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefSubtract::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefSubtract::Parse);

				return ctx.returnValue(integerType, new ull(op1 - op2));
			}
		}
		namespace DefIncrement
		{
			bool Parse(AMLContext &ctx)
			{
				// IncrementOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefIncrement::Parse);

				// SuperName
				AMLDataObject* var;
				AMLAssertPassthrough(SuperName::Parse(ctx), DefIncrement::Parse);
				AMLAssert(ctx.getReturnByReference(variableType, var), unexpectedExpressionTypeError, DefIncrement::Parse);
				AMLAssert(var->type == integerType, unexpectedExpressionTypeError, DefIncrement::Parse);

				var->deref<ull>()++;
				return true;
			}
		}
		namespace DefDecrement
		{
			bool Parse(AMLContext &ctx)
			{
				// DecrementOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefDecrement::Parse);

				// SuperName
				AMLDataObject* var;
				AMLAssertPassthrough(SuperName::Parse(ctx), DefDecrement::Parse);
				AMLAssert(ctx.getReturnByReference(variableType, var), unexpectedExpressionTypeError, DefDecrement::Parse);
				AMLAssert(var->type == integerType, unexpectedExpressionTypeError, DefDecrement::Parse);

				var->deref<ull>()--;
				return true;
			}
		}
		namespace DefMultiply
		{
			bool Parse(AMLContext &ctx)
			{
				// MultiplyOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefMultiply::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefMultiply::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefMultiply::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefMultiply::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefMultiply::Parse);

				return ctx.returnValue(integerType, new ull(op1 * op2));
			}
		}
		namespace DefDivide
		{
			bool Parse(AMLContext &ctx)
			{
				// DivideOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefDivide::Parse);

				ull op1, op2;

				// Dividend
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefDivide::Parse);

				// Divisor
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefDivide::Parse);

				// Remainder
				// Quotient

				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefShiftLeft
		{
			bool Parse(AMLContext &ctx)
			{
				// ShiftLeftOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefShiftLeft::Parse);

				ull op, shiftCount;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op), DefShiftLeft::Parse);

				// ShiftCount
				AMLAssertPassthrough(Operand::Evaluate(ctx, shiftCount), DefShiftLeft::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefShiftLeft::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefShiftLeft::Parse);

				return ctx.returnValue(integerType, new ull(op << shiftCount));
			}
		}
		namespace DefShiftRight
		{
			bool Parse(AMLContext &ctx)
			{
				// ShiftRightOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefShiftRight::Parse);

				ull op, shiftCount;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op), DefShiftRight::Parse);

				// ShiftCount
				AMLAssertPassthrough(Operand::Evaluate(ctx, shiftCount), DefShiftRight::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefShiftRight::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefShiftRight::Parse);

				return ctx.returnValue(integerType, new ull(op >> shiftCount));
			}
		}
		namespace DefAnd
		{
			bool Parse(AMLContext &ctx)
			{
				// AndOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefAnd::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefAnd::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefAnd::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefAnd::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefAnd::Parse);

				return ctx.returnValue(integerType, new ull(op1 & op2));
			}
		}
		namespace DefNand
		{
			bool Parse(AMLContext &ctx)
			{
				// NandOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefNand::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefNand::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefNand::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefNand::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefNand::Parse);

				return ctx.returnValue(integerType, new ull(~(op1 & op2)));
			}
		}
		namespace DefOr
		{
			bool Parse(AMLContext &ctx)
			{
				// OrOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefOr::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefOr::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefOr::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefOr::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefOr::Parse);

				return ctx.returnValue(integerType, new ull(op1 | op2));
			}
		}
		namespace DefNor
		{
			bool Parse(AMLContext &ctx)
			{
				// NorOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefNor::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefNor::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefNor::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefNor::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefNor::Parse);

				return ctx.returnValue(integerType, new ull(~(op1 | op2)));
			}
		}
		namespace DefXor
		{
			bool Parse(AMLContext &ctx)
			{
				// XorOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefXor::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefXor::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefXor::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefXor::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefXor::Parse);

				return ctx.returnValue(integerType, new ull(op1 ^ op2));
			}
		}
		namespace DefNot
		{
			bool Parse(AMLContext &ctx)
			{
				// NotOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefNot::Parse);

				ull op;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op), DefXor::Parse);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefNot::Parse);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefNot::Parse);

				return ctx.returnValue(integerType, new ull(~op));
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
				// LandOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefLand::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefLand::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefLand::Parse);

				return ctx.returnValue(integerType, new ull((op1 != 0 && op2 != 0) ? 1 : 0));
			}
		}
		namespace DefLor
		{
			bool Parse(AMLContext &ctx)
			{
				// LorOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefLor::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefLor::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefLor::Parse);

				return ctx.returnValue(integerType, new ull((op1 != 0 || op2 != 0) ? 1 : 0));
			}
		}
		namespace DefLnot
		{
			bool Parse(AMLContext &ctx)
			{
				// LnotOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefLnot::Parse);

				ull op;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op), DefLnot::Parse);

				return ctx.returnValue(integerType, new ull(op == 0 ? 1 : 0));
			}
		}
		namespace DefLequal
		{
			bool Parse(AMLContext &ctx)
			{
				// LequalOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefLequal::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefLequal::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefLequal::Parse);

				return ctx.returnValue(integerType, new ull(op1 == op2 ? 1 : 0));
			}
		}
		namespace DefLgreater
		{
			bool Parse(AMLContext &ctx)
			{
				// LgreaterOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefLgreater::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefLgreater::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefLgreater::Parse);

				return ctx.returnValue(integerType, new ull(op1 > op2 ? 1 : 0));
			}
		}
		namespace DefLless
		{
			bool Parse(AMLContext &ctx)
			{
				// LlessOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefLless::Parse);

				ull op1, op2;

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op1), DefLless::Parse);

				// Operand
				AMLAssertPassthrough(Operand::Evaluate(ctx, op2), DefLless::Parse);

				return ctx.returnValue(integerType, new ull(op1 < op2 ? 1 : 0));
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
					AMLAssert(ctx.check(1), endOfStreamError, ImmediateConst::Parse);
					return ctx.returnValue(integerType, new ull(ctx.pop<byte>()));
				}
				if (ctx.assertPop<byte>(wordPrefix))
				{
					AMLAssert(ctx.check(2), endOfStreamError, ImmediateConst::Parse);
					return ctx.returnValue(integerType, new ull(ctx.pop<word>()));
				}
				if (ctx.assertPop<byte>(dwordPrefix))
				{
					AMLAssert(ctx.check(4), endOfStreamError, ImmediateConst::Parse);
					return ctx.returnValue(integerType, new ull(ctx.pop<dword>()));
				}
				if (ctx.assertPop<byte>(qwordPrefix))
				{
					AMLAssert(ctx.check(8), endOfStreamError, ImmediateConst::Parse);
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
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, StringObj::Parse);

				// AsciiCharList
				string *str = new string;
				while (ctx.check(1) && isAsciiChar(ctx.peek<byte>()))
				{
					str->push_back(ctx.pop<char>());
				}

				// NullChar
				ctx.returnValue(stringType, str);
				AMLAssert(ctx.assertPop(NameString::nullChar), endOfStreamError, StringObj::Parse);
				
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
					AMLAssertPassthrough(FieldElement::Parse(ctx), FieldList::Parse);
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
				AMLAssertPassthrough(NameSeg::Parse(ctx), NamedField::Parse);
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError, NamedField::Parse);
				ctx.debug_info() << "NamedField (" << *name << ", ";

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx), NamedField::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, NamedField::Parse);
				ctx.debug_info(true) << pkgLength << ")\n";

				return true;
			}
		}
		namespace ReservedField
		{
			bool Parse(AMLContext &ctx)
			{
				// 0x00
				AMLAssert(ctx.assertPop<byte>(0x00), unknownError, ReservedField::Parse);

				// PkgLength
				ull pkgLength;
				AMLAssertPassthrough(PkgLength::Parse(ctx), ReservedField::Parse);
				AMLAssert(ctx.getReturnByValue(integerType, &pkgLength), unknownError, ReservedField::Parse);

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
		namespace PackageElement
		{
			bool Parse(AMLContext &ctx)
			{
				// PackageElement := DataRefObject | NameString
				// DataRefObject := DataObject | ObjectReference(N/A)

				bool isDataObject = (ctx.check(1) && DataObject::OpCodeMatches(ctx.peek<byte>())) || (ctx.check(2) && DataObject::OpCodeMatches(ctx.peek<word>()));

				if (isDataObject)
				{
					AMLAssertPassthrough(DataObject::Parse(ctx), PackageElement::Parse);
				}
				else
				{
					AMLAssertPassthrough(NameString::Parse(ctx), PackageElement::Parse);
				}

				AMLAssert(ctx.returnValueObj.type == integerType || ctx.returnValueObj.type == stringType, unexpectedExpressionTypeError, PackageElement::Parse);
				return true; // simply leave the return value unchanged
			}
		}
		namespace DefMutex
		{
			bool Parse(AMLContext &ctx)
			{
				// MutexOp
				AMLAssert(ctx.assertPop<word>(extOpCode), unknownError, DefMutex::Parse);

				// NameString
				string* name;
				AMLAssertPassthrough(NameString::Parse(ctx), DefMutex::Parse);
				AMLAssert(ctx.getReturnByReference(stringType, name), unknownError, DefMutex::Parse);

				// SyncFlags
				byte syncFlags;
				AMLAssert(ctx.check(1), endOfStreamError, DefMutex::Parse);
				syncFlags = ctx.pop<byte>();

				ctx.debug_info() << "Mutex(" << *name << ", " << syncFlags << "),\n";
				return true;
			}
		}
		namespace SuperName
		{
			static constexpr OpCodeHandler possibleOpCodes[] =
			{
				{ DefRefOf::opCode, DefRefOf::Parse },
				{ DefDerefOf::opCode, DefDerefOf::Parse },
				{ DefIndex::opCode, DefIndex::ParseAsReference },
				// UserTermObj ???
			};
			static constexpr OpCodeHandler possibleExtOpCodes[] =
			{
				{ DebugObj::extOpCode, DebugObj::Parse },
			};
			bool Parse(AMLContext &ctx)
			{
				// try extended op codes
				if (ctx.check(2))
				{
					word extOpCode = ctx.peek<word>();
					for (auto& OpCodeHandler : possibleExtOpCodes)
					{
						if (OpCodeHandler.opCode == extOpCode) return OpCodeHandler.handler(ctx);
					}
				}

				// try basic opcodes
				if (ctx.check(1))
				{
					byte opCode = ctx.peek<byte>();
					for (auto& opcodeHandler : possibleOpCodes)
					{
						if (opcodeHandler.opCode == opCode) return opcodeHandler.handler(ctx);
					}
				}

				//no opcode match: SimpleName or UserTermObj
				return SimpleName::Parse(ctx);
			}
		}
		namespace SimpleName
		{
			bool Parse(AMLContext &ctx)
			{
				// NameString | ArgObj | LocalObj

				if (ctx.check(1) && ArgObj::OpcodeMatches(ctx.peek<byte>()))
				{
					return ArgObj::ParseAsSimpleName(ctx);
				}
				if (ctx.check(1) && LocalObj::OpcodeMatches(ctx.peek<byte>()))
				{
					return LocalObj::ParseAsSimpleName(ctx);
				}

				return ctx.logError(unhandledElementError); // DefIndex::ParseAsReference relies on only ArgObj or LocalObj; change when implementing the name string stuff
				return NameString::Parse(ctx);
			}
		}
		namespace Target
		{
			bool Parse(AMLContext &ctx)
			{
				if (ctx.assertPop<byte>(0x00))
				{
					return ctx.returnValue<void>(voidType, nullptr);
				}

				return SuperName::Parse(ctx);
			}
		}
		namespace DebugObj
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
		namespace DefDerefOf
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}
		namespace DefIndex
		{
			bool ParseAsReference(AMLContext &ctx)
			{
				// IndexOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefIndex::ParseAsReference);

				// BuffPkgStrObj
				AMLDataObject* ref;
				AMLAssertPassthrough(SimpleName::Parse(ctx), DefIndex::ParseAsReference);
				AMLAssert(ctx.getReturnByReference(variableType, ref), unexpectedExpressionTypeError, DefIndex::ParseAsReference);
				AMLAssert(ref->type == packageType, unexpectedExpressionTypeError, DefIndex::ParseAsReference);

				// IndexValue
				ull idx;
				AMLAssertPassthrough(Operand::Evaluate(ctx, idx), DefIndex::ParseAsReference);

				// Target
				AMLAssertPassthrough(Target::Parse(ctx), DefIndex::ParseAsReference);
				AMLAssert(ctx.returnValueObj.type == voidType, unknownError, DefIndex::ParseAsReference);

				PackageObject* pkg = &ref->deref<PackageObject>();
				AMLAssert(idx < pkg->getSize(), indexOutOfRangeError, DefIndex::ParseAsReference);

				return ctx.returnValue(variableType, &pkg->at(idx));
			}
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);

				// IndexOp
				AMLAssert(ctx.assertPop<byte>(opCode), unknownError, DefIndex::Parse);

				// BuffPkgStrObj

				// IndexValue

				// Target

				return ctx.logError(unhandledElementError);
			}
		}
		namespace UserTermObj
		{
			bool Parse(AMLContext &ctx)
			{
				return ctx.logError(unhandledElementError);
			}
		}

		namespace Operand
		{
			bool Evaluate(AMLContext &ctx, ull &result)
			{
				AMLAssertPassthrough(TermArg::Parse(ctx), Operand::Evaluate);
				AMLAssert(ctx.getReturnByValue(integerType, &result), unexpectedExpressionTypeError, Operand::Evaluate);
				return true;
			}
		}
	}
}