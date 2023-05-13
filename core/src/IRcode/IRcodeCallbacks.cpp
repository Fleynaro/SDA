#include "SDA/Core/IRcode/IRcodeProgram.h"

using namespace sda::ircode;

void Program::Callbacks::onBlockCreated(Block* block) {
    if (m_prevCallbacks)
        m_prevCallbacks->onBlockCreated(block);
    if (m_enabled) {
        onBlockCreatedImpl(block);
    }
}

void Program::Callbacks::onBlockDecompiled(Block* block) {
    if (m_prevCallbacks)
        m_prevCallbacks->onBlockDecompiled(block);
    if (m_enabled) {
        onBlockDecompiledImpl(block);
    }
}

void Program::Callbacks::onBlockRemoved(Block* block) {
    if (m_prevCallbacks)
        m_prevCallbacks->onBlockRemoved(block);
    if (m_enabled) {
        onBlockRemovedImpl(block);
    }
}

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
