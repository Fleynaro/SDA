#pragma once
#include "SDA/Program.h"

namespace sda::bind
{
    class ProgramCallbacksBind
    {
        using Callbacks = Program::Callbacks;
        class CallbacksJsImpl : public Callbacks
        {
        public:
            std::shared_ptr<Callbacks> m_prevCallbacks;
            Callback m_onProjectAdded;
            Callback m_onProjectRemoved;
            
            void onProjectAdded(Project* project) override {
                m_prevCallbacks->onProjectAdded(project);
                if (m_onProjectAdded.isDefined()) {
                    m_onProjectAdded.call(project);
                }
            }

            void onProjectRemoved(Project* project) override {
                m_prevCallbacks->onProjectRemoved(project);
                if (m_onProjectRemoved.isDefined()) {
                    m_onProjectRemoved.call(project);
                }
            }
        };
        
        static auto New() {
            return std::make_shared<CallbacksJsImpl>();
        }

    public:
        static void Init(v8pp::module& module) {
            {
                auto cl = NewClass<Callbacks, v8pp::shared_ptr_traits>(module);
                cl
                    .auto_wrap_object_ptrs(true)
                    .method("onProjectAdded", &Callbacks::onProjectAdded)
                    .method("onProjectRemoved", &Callbacks::onProjectRemoved);
                module.class_("ProgramCallbacks", cl);
            }
            
            {
                auto cl = NewClass<CallbacksJsImpl, v8pp::shared_ptr_traits>(module);
                cl
                    .inherit<Callbacks>()
                    .var("prevCallbacks", &CallbacksJsImpl::m_prevCallbacks)
                    .static_method("New", &New);
                Callback::Register(cl, "onProjectAdded", &CallbacksJsImpl::m_onProjectAdded);
                Callback::Register(cl, "onProjectRemoved", &CallbacksJsImpl::m_onProjectRemoved);
                module.class_("ProgramCallbacksImpl", cl);
            }
        }
    };

    class ProgramBind
    {
        class RefCallbacks : public Program::Callbacks {
            void onProjectAdded(Project* project) override {
                ExportObjectRef(project);
            }

            void onProjectRemoved(Project* project) override {
                RemoveObjectRef(project);
                RemoveObjectRef(project->getContext());
            }
        };
        static auto New() {
            auto program = new Program();
            program->setCallbacks(std::make_shared<RefCallbacks>());
            return ExportObject(program);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Program>(module);
            cl
                .property("callbacks", &Program::getCallbacks, &Program::setCallbacks)
                .property("projects", [](Program& self) { return to_v8(self.getProjects()); })
                .method("removeProject", &Program::removeProject)
                .static_method("New", &New);
            module.class_("Program", cl);
        }
    };
};