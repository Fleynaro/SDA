#include "SDA/Core/IRcode/IRcodeProgram.h"

using namespace sda::ircode;

void Program::Callbacks::onOperationAdded(const Operation* op) {
    if (m_prevCallbacks)
        m_prevCallbacks->onOperationAdded(op);
    if (m_enabled) {
        onOperationAddedImpl(op);
    }
}

void Program::Callbacks::onOperationRemoved(const Operation* op) {
    if (m_prevCallbacks)
        m_prevCallbacks->onOperationRemoved(op);
    if (m_enabled) {
        onOperationRemovedImpl(op);
    }
}

void Program::Callbacks::setPrevCallbacks(std::shared_ptr<Callbacks> prevCallbacks) {
    m_prevCallbacks = prevCallbacks;
}

void Program::Callbacks::setEnabled(bool enabled) {
    m_enabled = enabled;
}
