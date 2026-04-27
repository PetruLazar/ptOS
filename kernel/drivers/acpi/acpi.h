#pragma once
#include "acpibase.h"
#include "fadt.h"
#include "dsdt.h"
#include "madt.h"
#include "ssdt.h"
#include "aml.h"
#include <string.h>

namespace ACPI
{
	void Initialize();
	void CleanUp();

	void listRootEntries();
	GenericSDT* getTable(const char tableId[4]);

	class ACPINamedObject;

	ACPINamedObject* GetRootNamespace();

	class ACPINamedObject
	{
	public:
		enum ScopeType : byte
		{
			predefinedScopeType,
			deviceType,
			methodType,
			nameType,

			anyType
		};

	private:
		ScopeType scopeType;
		std::string simpleName;
		ACPINamedObject *parent;

	protected:
		std::vector<ACPINamedObject*> children;
		
		ACPINamedObject(ScopeType scopeType) : scopeType(scopeType) { }

		virtual ACPINamedObject* getChild(const std::string& simpleName, ScopeType desiredType = anyType)
		{
			for (auto elem : children)
			{
				if (elem->GetSimpleName() == simpleName && (desiredType == anyType || desiredType == elem->GetScopeType()))
					return elem;
			}

			return nullptr;
		}
		virtual bool addChild(const std::string& simpleName, ACPINamedObject* obj)
		{
			if (getChild(simpleName) != nullptr) return false; // duplicate

			obj->parent = this;
			obj->simpleName = simpleName;
			children.push_back(obj);
			return true;
		}

	public:
		virtual ~ACPINamedObject()
		{
			for (auto elem : children)
				delete elem;
		}

		inline const std::string& GetSimpleName() { return simpleName; }
		ScopeType GetScopeType() { return scopeType; }

		virtual ACPINamedObject* get(const std::string& name, ScopeType desiredType = anyType)
		{
			ACPINamedObject* current = this;
			const char* cstr = name.data();
			ull len = name.length();

			if (len != 0 && cstr[0] == '\\')
			{
				current = GetRootNamespace();
				cstr++;
				len--;
			}

			while (len != 0 && cstr[0] == '^')
			{
				current = current->parent;

				if (current == nullptr) return nullptr;

				cstr++;
				len--;
			}

			while (len != 0)
			{
				current = current->getChild(std::string(cstr, 4));

				if (current == nullptr) return nullptr;

				cstr += 4;
				len -= 4;
			}

			if (desiredType == anyType || desiredType == current->GetScopeType())
			{
				return current;
			}

			return nullptr;
		}
		virtual bool add(const std::string& name, ACPINamedObject* obj)
		{
			if (name.length() == 4)
			{
				return addChild(name, obj);
			}

			std::string parentpath(name.data(), name.length() - 4);
			std::string simpleName(name.data() + parentpath.length(), 4);

			ACPINamedObject* scope = get(parentpath);
			if (scope == nullptr) return false;

			return scope->addChild(simpleName, obj);
		}

		virtual void DisplayContents(std::string& indentation) = 0;
	};

	void testPRT();
}