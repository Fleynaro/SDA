#include "SDA/Core/IRcode/IRcodeProgram.h"

using namespace sda::ircode;

void Program::Callbacks::onOperationAdded(const Operation* op, Block* block) {
    if (m_prevCallbacks)
        m_prevCallbacks->onOperationAdded(op, block);
    if (m_enabled) {
        onOperationAddedImpl(op, block);
    }
}

void Program::Callbacks::onOperationRemoved(const Operation* op, Block* block) {
    if (m_prevCallbacks)
        m_prevCallbacks->onOperationRemoved(op, block);
    if (m_enabled) {
        onOperationRemovedImpl(op, block);
    }
}

void Program::Callbacks::setPrevCallbacks(std::shared_ptr<Callbacks> prevCallbacks) {
    m_prevCallbacks = prevCallbacks;
}

void Program::Callbacks::setEnabled(bool enabled) {
    m_enabled = enabled;
}
