#include "SDA/Core/Pcode/PcodeGraph.h"

using namespace sda::pcode;

void Graph::Callbacks::onInstructionAdded(const Instruction* instruction, InstructionOffset nextOffset) {
    if (m_prevCallbacks)
        m_prevCallbacks->onInstructionAdded(instruction, nextOffset);
    if (m_enabled) {
        onInstructionAddedImpl(instruction, nextOffset);
    }
}

void Graph::Callbacks::onInstructionRemoved(const Instruction* instruction) {
    if (m_prevCallbacks)
        m_prevCallbacks->onInstructionRemoved(instruction);
    if (m_enabled) {
        onInstructionRemovedImpl(instruction);
    }
}

void Graph::Callbacks::onBlockCreated(Block* block) {
    if (m_prevCallbacks)
        m_prevCallbacks->onBlockCreated(block);
    if (m_enabled) {
        onBlockCreatedImpl(block);
    }
}

void Graph::Callbacks::onBlockUpdated(Block* block) {
    if (m_prevCallbacks)
        m_prevCallbacks->onBlockUpdated(block);
    if (m_enabled) {
        onBlockUpdatedImpl(block);
    }
}

void Graph::Callbacks::onBlockUpdateRequested(Block* block) {
    if (m_prevCallbacks)
        m_prevCallbacks->onBlockUpdateRequested(block);
    if (m_enabled) {
        onBlockUpdateRequestedImpl(block);
    }
}

void Graph::Callbacks::onBlockFunctionGraphChanged(Block* block, FunctionGraph* oldFunctionGraph, FunctionGraph* newFunctionGraph) {
    if (m_prevCallbacks)
        m_prevCallbacks->onBlockFunctionGraphChanged(block, oldFunctionGraph, newFunctionGraph);
    if (m_enabled) {
        onBlockFunctionGraphChangedImpl(block, oldFunctionGraph, newFunctionGraph);
    }
}

void Graph::Callbacks::onBlockRemoved(Block* block) {
    if (m_prevCallbacks)
        m_prevCallbacks->onBlockRemoved(block);
    if (m_enabled) {
        onBlockRemovedImpl(block);
    }
}

void Graph::Callbacks::onFunctionGraphCreated(FunctionGraph* functionGraph) {
    if (m_prevCallbacks)
        m_prevCallbacks->onFunctionGraphCreated(functionGraph);
    if (m_enabled) {
        onFunctionGraphCreatedImpl(functionGraph);
    }
}

void Graph::Callbacks::onFunctionGraphRemoved(FunctionGraph* functionGraph) {
    if (m_prevCallbacks)
        m_prevCallbacks->onFunctionGraphRemoved(functionGraph);
    if (m_enabled) {
        onFunctionGraphRemovedImpl(functionGraph);
    }
}

void Graph::Callbacks::onUnvisitedOffsetFound(InstructionOffset offset) {
    if (m_prevCallbacks)
        m_prevCallbacks->onUnvisitedOffsetFound(offset);
    if (m_enabled) {
        onUnvisitedOffsetFoundImpl(offset);
    }
}

void Graph::Callbacks::onCommitStarted() {
    if (m_prevCallbacks)
        m_prevCallbacks->onCommitStarted();
    if (m_enabled) {
        onCommitStartedImpl();
    }
}

void Graph::Callbacks::onCommitEnded() {
    if (m_prevCallbacks)
        m_prevCallbacks->onCommitEnded();
    if (m_enabled) {
        onCommitEndedImpl();
    }
}

void Graph::Callbacks::onWarningEmitted(const std::string& warning) {
    if (m_prevCallbacks)
        m_prevCallbacks->onWarningEmitted(warning);
    if (m_enabled) {
        onWarningEmittedImpl(warning);
    }
}

void Graph::Callbacks::setPrevCallbacks(std::shared_ptr<Callbacks> prevCallbacks) {
    m_prevCallbacks = prevCallbacks;
}

void Graph::Callbacks::setEnabled(bool enabled) {
    m_enabled = enabled;
}
