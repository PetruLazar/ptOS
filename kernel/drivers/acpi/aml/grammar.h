#pragma once
#include "amlcommon.h"

namespace AML
{
	typedef bool (*Operation)(AMLContext &ctx);

	class OpCodeHandler
	{
	public:
		ull opCode;
		Operation handler;
	};

	bool AssignValToVarUtil(AMLContext &ctx, Operation handler, ull val);
	bool AssignToVarUtil(AMLContext &ctx, Operation handler, AMLDataObject obj);

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
		namespace DefElse { bool Parse(AMLContext &ctx); bool Skip(AMLContext &ctx); static constexpr byte opCode = 0xa1; }
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
		namespace DefIndex { bool Parse(AMLContext &ctx); bool ParseAsReference(AMLContext &ctx); static constexpr byte opCode = 0x88; }
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
		namespace PackageElement { bool Parse(AMLContext &ctx); }
		namespace DefMutex { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x015b; }
		namespace SuperName { bool Parse(AMLContext &ctx); }
		namespace SimpleName { bool Parse(AMLContext &ctx); }
		namespace Target { bool Parse(AMLContext &ctx); }
		namespace DebugObj { bool Parse(AMLContext &ctx); static constexpr word extOpCode = 0x315b; }
		namespace UserTermObj { bool Parse(AMLContext &ctx); }

		namespace Operand { bool Evaluate(AMLContext &ctx, ull &result); }

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
			bool ParseAsSimpleName(AMLContext &ctx);
			bool Parse(AMLContext &ctx);
		}
		namespace ArgObj
		{
			static constexpr byte opCodeBase = 0x68;
			static constexpr byte opCodeCount = 7;
			inline bool OpcodeMatches(byte opcode) { return opcode >= opCodeBase && opcode < opCodeBase + opCodeCount; }
			bool ParseAsSimpleName(AMLContext &ctx);
			bool Parse(AMLContext &ctx);
		}
	}
}