#pragma once
#include <types.h>

namespace ACPI
{
	inline byte doChecksum(byte* addr, qword len)
	{
		byte checksum = 0;
		for (qword i = 0; i < len; i++)
			checksum += addr[i];
		return checksum;
	}

	namespace TableId
	{
		static constexpr char RSDT[] = "RSDT";
		static constexpr char XSDT[] = "XSDT";

		static constexpr char MADT[] = "APIC";
		static constexpr char FADT[] = "FACP";
	}

	class SDTHeader
	{
	public:
		const char signature[4];
		dword length;
		byte revision;
		byte checksum;
		const char OEMId[6];
		const char OEMTableId[8];
		dword OEMRevision;
		dword creatorId;
		dword creatorRevision;

		inline uint signatureAsUInt() { return *(uint*)signature; }
	};
	class GenericSDT
	{
	public:
		SDTHeader header;

		inline byte doChecksum() { return ACPI::doChecksum((byte*)this, header.length); }
		inline bool checkSignature(const char expectedSignature[4]) { return *(uint*)expectedSignature == header.signatureAsUInt(); }
		template <class FieldType> inline bool ContainsField(const FieldType& fieldPtr) const volatile
		{
			ull fieldAddr = (ull)&fieldPtr;
			ull tableAddr = (ull)this;

			return (fieldAddr >= tableAddr) && (fieldAddr < tableAddr + header.length) && (fieldAddr + sizeof(fieldPtr) <= tableAddr + header.length);
		}
	};
}