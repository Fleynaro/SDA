#include "TypeUnit.h"
#include "SystemType.h"
#include "Typedef.h"
#include <utilities/Helper.h>
#include <map>

using namespace CE;
using namespace CE::DataType;

Unit::Unit(DataType::IType* type, std::list<int> levels)
	: m_type(type), m_levels(levels)
{}

Unit::Group Unit::getGroup() {
	return m_type->getGroup();
}

bool Unit::isUserDefined() {
	return m_type->isUserDefined();
}

bool Unit::isFloatingPoint() {
	if (isPointer())
		return false;
	if (auto sysType = dynamic_cast<SystemType*>(getBaseType(true, false))) {
		return sysType->getSet() == SystemType::Real;
	}
	return false;
}

int Unit::getPointerLvl() {
	return static_cast<int>(getPointerLevels().size());
}

bool Unit::isArray() {
	auto ptrLevels = getPointerLevels();
	if (ptrLevels.empty())
		return false;
	return *ptrLevels.begin() >= 2;
}

bool Unit::isPointer() {
	auto ptrLevels = getPointerLevels();
	if (ptrLevels.empty())
		return false;
	return *ptrLevels.begin() == 1;
}

std::list<int> Unit::getPointerLevels() {
	if (const auto Typedef = dynamic_cast<DataType::Typedef*>(m_type)) {
		std::list<int> result = Typedef->getRefType()->getPointerLevels();
		result.insert(result.begin(), m_levels.begin(), m_levels.end());
		return result;
	}
	return m_levels;
}

void Unit::addPointerLevelInFront(int size) {
	m_levels.push_front(size);
}

void Unit::removePointerLevelOutOfFront() {
	m_levels.pop_front();
}

bool Unit::isString() {
	if (!isPointer())
		return false;
	const auto baseType = getBaseType();
	return dynamic_cast<Char*>(baseType) || dynamic_cast<WChar*>(baseType);
}

bool Unit::equal(DataType::Unit* typeUnit) {
	const auto baseType1 = getBaseType();
	const auto baseType2 = typeUnit->getBaseType();
	auto sysType1 = dynamic_cast<SystemType*>(baseType1);
	auto sysType2 = dynamic_cast<SystemType*>(baseType2);
	if (sysType1 && sysType2) {
		return sysType1->getSize() == sysType2->getSize() && sysType1->isSigned() == sysType2->isSigned();
	}
	if (baseType1 != baseType2)
		return false;
	return EqualPointerLvls(getPointerLevels(), typeUnit->getPointerLevels());
}

int Unit::getPriority() {
	auto baseType = getBaseType();
	const auto size = std::min(baseType->getSize(), 0x8);
	const bool hasPointerLvl = getPointerLvl() != 0;
	const bool isSigned = baseType->isSigned();
	const bool isNotSimple = baseType->getGroup() != Simple;
	bool isFloatingPoint = false;
	if (auto sysType = dynamic_cast<SystemType*>(baseType))
		isFloatingPoint = (sysType->getSet() == SystemType::Real);
	return size | (hasPointerLvl << 3) | (isSigned << 4) | (isNotSimple << 5) | (isFloatingPoint << 6);
}

int Unit::getConversionPriority() {
	if (isPointer())
		return 10;
	const auto baseType = getBaseType();
	if (auto systemType = dynamic_cast<SystemType*>(baseType)) {
		static std::map<SystemType::Types, int> typesInOrder = {
			std::pair(SystemType::Double, 5),
			std::pair(SystemType::Float, 4),
			std::pair(SystemType::UInt64, 3),
			std::pair(SystemType::Int64, 2),
			std::pair(SystemType::UInt32, 1)
		};

		const auto it = typesInOrder.find(systemType->getTypeId());
		if (it != typesInOrder.end())
			return it->second;
		return 0;
	}
	return -1;
}

const std::string Unit::getName() {
	return m_type->getName();
}

const std::string Unit::getComment() {
	return m_type->getComment();
}

void Unit::setName(const std::string& name)
{
	m_type->setName(name);
}

void Unit::setComment(const std::string& comment)
{
	m_type->setComment(comment);
}

std::string Unit::getDisplayName() {
	auto name = m_type->getDisplayName();
	for (auto level : m_levels) {
		name += (level == 1 ? "*" : ("[" + std::to_string(level) + "]"));
	}
	return name;
}

int Unit::getSize() {
	auto ptrLevels = getPointerLevels();
	if(ptrLevels.empty())
		return m_type->getSize();
	auto mulDim = 1;
	for (auto lvl : ptrLevels) {
		if (lvl == 1)
			return mulDim * 0x8;
		mulDim *= lvl;
	}
	return mulDim * m_type->getSize();
}

std::string DataType::Unit::getViewValue(uint64_t value) {
	return m_type->getViewValue(value);
}

DataType::IType* Unit::getType() const
{
	return m_type;
}

bool CE::DataType::Unit::EqualPointerLvls(const std::list<int>& ptrList1, const std::list<int>& ptrList2) {
	if (ptrList1.size() != ptrList2.size())
		return false;
	auto it1 = ptrList1.begin();
	auto it2 = ptrList2.begin();
	while (it1 != ptrList1.end()) {
		if (*it1 != *it2)
			return false;
		it1++;
		it2++;
	}
	return true;
}

/*
	arr					<=>			arr
	arr*				<=>			arr[1]
	arr*[20][5]			<=>			arr[20][5][1]
	(arr[2])*			<=>			arr[1][2]
	(arr**[5])*			<=>			arr[1][5][1][1]
	((arr[5])*[10])*	<=>			arr[1][10][1][5]
*/
std::list<int> CE::DataType::ParsePointerLevelsStr(const std::string& str) {
	std::list<int> result;
	std::list<int> seq;

	int lastClosedSquareBracketIdx = 0;
	int idx = static_cast<int>(str.length()) - 1;
	while (idx >= 0) {
		const auto ch = str[idx];

		if (lastClosedSquareBracketIdx != 0) {
			if (ch == '[') {
				auto arrSize = std::stoi(str.substr(idx + 1, lastClosedSquareBracketIdx - idx - 1));
				seq.push_front(arrSize);

				if (idx != 0 && str[idx - 1] == ']') {
					lastClosedSquareBracketIdx = idx - 1;
				}
				else {
					result.insert(result.end(), seq.begin(), seq.end());
					lastClosedSquareBracketIdx = 0;
					seq.clear();
				}
			}
		}
		else {
			if (ch == '*') {
				result.push_back(1);
			}
			else if (ch == ']') {
				lastClosedSquareBracketIdx = idx;
			}
		}

		idx--;
	}

	return result;
}

DataTypePtr CE::DataType::CloneUnit(DataTypePtr dataType) {
	return GetUnit(dataType->getType(), GetPointerLevelStr(dataType));
}

DataTypePtr CE::DataType::MakePointer(DataTypePtr dataType) {
	auto pointerDataType = DataType::CloneUnit(dataType);
	pointerDataType->addPointerLevelInFront();
	return pointerDataType;
}

std::string CE::DataType::GetPointerLevelStr(DataTypePtr type) {
	std::string result = "";
	for (auto arrSize : type->getPointerLevels()) {
		result = result + "["+ std::to_string(arrSize) +"]";
	}
	return result;
}

DataTypePtr CE::DataType::GetUnit(DataType::IType* type, const std::string& levels) {
	const auto levels_list = ParsePointerLevelsStr(levels);
	return GetUnit(type, levels_list);
}

DataTypePtr CE::DataType::GetUnit(DataType::IType* type, const std::list<int>& levels_list) {
	return std::make_shared<DataType::Unit>(type, levels_list);
}
