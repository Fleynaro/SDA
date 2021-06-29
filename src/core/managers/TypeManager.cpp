#pragma once
#include "TypeManager.h"
#include <DB/Mappers/DataTypeMapper.h>
#include <GhidraSync/Mappers/GhidraTypedefTypeMapper.h>
#include <GhidraSync/Mappers/GhidraStructureTypeMapper.h>
#include <GhidraSync/Mappers/GhidraClassTypeMapper.h>
#include <GhidraSync/Mappers/GhidraEnumTypeMapper.h>
#include <GhidraSync/Mappers/GhidraSignatureTypeMapper.h>
#include <Utils/ObjectHash.h>

using namespace CE;
using namespace CE::DataType;

TypeManager::TypeManager(Project* module)
	: AbstractItemManager(module)
{
	m_dataTypeMapper = new DB::DataTypeMapper(this);
	m_ghidraDataTypeMapper = new Ghidra::DataTypeMapper(this);
	addSystemTypes();
	addGhidraTypedefs();
}

TypeManager::~TypeManager() {
	delete m_dataTypeMapper;
	delete m_ghidraDataTypeMapper;
}

TypeManager::Factory TypeManager::getFactory(bool markAsNew) {
	return Factory(this, m_ghidraDataTypeMapper, m_dataTypeMapper, markAsNew);
}

void TypeManager::addSystemTypes() {
	m_items.insert({
		std::make_pair(DataType::SystemType::Void, new DataType::Void),
		std::make_pair(DataType::SystemType::Bool, new DataType::Bool),
		std::make_pair(DataType::SystemType::Byte, new DataType::Byte),
		std::make_pair(DataType::SystemType::Int8, new DataType::Int8),
		std::make_pair(DataType::SystemType::Int16, new DataType::Int16),
		std::make_pair(DataType::SystemType::Int32, new DataType::Int32),
		std::make_pair(DataType::SystemType::Int64, new DataType::Int64),
		std::make_pair(DataType::SystemType::UInt16, new DataType::UInt16),
		std::make_pair(DataType::SystemType::UInt32, new DataType::UInt32),
		std::make_pair(DataType::SystemType::UInt64, new DataType::UInt64),
		std::make_pair(DataType::SystemType::UInt128, new DataType::UInt128),
		std::make_pair(DataType::SystemType::Float, new DataType::Float),
		std::make_pair(DataType::SystemType::Double, new DataType::Double),
		std::make_pair(DataType::SystemType::Char, new DataType::Char),
		std::make_pair(DataType::SystemType::WChar, new DataType::WChar)
		});

	Iterator it(this);
	while (it.hasNext()) {
		auto type = it.next();
		type->setTypeManager(this);
	}
}

void TypeManager::addGhidraTypedefs() {
	static std::pair<std::string, DB::Id> typedefs[] = {
		std::make_pair("void", DataType::SystemType::Void),
		std::make_pair("unicode", DataType::SystemType::Void),
		std::make_pair("string", DataType::SystemType::Void),
		std::make_pair("IMAGE_RICH_HEADER", DataType::SystemType::Void),

		std::make_pair("uchar", DataType::SystemType::Byte),
		std::make_pair("uint8_t", DataType::SystemType::Byte),
		std::make_pair("undefined1", DataType::SystemType::Int8),
		std::make_pair("TerminatedCString", DataType::SystemType::Char),

		std::make_pair("short", DataType::SystemType::Int16),
		std::make_pair("ushort", DataType::SystemType::UInt16),
		std::make_pair("word", DataType::SystemType::Int16),
		std::make_pair("undefined2", DataType::SystemType::Int16),

		std::make_pair("int", DataType::SystemType::Int32),
		std::make_pair("uint", DataType::SystemType::UInt32),
		std::make_pair("long", DataType::SystemType::Int32),
		std::make_pair("ulong", DataType::SystemType::UInt32),
		std::make_pair("dword", DataType::SystemType::Int32),
		std::make_pair("float", DataType::SystemType::Float),
		std::make_pair("ImageBaseOffset32", DataType::SystemType::UInt32),
		std::make_pair("undefined4", DataType::SystemType::Int32),

		std::make_pair("longlong", DataType::SystemType::Int64),
		std::make_pair("ulonglong", DataType::SystemType::UInt64),
		std::make_pair("qword", DataType::SystemType::Int64),
		std::make_pair("double", DataType::SystemType::Double),
		std::make_pair("undefined8", DataType::SystemType::Int64),

		std::make_pair("GUID", DataType::SystemType::UInt128)
	};

	DB::Id startId = 100;
	for (const auto& it : typedefs) {
		auto type = new DataType::Typedef(this, it.first);
		type->setTypeManager(this);
		type->setId(startId);
		type->setGhidraMapper(m_ghidraDataTypeMapper->m_typedefTypeMapper);
		type->setRefType(DataType::GetUnit(findTypeById(it.second)));
		m_items.insert(std::make_pair(startId, type));
		startId++;
	}
}

void TypeManager::loadBefore() {
	m_dataTypeMapper->loadBefore();
}

void TypeManager::loadAfter() {
	m_dataTypeMapper->loadAfter();
}

void TypeManager::loadTypesFrom(ghidra::packet::SDataFullSyncPacket* dataPacket) {
	m_ghidraDataTypeMapper->load(dataPacket);
}

DataTypePtr CE::TypeManager::getType(DB::Id id) {
	return DataType::GetUnit(findTypeById(id));
}

DataTypePtr CE::TypeManager::getDefaultType(int size, bool sign, bool floating) {
	if (floating) {
		if (size == 0x4)
			return getType(SystemType::Float);
		if (size == 0x8)
			return getType(SystemType::Double);
	}
	if (size == 0x0)
		return getType(SystemType::Void);
	if (size == 0x1)
		return getType(sign ? SystemType::Char : SystemType::Byte);
	if (size == 0x2)
		return getType(sign ? SystemType::Int16 : SystemType::UInt16);
	if (size == 0x4)
		return getType(sign ? SystemType::Int32 : SystemType::UInt32);
	if (size == 0x8)
		return getType(sign ? SystemType::Int64 : SystemType::UInt64);
	return nullptr;
}

DataTypePtr CE::TypeManager::calcDataTypeForNumber(uint64_t value) {
	if ((value & ~uint64_t(0xFFFFFFFF)) == (uint64_t)0x0)
		return getType(SystemType::Int32);
	return getType(SystemType::Int64);
}

DataType::IType* CE::TypeManager::findTypeById(DB::Id id) {
	return dynamic_cast<DataType::AbstractType*>(find(id));
}

DataType::IType* CE::TypeManager::findTypeByName(const std::string& typeName)
{
	Iterator it(this);
	while (it.hasNext()) {
		auto type = it.next();
		if (type->getName() == typeName) {
			return type;
		}
	}
	return nullptr;
}

DataType::IType* CE::TypeManager::findTypeByGhidraId(Ghidra::Id id) {
	Iterator it(this);
	while (it.hasNext()) {
		auto type = it.next();
		if (getGhidraId(type) == id) {
			return type;
		}
	}
	return nullptr;
}

Ghidra::Id TypeManager::getGhidraId(DataType::IType* type) {
	if (auto userType = dynamic_cast<DataType::UserDefinedType*>(type)) {
		return userType->getGhidraId();
	}
	
	ObjectHash objHash;
	objHash.addValue(type->getName());
	return objHash.getHash();
}

DataType::Typedef* CE::TypeManager::Factory::createTypedef(const std::string& name, const std::string& desc) {
	auto type = new DataType::Typedef(m_typeManager, name, desc);
	type->setMapper(m_dataTypeMapper);
	type->setGhidraMapper(m_ghidraDataTypeMapper->m_typedefTypeMapper);
	if (m_markAsNew)
		m_typeManager->getProject()->getTransaction()->markAsNew(type);
	return type;
}

DataType::Enum* CE::TypeManager::Factory::createEnum(const std::string& name, const std::string& desc) {
	auto type = new DataType::Enum(m_typeManager, name, desc);
	type->setMapper(m_dataTypeMapper);
	type->setGhidraMapper(m_ghidraDataTypeMapper->m_enumTypeMapper);
	if (m_markAsNew)
		m_typeManager->getProject()->getTransaction()->markAsNew(type);
	return type;
}

DataType::Structure* CE::TypeManager::Factory::createStructure(const std::string& name, const std::string& desc) {
	auto type = new DataType::Structure(m_typeManager, name, desc);
	type->setMapper(m_dataTypeMapper);
	type->setGhidraMapper(m_ghidraDataTypeMapper->m_structureTypeMapper);
	if (m_markAsNew)
		m_typeManager->getProject()->getTransaction()->markAsNew(type);
	return type;
}

DataType::Class* CE::TypeManager::Factory::createClass(const std::string& name, const std::string& desc) {
	auto type = new DataType::Class(m_typeManager, name, desc);
	type->setMapper(m_dataTypeMapper);
	type->setGhidraMapper(m_ghidraDataTypeMapper->m_classTypeMapper);
	if (m_markAsNew)
		m_typeManager->getProject()->getTransaction()->markAsNew(type);
	return type;
}

DataType::FunctionSignature* CE::TypeManager::Factory::createSignature(DataType::FunctionSignature::CallingConvetion callingConvetion, const std::string& name, const std::string& desc) {
	auto type = new DataType::FunctionSignature(m_typeManager, name, desc, callingConvetion);
	type->setMapper(m_typeManager->m_dataTypeMapper);
	type->setGhidraMapper(m_ghidraDataTypeMapper->m_signatureTypeMapper);
	if (m_markAsNew)
		m_typeManager->getProject()->getTransaction()->markAsNew(type);
	return type;
}

DataType::FunctionSignature* CE::TypeManager::Factory::createSignature(const std::string& name, const std::string& desc) {
	return createSignature(DataType::FunctionSignature::FASTCALL, name, desc);
}

DataType::IType* CE::TypeManager::Factory::getDefaultType() {
	return m_typeManager->findTypeById(DataType::SystemType::Byte);
}

DataType::IType* CE::TypeManager::Factory::getDefaultReturnType() {
	return m_typeManager->findTypeById(DataType::SystemType::Void);
}

