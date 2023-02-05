#include "SDA/Core/Context.h"

using namespace sda;

std::string Context::Callbacks::getName() const {
    return "Empty";
}

void Context::Callbacks::onObjectAdded(Object* obj) {
    if (m_prevCallbacks) {
        m_prevCallbacks->onObjectAdded(obj);
    }
    if (m_enabled) {
        onObjectAddedImpl(obj);
    }
}

void Context::Callbacks::onObjectModified(Object* obj) {
    if (m_prevCallbacks) {
        m_prevCallbacks->onObjectModified(obj);
    }
    if (m_enabled) {
        onObjectModifiedImpl(obj);
    }
}

void Context::Callbacks::onObjectRemoved(Object* obj) {
    if (m_prevCallbacks) {
        m_prevCallbacks->onObjectRemoved(obj);
    }
    if (m_enabled) {
        onObjectRemovedImpl(obj);
    }
}

void Context::Callbacks::onContextDestroyed(Context* context) {
    if (m_prevCallbacks) {
        m_prevCallbacks->onContextDestroyed(context);
    }
    if (m_enabled) {
        onContextDestroyedImpl(context);
    }
}

void Context::Callbacks::setPrevCallbacks(std::shared_ptr<Context::Callbacks> callbacks) {
    m_prevCallbacks = callbacks;
}

void Context::Callbacks::setEnabled(bool enabled) {
    m_enabled = enabled;
}

std::shared_ptr<Context::Callbacks> Context::Callbacks::Find(const std::string& name, std::shared_ptr<Context::Callbacks> callbacks) {
    if (callbacks->getName() == name)
        return callbacks;
    if (callbacks->m_prevCallbacks)
        return Find(name, callbacks->m_prevCallbacks);
    return nullptr;
}