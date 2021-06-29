#pragma once


#include "main.h"
#include "resource.h"




using ResourceName = std::string;
enum ResourceType : int
{
	SQL,
	JSON,
	BINARY,
	FONT
};


//DLL embeded resource interface
template<typename T, ResourceType Type>
class IResource
{
	const char* getTypeName(ResourceType type)
	{
		switch (type)
		{
		case ResourceType::SQL: return "SQL";
		case ResourceType::JSON: return "JSON";
		case ResourceType::BINARY: return "BINARY";
		case ResourceType::FONT: return "FONTS";
		}
		return "NOT";
	}
public:
	IResource() {};
	IResource(ResourceName name, HMODULE hModule = nullptr)
		: m_name(name), m_hModule(hModule)
	{}
	~IResource() {
		free();
	}

	ResourceType getType() {
		return Type;
	}

	ResourceName getName() {
		return "IDR_" + m_name;
	}

	HMODULE getModule() {
		return m_hModule;
	}

	virtual T getData() = 0;

	std::size_t getSize() {
		return m_Size;
	}

	void load() {
		HRSRC resource = find();
		if (resource) {
			m_hMemory = LoadResource(getModule(), resource);
			m_Size = SizeofResource(getModule(), resource);
		}
	}
	
	bool isLoaded() {
		return getSize() != 0;
	}

	void free() {
		UnlockResource(getMemory());
		FreeResource(getMemory());
	}
private:
	ResourceName m_name = "";
	HMODULE m_hModule = nullptr;
	HGLOBAL m_hMemory = nullptr;
	std::size_t m_Size = 0;
	
	HRSRC find() {
		return FindResource(
			m_hModule,
			getName().c_str(),
			getTypeName(getType())
		);
	}
protected:
	HGLOBAL getMemory() {
		return m_hMemory;
	}

	LPVOID getRawData() {
		return LockResource(getMemory());
	}
};


//WARNING: delete ids in resource manager(symbols), remain only accessing by name

class SQL_Res : public IResource<std::string, ResourceType::SQL>
{
public:
	using IResource::IResource;

	std::string getData() override {
		auto data = static_cast<const char*>(
			getRawData()
		);
		return data;
	}
};

class JSON_Res : public IResource<json, ResourceType::JSON>
{
public:
	using IResource::IResource;

	json getData() override {
		auto data = static_cast<const char*>(
			getRawData()
		);
		return json::parse(data);
	}
};

class BINARY_Res : public IResource<byte*, ResourceType::BINARY>
{
public:
	using IResource::IResource;

	byte* getData() override {
		return static_cast<byte*>(
			getRawData()
		);
	}
};

class FONT_Res : public IResource<byte*, ResourceType::FONT>
{
public:
	using IResource::IResource;

	byte* getData() override {
		return static_cast<byte*>(
			getRawData()
		);
	}
};