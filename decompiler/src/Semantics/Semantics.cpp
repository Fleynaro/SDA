#include "Decompiler/Semantics/Semantics.h"
#include "Decompiler/Semantics/SemanticsManager.h"

using namespace sda;
using namespace sda::decompiler;

Semantics::Propagator::Propagator(SemanticsManager* semManager)
    : m_semManager(semManager)
{}

SemanticsManager* Semantics::Propagator::getManager() const {
    return m_semManager;
}

DataType* Semantics::Propagator::findDataType(const std::string& name) const {
    auto dataType = m_semManager->getContext()->getDataTypes()->getByName(name);
    assert(dataType);
    return dataType;
}