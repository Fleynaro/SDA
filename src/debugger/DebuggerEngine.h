#pragma once
#include <Windows.h>
#include <dbgeng.h>
#include <atlcomcli.h>
#include "DebugSession.h"
#include <map>
#include <mutex>
#include <thread>
#include <magic_enum.hpp>

#pragma comment(lib,"Ole32.lib")
#pragma comment(lib,"dbgeng.lib")

#define GET_HR_ERROR(hr)    -((hr) & 0xffffff)

namespace CE
{
	class DebuggerEngineException : public DebugException
	{
		long m_errorCode;
	public:
		DebuggerEngineException(const std::string& message = "", long errorCode = 0)
			: DebugException(message), m_errorCode(errorCode)
		{}
	};

    class DebuggerEngineSession;
    class DebuggerEngineEventCallbacks : public DebugBaseEventCallbacks
    {
        DebuggerEngineSession* m_debugSession;
    public:
        DebuggerEngineEventCallbacks(DebuggerEngineSession* debugSession)
            : m_debugSession(debugSession)
        { }

    	virtual ~DebuggerEngineEventCallbacks() {
            Release();
        }

        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    	
        ULONG STDMETHODCALLTYPE Release() override { return 0; }

        HRESULT Breakpoint(PDEBUG_BREAKPOINT p) override {
            return DEBUG_STATUS_BREAK;
        }

        HRESULT ChangeDebuggeeState(ULONG flags, ULONG64 arg) override {
            printf("ChangeDebuggeeState flags[0x%lx] arg[0x%llx]\n", flags, arg);
            return DEBUG_STATUS_NO_CHANGE;
        }

        HRESULT ChangeEngineState(ULONG flags, ULONG64 arg) override {
            printf("ChangeEngineState flags [0x%lx] arg[0x%llx]\n", flags, arg);
            return DEBUG_STATUS_NO_CHANGE;
        }

        HRESULT ChangeSymbolState(ULONG flags, ULONG64 arg) override {
            printf("ChangeSymbolState flags [0x%lx] arg[0x%llx]\n", flags, arg);
            return DEBUG_STATUS_NO_CHANGE;
        }

        HRESULT CreateProcess(ULONG64 imghdl, ULONG64 hdl, ULONG64 baseoff, ULONG modsize,
                              PCSTR modname, PCSTR imgname, ULONG chksum, ULONG timestamp, ULONG64 initthrhdl,
                              ULONG64 thrdataoff,
                              ULONG64 startoff) override;

        HRESULT CreateThread(ULONG64 hdl, ULONG64 dataoff, ULONG64 startoff) override {
            printf("CreateThread hdl[0x%llx] dataoff[0x%llx] startoff[0x%llx]",
                hdl, dataoff, startoff);
            return DEBUG_STATUS_NO_CHANGE;
        }

        HRESULT Exception(PEXCEPTION_RECORD64 pexp, ULONG firstchance) override {
            printf("Exception pexp[%p] firstchance [0x%lx]", pexp, firstchance);
            return DEBUG_STATUS_NO_CHANGE;
        }

        HRESULT ExitProcess(ULONG exitcode) override {
            printf("ExitProcess exitcode [0x%lx]", exitcode);
            return DEBUG_STATUS_NO_CHANGE;
        }

        HRESULT ExitThread(ULONG exitcode) override {
            printf("ExitThread exitcode [0x%lx]", exitcode);
            return DEBUG_STATUS_NO_CHANGE;
        }

        HRESULT GetInterestMask(ULONG* pmask) override {
            *pmask = (DEBUG_EVENT_BREAKPOINT |
                DEBUG_EVENT_EXCEPTION |
                DEBUG_EVENT_CREATE_THREAD |
                DEBUG_EVENT_EXIT_THREAD |
                DEBUG_EVENT_CREATE_PROCESS |
                DEBUG_EVENT_EXIT_PROCESS |
                DEBUG_EVENT_LOAD_MODULE |
                DEBUG_EVENT_UNLOAD_MODULE |
                DEBUG_EVENT_SYSTEM_ERROR |
                DEBUG_EVENT_SESSION_STATUS |
                DEBUG_EVENT_CHANGE_DEBUGGEE_STATE |
                DEBUG_EVENT_CHANGE_ENGINE_STATE |
                DEBUG_EVENT_CHANGE_SYMBOL_STATE);
            return S_OK;
        }

        HRESULT LoadModule(ULONG64 imghdl, ULONG64 baseoff, ULONG modsize, PCSTR modname, PCSTR imgname, ULONG chksum,
                           ULONG timestamp) override;

        HRESULT SessionStatus(ULONG status) override {
            printf("SessionStatus status[0x%lx]", status);
            return DEBUG_STATUS_NO_CHANGE;
        }

        HRESULT SystemError(ULONG error, ULONG level) override {
            printf("SystemError error[0x%lx] level[0x%lx]", error, level);
            return DEBUG_STATUS_NO_CHANGE;
        }

        HRESULT UnloadModule(PCSTR imgname, ULONG64 baseoff) override {
            printf("UnloadModule imgname[%s] baseoff[0x%llx]", imgname, baseoff);
            return DEBUG_STATUS_NO_CHANGE;
        }
    };

    class DebuggerEngineIOCallbacks : public IDebugInputCallbacks, public IDebugOutputCallbacks
    {
    public:
        DebuggerEngineIOCallbacks() {}

        virtual ~DebuggerEngineIOCallbacks() {
            Release();
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(const IID& InterfaceId, PVOID* Interface) override {
            if (IsEqualIID(InterfaceId, __uuidof(IDebugInputCallbacks)))
            {
                *Interface = (IDebugInputCallbacks*)this;
                AddRef();
                return S_OK;
            }
            if (IsEqualIID(InterfaceId, __uuidof(IDebugOutputCallbacks)))
            {
                *Interface = (IDebugOutputCallbacks*)this;
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }

        ULONG STDMETHODCALLTYPE Release() override { return 0; }

        HRESULT Output(ULONG mask, PCSTR text) override {
            fprintf(stdout, "Output[0x%lx]%s", mask, text);
            return S_OK;
        }

        HRESULT EndInput() override {
            printf("EndInput");
            return S_OK;
        }

        HRESULT StartInput(ULONG bufsize) override {
            printf("StartInput bufsize[0x%lx]", bufsize);
            return S_OK;
        }
    };
	
	class DebuggerEngineSession : public IDebugSession
	{
        friend class DebuggerEngineEventCallbacks;
        friend class DebuggerEngineIOCallbacks;
		
		IDebugClient* m_client = nullptr;
		IDebugControl3* m_control = nullptr;
		IDebugRegisters2* m_registers = nullptr;
        IDebugSymbols3* m_symbols = nullptr;
        IDebugDataSpaces* m_dataSpaces = nullptr;
        IDebugDataSpaces4* m_dataSpaces4 = nullptr;

        DebuggerEngineEventCallbacks* m_eventCallbacks = nullptr;
        DebuggerEngineIOCallbacks* m_IOCallbacks = nullptr;

        enum class State
        {
            NONE,
        	START,
        	RUN,
            SUSPEND,
            STEP_OVER,
            STEP_INTO,
            RESUME,
        	STOP
        };

        State m_state = State::NONE;

        DebugProcess m_process;
        std::list<DebugModule> m_modules;
        std::map<std::uintptr_t, PDEBUG_BREAKPOINT> m_breakpoints;
        std::thread m_eventLoopThread;
        DebuggerEngineException m_exception;
        std::mutex m_mutex;
        std::vector<std::string> m_log;
	public:
		DebuggerEngineSession()
		{}

		~DebuggerEngineSession() {
            stop();
            while (m_state != State::NONE)
                Sleep(100);
		}

        Debugger getDebugger() override {
            return Debugger::DebuggerEngine;
		}
		
		void attachProcess(const DebugProcess& process) override {
            m_process = process;
            runSession();
		}

		DebugProcess getProcess() override {
            return m_process;
		}

		void stepOver() override {
            m_state = State::STEP_OVER;
		}

        void stepInto() override {
            m_state = State::STEP_INTO;
        }

		void pause(bool wait) override {
            m_log.emplace_back("pause");
            if (m_state == State::SUSPEND)
                return;
			// wait for run state
            if (m_state != State::RUN) {
                while (m_state != State::RUN)
                    Sleep(100);
            }
			// wait for suspend state
            interrupt(DEBUG_INTERRUPT_ACTIVE);
            if (wait) {
                while (m_state != State::SUSPEND)
                    Sleep(100);
            }
        }

        void resume() override {
            m_log.emplace_back("resume");
            if (m_state == State::RUN)
                return;
            m_state = State::RESUME;
        }

		void stop() override {
            m_state = State::STOP;
            interrupt(DEBUG_INTERRUPT_EXIT);
		}

        bool isSuspended() override {
            return m_state == State::SUSPEND;
		}

        bool isWorking() override {
            return m_state != State::NONE && m_state != State::STOP;
		}

        void addBreakpoint(std::uintptr_t address) override {
			// breakpoint can be added while the debugger is suspended
            pause(true);
			
            PDEBUG_BREAKPOINT bp = nullptr;
			const auto hr = m_control->AddBreakpoint(DEBUG_BREAKPOINT_CODE, DEBUG_ANY_ID, &bp);
            Check(hr, "AddBreakpoint error");
            bp->SetOffset(address);
            bp->AddFlags(DEBUG_BREAKPOINT_ENABLED);
            m_breakpoints[address] = bp;

            resume();
		}

        void removeBreakpoint(std::uintptr_t address) override {
            const auto it = m_breakpoints.find(address);
            if (it == m_breakpoints.end())
                return;
            const auto bp = it->second;
            const auto hr = m_control->RemoveBreakpoint(bp);
            Check(hr, "RemoveBreakpoint error");
            m_breakpoints.erase(it);
		}

		std::list<DebugModule> getModules() override {
            m_mutex.lock();
            const auto modules = m_modules;
            m_mutex.unlock();
            return modules;
		}

		void readMemory(uint64_t offset, std::vector<uint8_t>& data) override {
            // memory can be read while the debugger is suspended
            ULONG bytesRead;
            const auto hr = m_dataSpaces->ReadVirtual(offset, data.data(), static_cast<ULONG>(data.size()), &bytesRead);
            Check(hr, "ReadVirtual error");
            if (bytesRead != data.size()) {
                throw DebugException("not all bytes has read");
            }
		}

		std::uintptr_t getInstructionAddress() override {
            ULONG64 offset;
            const auto hr = m_registers->GetInstructionOffset(&offset);
            Check(hr, "GetInstructionOffset error");
            return offset;
		}

        std::list<DebugRegister> getRegisters() override {
            return {};
		}
	
	private:
        void runSession() {
            m_state = State::START;
            m_eventLoopThread = std::thread([&]()
                {
                    try {
                        // init
                        createClient();

                        // search
                        ULONG processId;
                        foundProcessByName(m_process.m_name, processId);

                        // attach
                        const auto hr = m_client->AttachProcess(NULL, processId, DEBUG_ATTACH_DEFAULT);
                        Check(hr, "process attachment failed");

                        while (true)
                        {
                        	// the target always runs until WaitForEvent completed
                            m_state = State::RUN;
                            m_log.emplace_back("State: RUN");
                        	
                        	// wait for some event (breakpoint hit, user interrupt, ...)
                            auto hr = m_control->WaitForEvent(0, INFINITE);
                            if (hr == E_PENDING)
                                break;
                            Check(hr, "WaitForEvent error");

                        	// get status
                            ULONG status;
                            hr = m_control->GetExecutionStatus(&status);
                            Check(hr, "GetExecutionStatus error");

                            m_log.emplace_back("New status: " + std::to_string(status));
                        	
                            if (status == DEBUG_STATUS_BREAK) {
                            	// for any break status event the debugger is suspended
                                m_state = State::SUSPEND;
                                while (m_state == State::SUSPEND)
                                    Sleep(100);
                                m_log.emplace_back(std::string("State after suspend: ") + magic_enum::enum_name(m_state).data());
                            	
                            	// set new status
                                ULONG nextStatus = DEBUG_STATUS_GO;
                                if (m_state == State::STEP_OVER)
                                    nextStatus = DEBUG_STATUS_STEP_OVER;
                                else if (m_state == State::STEP_INTO)
                                    nextStatus = DEBUG_STATUS_STEP_INTO;
                                else if (m_state == State::RESUME)
                                    nextStatus = DEBUG_STATUS_GO;
                                else if (m_state == State::STOP)
                                    break;

                                hr = m_control->SetExecutionStatus(nextStatus);
                                Check(hr, "SetExecutionStatus error");
                            }
                        }
                    } catch(DebuggerEngineException& ex) {
                        m_exception = ex;
                    }

                    m_log.emplace_back("END");
                    m_state = State::NONE;
                    freeResources();
                });
            m_eventLoopThread.detach();
        }

        void interrupt(ULONG flags) const {
            HRESULT hr;
            int iTryCount = 0;
            do
            {
                hr = m_control->SetInterrupt(flags);
                if (SUCCEEDED(hr))
                    break;
                Sleep(100);
            } while (iTryCount++ < 10);

            if (FAILED(hr))
                Check(hr, "SetInterrupt error");
        }
		
		void foundProcessByName(const std::string& name, ULONG& id) const {
			const auto hr = m_client->GetRunningProcessSystemIdByExecutableName(NULL, name.c_str(), 0, &id);
			Check(hr, "process found failed");
		}
		
		void createClient() {
			auto hr = DebugCreate(__uuidof(IDebugClient), reinterpret_cast<void**>(&m_client));
			Check(hr, "DebugCreate error");

            hr = m_client->QueryInterface(__uuidof(IDebugControl3), (void**)&m_control);
            Check(hr, "cannot query interface IDebugControl3");

            hr = m_client->QueryInterface(__uuidof(IDebugRegisters2), (void**)&m_registers);
            Check(hr, "cannot query interface IDebugRegisters2");

            hr = m_client->QueryInterface(__uuidof(IDebugSymbols3), (void**)&m_symbols);
            Check(hr, "cannot query interface IDebugSymbols3");

            hr = m_client->QueryInterface(__uuidof(IDebugDataSpaces), (void**)&m_dataSpaces);
            Check(hr, "cannot query interface IDebugDataSpaces");

            hr = m_client->QueryInterface(__uuidof(IDebugDataSpaces4), (void**)&m_dataSpaces4);
            Check(hr, "cannot query interface IDebugDataSpaces4");

            initCallbacks();

			const auto outmask =
                DEBUG_OUTPUT_NORMAL |
                DEBUG_OUTPUT_ERROR |
                DEBUG_OUTPUT_WARNING |
                DEBUG_OUTPUT_VERBOSE |
                DEBUG_OUTPUT_PROMPT |
                DEBUG_OUTPUT_PROMPT_REGISTERS |
                DEBUG_OUTPUT_EXTENSION_WARNING |
                DEBUG_OUTPUT_DEBUGGEE |
                DEBUG_OUTPUT_DEBUGGEE_PROMPT |
                DEBUG_OUTPUT_SYMBOLS |
                DEBUG_OUTPUT_STATUS;
            setMask(outmask);
		}

        void initCallbacks() {
            m_eventCallbacks = new DebuggerEngineEventCallbacks(this);
            auto hr = m_client->SetEventCallbacks(m_eventCallbacks);
            Check(hr, "SetEventCallbacks error");

            m_IOCallbacks = new DebuggerEngineIOCallbacks();
            hr = m_client->SetInputCallbacks(m_IOCallbacks);
            Check(hr, "SetInputCallbacks error");
			
            hr = m_client->SetOutputCallbacks(m_IOCallbacks);
            Check(hr, "SetOutputCallbacks error");
        }

		void setMask(ULONG mask) const {
            const auto hr = m_client->SetOutputMask(mask);
            Check(hr, "cannot set mask");
		}

		void freeResources() const {
            if (m_client) {
                m_client->DetachProcesses();
            }
        	
            if (m_eventCallbacks) {
                m_client->SetEventCallbacks(nullptr);
                delete m_eventCallbacks;
            }
            if (m_IOCallbacks) {
                m_client->SetInputCallbacks(nullptr);
                m_client->SetOutputCallbacks(nullptr);
                delete m_IOCallbacks;
            }

            if (m_client) {
                m_client->Release();
            }
            if (m_control) {
                m_control->Release();
            }
            if (m_registers) {
                m_registers->Release();
            }
            if (m_symbols) {
                m_symbols->Release();
            }
            if (m_dataSpaces) {
                m_dataSpaces->Release();
            }
            if (m_dataSpaces4) {
                m_dataSpaces4->Release();
            }
        }
		
		static void Check(HRESULT hr, const std::string& message) {
			if (hr != S_OK) {
				const auto codeError = GET_HR_ERROR(hr);
				throw DebuggerEngineException(message + " ["+ std::to_string(codeError) +"]", codeError);
			}
		}
	};
	
};