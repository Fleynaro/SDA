#include "TypeManager.h"
#include "database/Mappers/DataTypeMapper.h"

using namespace CE;
using namespace DataType;

TypeManager::TypeManager(Project* module)
	: AbstractItemManager(module)
{
	m_dataTypeMapper = new DB::DataTypeMapper(this);
	addSystemTypes();
	addGhidraTypedefs();
	m_defSignature = getFactory(false).createSignature(DataType::CallingConvetion::FASTCALL, "defSignature");
}

TypeManager::~TypeManager() {
	delete m_dataTypeMapper;
}

TypeManager::Factory TypeManager::getFactory(bool markAsNew) {
	return Factory(this, m_dataTypeMapper, markAsNew);
}

void TypeManager::addSystemTypes() {
	m_items.insert({
		std::make_pair(SystemType::Void, new Void),
		std::make_pair(SystemType::Bool, new Bool),
		std::make_pair(SystemType::Byte, new Byte),
		std::make_pair(SystemType::Int8, new Int8),
		std::make_pair(SystemType::Int16, new Int16),
		std::make_pair(SystemType::Int32, new Int32),
		std::make_pair(SystemType::Int64, new Int64),
		std::make_pair(SystemType::UInt16, new UInt16),
		std::make_pair(SystemType::UInt32, new UInt32),
		std::make_pair(SystemType::UInt64, new UInt64),
		std::make_pair(SystemType::UInt128, new UInt128),
		std::make_pair(SystemType::Float, new Float),
		std::make_pair(SystemType::Double, new Double),
		std::make_pair(SystemType::Char, new Char),
		std::make_pair(SystemType::WChar, new WChar)
		});

	Iterator it(this);
	while (it.hasNext()) {
		auto type = it.next();
		type->setTypeManager(this);
	}
}

void TypeManager::addGhidraTypedefs() {
	static std::pair<std::string, DB::Id> typedefs[] = {
		std::make_pair("unicode", SystemType::Void),
		std::make_pair("string", SystemType::Void),
		std::make_pair("IMAGE_RICH_HEADER", SystemType::Void),

		std::make_pair("uchar", SystemType::Byte),
		std::make_pair("uint8_t", SystemType::Byte),
		std::make_pair("undefined1", SystemType::Int8),
		std::make_pair("TerminatedCString", SystemType::Char),

		std::make_pair("short", SystemType::Int16),
		std::make_pair("ushort", SystemType::UInt16),
		std::make_pair("word", SystemType::Int16),
		std::make_pair("undefined2", SystemType::Int16),

		std::make_pair("int", SystemType::Int32),
		std::make_pair("uint", SystemType::UInt32),
		std::make_pair("long", SystemType::Int32),
		std::make_pair("ulong", SystemType::UInt32),
		std::make_pair("dword", SystemType::Int32),
		std::make_pair("ImageBaseOffset32", SystemType::UInt32),
		std::make_pair("undefined4", SystemType::Int32),

		std::make_pair("longlong", SystemType::Int64),
		std::make_pair("ulonglong", SystemType::UInt64),
		std::make_pair("qword", SystemType::Int64),
		std::make_pair("undefined8", SystemType::Int64),

		std::make_pair("GUID", SystemType::UInt128)
	};

	DB::Id startId = 100;
	for (const auto& it : typedefs) {
		auto type = new Typedef(this, it.first);
		type->setTypeManager(this);
		type->setId(startId);
		type->setRefType(GetUnit(findTypeById(it.second)));
		m_items.insert(std::make_pair(startId, type));
		startId++;
	}
}

void TypeManager::loadBefore() const
{
	m_dataTypeMapper->loadBefore();
}

void TypeManager::loadAfter() const
{
	m_dataTypeMapper->loadAfter();
}

DataTypePtr TypeManager::getType(DB::Id id) {
	return GetUnit(findTypeById(id));
}

DataTypePtr TypeManager::getDefaultType(int size, bool sign, bool floating) {
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

DataTypePtr TypeManager::calcDataTypeForNumber(uint64_t value) {
	if ((value & ~static_cast<uint64_t>(0xFFFFFFFF)) == static_cast<uint64_t>(0x0))
		return getType(SystemType::Int32);
	return getType(SystemType::Int64);
}

IFunctionSignature* TypeManager::getDefaultFuncSignature() const {
	return m_defSignature;
}

IType* TypeManager::findTypeById(DB::Id id) {
	return dynamic_cast<AbstractType*>(find(id));
}

IType* TypeManager::findTypeByName(const std::string& typeName)
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

Typedef* TypeManager::Factory::createTypedef(const std::string& name, const std::string& desc) const
{
	auto type = new Typedef(m_typeManager, name, desc);
	type->setMapper(m_dataTypeMapper);
	if (m_markAsNew)
		m_typeManager->getProject()->getTransaction()->markAsNew(type);
	return type;
}

Enum* TypeManager::Factory::createEnum(const std::string& name, const std::string& desc) const
{
	auto type = new Enum(m_typeManager, name, desc);
	type->setMapper(m_dataTypeMapper);
	if (m_markAsNew)
		m_typeManager->getProject()->getTransaction()->markAsNew(type);
	return type;
}

Structure* TypeManager::Factory::createStructure(const std::string& name, const std::string& desc) const
{
	auto type = new Structure(m_typeManager, name, desc);
	type->setMapper(m_dataTypeMapper);
	if (m_markAsNew)
		m_typeManager->getProject()->getTransaction()->markAsNew(type);
	return type;
}

FunctionSignature* TypeManager::Factory::createSignature(CallingConvetion callingConvetion, const std::string& name, const std::string& desc) const
{
	auto type = new FunctionSignature(m_typeManager, name, desc, callingConvetion);
	type->setMapper(m_typeManager->m_dataTypeMapper);
	if (m_markAsNew)
		m_typeManager->getProject()->getTransaction()->markAsNew(type);
	return type;
}

FunctionSignature* TypeManager::Factory::createSignature(const std::string& name, const std::string& desc) const
{
	return createSignature(CallingConvetion::FASTCALL, name, desc);
}

IType* TypeManager::Factory::getDefaultType() const
{
	return m_typeManager->findTypeById(SystemType::Byte);
}

IType* TypeManager::Factory::getDefaultReturnType() const
{
	return m_typeManager->findTypeById(SystemType::Void);
}

