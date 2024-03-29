#pragma once
#include <utility>
#include "ImageDecorator.h"
#include "controllers/AddressSpaceManagerController.h"
#include "datatypes/Enum.h"
#include "datatypes/Structure.h"
#include "datatypes/SystemType.h"
#include "datatypes/Typedef.h"
#include "debugger/Debug.h"
#include "decompiler/Graph/DecCodeGraph.h"
#include "decompiler/PCode/DecPCodeInstructionPool.h"
#include "decompiler/PCode/DecPCodeVirtualMachine.h"
#include "decompiler/PCode/DecRegisterFactory.h"
#include "decompiler/Graph/DecPCodeGraph.h"
#include "decompiler/Graph/DecCodeGraphBlock.h"
#include "images/PEImage.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/List.h"
#include "imgui_wrapper/controls/Text.h"
#include "managers/AddressSpaceManager.h"
#include "managers/ImageManager.h"
#include "managers/SymbolTableManager.h"
#include "panels/BuiltinInputPanel.h"
#include "utilities/Helper.h"

namespace GUI
{
	class DebuggerAttachProcessPanel : public AbstractPanel
	{
		class DebugProcessListModel : public IListModel<CE::DebugProcess*>
		{
			std::list<CE::DebugProcess>* m_list;
		public:
			std::string m_filterName;
			
			DebugProcessListModel(std::list<CE::DebugProcess>* list)
				: m_list(list)
			{}

		private:
			class DebugProcessIterator : public Iterator
			{
				DebugProcessListModel* m_model;
				std::list<CE::DebugProcess>::iterator m_it;
			public:
				DebugProcessIterator(DebugProcessListModel* model)
					: m_model(model), m_it(model->m_list->begin())
				{}

				void getNextItem(std::string* text, CE::DebugProcess** data) override {
					if (m_it->m_name.find(m_model->m_filterName) != std::string::npos) {
						*text = std::to_string(m_it->m_id) + "," + m_it->m_name;
						*data = &*m_it;
					}
					++m_it;
				}

				bool hasNextItem() override {
					return m_it != m_model->m_list->end();
				}
			};

			void newIterator(const IteratorCallback& callback) override
			{
				DebugProcessIterator iterator(this);
				callback(&iterator);
			}
		};

		std::string m_selectedDebuggerStr;
		std::string m_selectedParentAddrSpaceStr;
		CE::Debugger m_selectedDebugger;
		CE::DebugProcess* m_selectedProcess = nullptr;
		std::list<CE::DebugProcess> m_debugProcesses;
		DebugProcessListModel m_processListModel;
		Input::TextInput m_textInput;
		SelectableTableListView<CE::DebugProcess*> m_tableProcessListView;
		AddressSpaceManagerController m_addrSpaceController;
		StdListView<CE::AddressSpace*> m_addrSpaceListView;

		EventHandler<> m_selectProcessEventHandler;
	public:
		CE::AddressSpace* m_selectedParentAddrSpace = nullptr;
		CE::IDebugSession* m_debugSession = nullptr;

		DebuggerAttachProcessPanel(CE::Project* project);

		void selectProcessEventHandler(const std::function<void()>& handler) {
			m_selectProcessEventHandler = handler;
		}

	private:
		void renderPanel() override {
			Text::Text("Select debugger:").show();
			if (ImGui::BeginCombo("##combo_process", m_selectedDebuggerStr.c_str())) {
				for(const auto debugger : CE::GetAvailableDebuggers()) {
					const auto name = GetDubuggerName(debugger);
					const auto isSelected = debugger == m_selectedDebugger;
					if (ImGui::Selectable(name.c_str(), isSelected)) {
						m_selectedDebugger = debugger;
						m_selectedDebuggerStr = name;
					}
				}
				ImGui::EndCombo();
			}

			NewLine();
			Text::Text("Select process to attach:").show();
			m_textInput.show();
			if (m_textInput.isTextEntering())
				m_processListModel.m_filterName = m_textInput.getInputText();
			ImGui::BeginChild("##processes", ImVec2(0, 400));
			m_tableProcessListView.show();
			ImGui::EndChild();

			NewLine();
			Text::Text("Select parent address space:").show();
			m_addrSpaceController.update();
			if (ImGui::BeginCombo("##combo_addr_space", m_selectedParentAddrSpaceStr.c_str())) {
				m_addrSpaceListView.show();
				ImGui::EndCombo();
			}
			
			NewLine();
			std::string btnText = "Nothing to attach";
			if (m_selectedProcess)
				btnText = "Attach to " + m_selectedProcess->m_name;
			if (Button::StdButton(btnText).present()) {
				if(m_selectedProcess) {
					m_debugSession = CreateDebugSession(m_selectedDebugger);
					m_debugSession->attachProcess(*m_selectedProcess);
					m_selectProcessEventHandler();
					m_window->close();
				}
			}
		}
	};

	struct SymbolValue
	{
		CE::Decompiler::DataValue m_value;
		bool m_userDefined;
	};
	using SymbolValueMap = std::map<HS, SymbolValue>;
	
	class ValueViewerPanel : public AbstractPanel
	{
	public:
		struct Location
		{
			bool m_isSymbol;
			CE::Decompiler::Symbol::Symbol* m_symbol;
			std::uintptr_t m_address = 0x0;

			Location(CE::Decompiler::Symbol::Symbol* symbol)
				: m_symbol(symbol), m_isSymbol(true)
			{}

			Location(std::uintptr_t address = 0x0)
				: m_address(address), m_isSymbol(false)
			{}
		};
	
	private:
		std::string m_name;
		Location m_location;
		CE::DataTypePtr m_dataType;
		SymbolValueMap* m_symbolValueMap;
		CE::Decompiler::PCode::VmExecutionContext* m_execCtx;
		CE::Decompiler::PCode::VmMemoryContext* m_memCtx;
		bool m_hexView = false;

		PopupBuiltinWindow* m_builtinWindow = nullptr;
	
	public:
		// need to set param registers in the beigining of the function
		bool m_takeValueFromRegister = false;
		
		ValueViewerPanel(const std::string& name, const Location& location, CE::DataTypePtr dataType, SymbolValueMap* symbolValueMap, CE::Decompiler::PCode::VmExecutionContext* execCtx, CE::Decompiler::PCode::VmMemoryContext* memCtx)
			: m_name(name), m_location(location), m_dataType(std::move(dataType)), m_symbolValueMap(symbolValueMap), m_execCtx(execCtx), m_memCtx(memCtx)
		{}

		~ValueViewerPanel() {
			delete m_builtinWindow;
		}

	private:
		void renderPanel() override {
			if (ImGui::BeginTable("##empty", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
			{
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 7.0f);
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("DataType", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
				
				renderContent(m_name, m_location, m_dataType);
				ImGui::EndTable();
			}

			if(m_builtinWindow) {
				// don't close unfocused window if the cursor hovers the child builtin window
				if (const auto win = dynamic_cast<PopupBuiltinWindow*>(m_window))
					win->updateCloseTimer();
			}
			Show(m_builtinWindow);
		}

		void renderContent(const std::string& name, const Location& location, CE::DataTypePtr dataType) {
			const auto isPointer = dataType->isPointer();
			if (!isPointer) {
				if (const auto Structure = dynamic_cast<CE::DataType::IStructure*>(dataType->getType())) {
					if (!location.m_isSymbol) {
						// todo: show unknown fields also
						for (const auto& [offset, field] : Structure->getFields()) {
							if (field->isBitField())
								continue;
							const auto fieldName = name + "." + field->getName();
							const auto fieldLoc = Location(location.m_address + field->getOffset());
							
							if (!field->getDataType()->isPointer() && dynamic_cast<CE::DataType::IStructure*>(field->getDataType()->getType())) {
								ImGui::TableNextRow();

								ImGui::TableNextColumn();
								Text::Text(">").show();
								const auto events = GenericEvents(true);
								if (events.isHovered()) {
									delete m_builtinWindow;
									const auto panel = new ValueViewerPanel(field->getName(), fieldLoc, field->getDataType(), m_symbolValueMap, m_execCtx, m_memCtx);
									m_builtinWindow = new PopupBuiltinWindow(panel);
									m_builtinWindow->m_closeTimerMs = 50;
									m_builtinWindow->placeAfterItem();
									m_builtinWindow->open();
								}
								
								ImGui::TableNextColumn();
								Text::Text(fieldName).show();
								
								ImGui::TableNextColumn();
								Text::Text(field->getDataType()->getName()).show();

								ImGui::TableNextColumn();
								Text::Text("...").show();
							}
							else {
								renderContent(fieldName, fieldLoc, field->getDataType());
							}
						}
						return;
					}
				}
				else if (const auto Typedef = dynamic_cast<CE::DataType::Typedef*>(dataType->getType())) {
					renderContent(name, location, Typedef->getRefType());
					return;
				}
			}

			// todo: array

			// get value
			CE::Decompiler::DataValue value;
			auto isTakenFromRegister = false; // mark value as can-be-invalid
			const auto hasValue = getValue(location, value, isTakenFromRegister);
			ImGui::TableNextRow();

			// +
			ImGui::TableNextColumn();
			if (hasValue && isPointer) {
				Text::Text(">").show();
				const auto events = GenericEvents(true);
				if (events.isHovered()) {
					const auto derefDataType = CloneUnit(dataType);
					derefDataType->removePointerLevelFromTop();
					delete m_builtinWindow;
					const auto panel = new ValueViewerPanel(name, Location(value), derefDataType, m_symbolValueMap, m_execCtx, m_memCtx);
					m_builtinWindow = new PopupBuiltinWindow(panel);
					m_builtinWindow->m_closeTimerMs = 50;
					m_builtinWindow->placeAfterItem();
					m_builtinWindow->open();
				}
			} else {
				Text::Text("-").show();
			}

			// name
			ImGui::TableNextColumn();
			Text::Text(name).show();

			// data type
			ImGui::TableNextColumn();
			Text::Text(dataType->getDisplayName()).show();

			// value
			ImGui::TableNextColumn();
			if (hasValue) {
				const auto color = isTakenFromRegister ? 0xe6d8c3FF : 0xffffffFF;
				if(isPointer) {
					const auto addrStr = "0x" + Helper::String::NumberToHex(value);
					Text::ColoredText("-> " + addrStr, color).show();
				}
				else {
					const auto valueStr = ValueToStr(value, dataType->getType(), m_hexView);
					Text::ColoredText(valueStr, color).show();
					renderEditor(valueStr, location, dataType);
				}
			}
			else {
				Text::ColoredText("cannot read", 0xd6b0b0FF).show();
				if (!isPointer) {
					renderEditor("0", location, dataType);
				}
			}
		}

		void renderEditor(const std::string& valueStr, const Location& location, CE::DataTypePtr dataType) {
			const auto events = GenericEvents(true);
			if (events.isHovered()) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			}
			if (events.isClickedByLeftMouseBtn()) {
				const auto panel = new BuiltinTextInputPanel(valueStr);
				panel->handler([&, panel, location, dataType](const std::string& inputValue)
					{
						const auto value = StrToValue(inputValue, dataType->getType());
						setValue(location, value);
						if (const auto win = dynamic_cast<PopupBuiltinWindow*>(m_window))
							win->m_closeTimerMs = 0;
						panel->m_window->close();
					});
				delete m_builtinWindow;
				m_builtinWindow = new PopupBuiltinWindow(panel);
				m_builtinWindow->placeAfterItem();
				m_builtinWindow->open();
			}
		}
		
		bool getValue(const Location& location, CE::Decompiler::DataValue& value, bool& isTakenFromRegister) const {
			if (location.m_isSymbol) {
				// from register
				CE::Decompiler::Register reg;
				bool hasRegValue = false;
				if(GetRegister(location.m_symbol, reg)) {
					hasRegValue = m_execCtx->getRegisterValue(reg, value);
					if (m_takeValueFromRegister || reg.isPointer()) // rip/rsp
						return hasRegValue;
				}
				
				// from symbol value map
				const auto it = m_symbolValueMap->find(location.m_symbol->getHash());
				if (it == m_symbolValueMap->end()) {
					// if we cannot take value from symbol value map then take from register right away but this value can be invalid (orange color)
					isTakenFromRegister = true;
					return hasRegValue;
				}
				value = it->second.m_value;
				return true;
			}
			return m_memCtx->getValue(location.m_address, value);
		}

		void setValue(const Location& location, CE::Decompiler::DataValue value) const {
			if (location.m_isSymbol) {
				CE::Decompiler::Register reg;
				if (GetRegister(location.m_symbol, reg)) {
					if (m_takeValueFromRegister || reg.isPointer()) {
						m_execCtx->setRegisterValue(reg, value);
						(*m_symbolValueMap)[location.m_symbol->getHash()] = { value, false };
					}
					return;
				}
				const auto it = m_symbolValueMap->find(location.m_symbol->getHash());
				if (it == m_symbolValueMap->end()) {
					(*m_symbolValueMap)[location.m_symbol->getHash()] = { value, true };
				}
				// todo: make symbol reassignments (see loops where localVar/memVar/funcVar is in)
			}
			else {
				m_memCtx->setValue(location.m_address, value);
			}
		}

		static bool GetRegister(CE::Decompiler::Symbol::Symbol* symbol, CE::Decompiler::Register& reg) {
			if(const auto regSymbol = dynamic_cast<CE::Decompiler::Symbol::RegisterVariable*>(symbol)) {
				reg = regSymbol->m_register;
				return true;
			}
			if (const auto localSymbol = dynamic_cast<CE::Decompiler::Symbol::LocalVariable*>(symbol)) {
				reg = localSymbol->m_register;
				return true;
			}
			if (const auto funcSymbol = dynamic_cast<CE::Decompiler::Symbol::FunctionResultVar*>(symbol)) {
				reg = funcSymbol->m_register;
				return true;
			}
			return false;
		}
		
		static std::string ValueToStr(uint64_t value, CE::DataType::IType* dataType, bool hexView) {
			value &= CE::Decompiler::BitMask64(dataType->getSize()).getValue();
			if(hexView) {
				return "0x" + Helper::String::NumberToHex(value);
			}
			
			if(const auto SysType = dynamic_cast<CE::DataType::SystemType*>(dataType)) {
				if(SysType->getTypeId() == CE::DataType::SystemType::Bool) {
					return (bool&)value ? "true" : "false";
				}
				if (SysType->getTypeId() == CE::DataType::SystemType::Int8) {
					return std::to_string((int8_t&)value);
				}
				if (SysType->getTypeId() == CE::DataType::SystemType::Int16) {
					return std::to_string((int16_t&)value);
				}
				if (SysType->getTypeId() == CE::DataType::SystemType::Int32) {
					return std::to_string((int32_t&)value);
				}
				if (SysType->getTypeId() == CE::DataType::SystemType::Int64) {
					return std::to_string((int64_t&)value);
				}
				if (SysType->getTypeId() == CE::DataType::SystemType::Float) {
					return std::to_string((float&)value);
				}
				if (SysType->getTypeId() == CE::DataType::SystemType::Double) {
					return std::to_string((double&)value);
				}
			} else if (const auto Enum = dynamic_cast<CE::DataType::Enum*>(dataType)) {
				const auto it = Enum->getFields().find((int&)value);
				const auto intValueStr = std::to_string((int&)value);
				if (it == Enum->getFields().end())
					return intValueStr;
				return it->second + " (" + intValueStr + ")";
			}
			
			return std::to_string(value);
		}

		static uint64_t StrToValue(const std::string& valueStr, CE::DataType::IType* dataType) {
			uint64_t value = 0;
			
			if(valueStr[1] == 'x') {
				value = Helper::String::HexToNumber(valueStr) & CE::Decompiler::BitMask64(dataType->getSize()).getValue();
			} else {
				if (const auto SysType = dynamic_cast<CE::DataType::SystemType*>(dataType)) {
					if (SysType->getTypeId() == CE::DataType::SystemType::Int8) {
						(int8_t&)value = std::stoi(valueStr);
					}
					else if (SysType->getTypeId() == CE::DataType::SystemType::Int16) {
						(int16_t&)value = std::stoi(valueStr);
					}
					else if (SysType->getTypeId() == CE::DataType::SystemType::Int32) {
						(int32_t&)value = std::stoi(valueStr);
					}
					else if (SysType->getTypeId() == CE::DataType::SystemType::Int64) {
						(int64_t&)value = std::stoll(valueStr);
					}
					else if (SysType->getTypeId() == CE::DataType::SystemType::Float) {
						(float&)value = std::stof(valueStr);
					}
					else if (SysType->getTypeId() == CE::DataType::SystemType::Double) {
						(double&)value = std::stod(valueStr);
					}
					else {
						value = std::stoull(valueStr);
					}
				}
			}
			
			return value;
		}
	};
	
	class PCodeEmulator : public Control
	{
		class AbstractViewerPanel : public AbstractPanel
		{
		protected:
			CE::Decompiler::PCode::VmExecutionContext* m_execCtx;
			CE::Decompiler::AbstractRegisterFactory* m_registerFactory;
			PopupBuiltinWindow* m_builtinWindow = nullptr;

			AbstractViewerPanel(const std::string& name, CE::Decompiler::PCode::VmExecutionContext* execCtx, CE::Decompiler::AbstractRegisterFactory* registerFactory)
				: AbstractPanel(name), m_execCtx(execCtx), m_registerFactory(registerFactory)
			{}

			~AbstractViewerPanel() {
				delete m_builtinWindow;
			}

			void renderValueEditor(const std::string& hexValue, const std::function<void(uint64_t)>& callback) {
				const auto events = GenericEvents(true);
				if (events.isHovered()) {
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				}
				if (events.isClickedByLeftMouseBtn()) {
					const auto panel = new BuiltinTextInputPanel("0x" + hexValue);
					panel->handler([&, panel, callback](const std::string& inputValue)
						{
							// todo: special input with various types of values
							const auto value =
								inputValue[1] == 'x' ? Helper::String::HexToNumber(inputValue) : std::stoull(inputValue);
							callback(value);
							panel->m_window->close();
						});
					delete m_builtinWindow;
					m_builtinWindow = new PopupBuiltinWindow(panel);
					m_builtinWindow->placeAfterItem();
					m_builtinWindow->open();
				}
			}
		};
		
		class ExecContextViewerPanel : public AbstractViewerPanel
		{
			Input::BoolInput m_symbolVarnodeCb;
		public:
			ExecContextViewerPanel(CE::Decompiler::PCode::VmExecutionContext* execCtx, CE::Decompiler::AbstractRegisterFactory* registerFactory)
				: AbstractViewerPanel("Execution Context Viewer", execCtx, registerFactory)
			{
				m_symbolVarnodeCb = Input::BoolInput("Symbol varnodes", false);
			}

		private:
			void renderPanel() override {
				m_symbolVarnodeCb.show();
				
				if (ImGui::BeginTable("##empty", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
				{
					ImGui::TableSetupColumn("Register", ImGuiTableColumnFlags_WidthFixed, 70.0f);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
					ImGui::TableHeadersRow();

					if (m_symbolVarnodeCb.isSelected()) {
						renderSymbolVarnodeRows();
					}
					
					renderFlagRegisterRow();

					renderRegisterRow(ZYDIS_REGISTER_RIP, 0x8);
					for(int regId = ZYDIS_REGISTER_RAX; regId <= ZYDIS_REGISTER_R15; regId++) {
						renderRegisterRow(regId, 0x8);
					}

					for (int regId = ZYDIS_REGISTER_XMM0; regId <= ZYDIS_REGISTER_XMM15; regId++) {
						renderRegisterRow(regId, 0x10);
					}

					ImGui::EndTable();
				}

				Show(m_builtinWindow);
			}

			void renderSymbolVarnodeRows() {
				const auto& symbolVarnodes = m_execCtx->getSymbolVarnodes();
				for(const auto& [symbolVarnode, value] : symbolVarnodes) {
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					CE::Decompiler::InstructionTextGenerator instrTextGenerator;
					instrTextGenerator.generateVarnode(symbolVarnode);
					Text::Text(instrTextGenerator.m_text).show();
					ImGui::TableNextColumn();
					const auto hexValue = Helper::String::NumberToHex(value, false);
					Text::Text("0x" + hexValue).show();
					const auto varnode = symbolVarnode;
					renderValueEditor(hexValue, [&, varnode](uint64_t value)
						{
							m_execCtx->setValue(varnode, value);
						});
				}
			}

			void renderRegisterRow(int regId, int size) {
				using namespace Helper::String;
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				Text::Text(CE::Decompiler::PCode::InstructionViewGenerator::GetRegisterName(regId)).show();
				ImGui::TableNextColumn();

				
				int offset = size;
				while (offset > 0) {
					offset -= 0x8;
					const auto reg = m_registerFactory->createRegister(regId, size, offset);
					CE::Decompiler::DataValue regValue;
					std::string hexValue;
					bool hasValue;
					if ((hasValue = m_execCtx->getRegisterValue(reg, regValue))) {
						hexValue = NumberToHex(regValue, true);
					} else {
						hexValue = "----------------";
					}
					Text::Text(hexValue).show();
					renderValueEditor(hasValue ? hexValue : "0000000000000000", [&, reg](uint64_t value)
						{
							m_execCtx->setRegisterValue(reg, value);
						});
					SameLine(5.0f);
				}
			}

			void renderFlagRegisterRow() {
				const auto flags = {
					ZYDIS_CPUFLAG_CF,
					ZYDIS_CPUFLAG_OF,
					ZYDIS_CPUFLAG_SF,
					ZYDIS_CPUFLAG_ZF,
					ZYDIS_CPUFLAG_AF,
					ZYDIS_CPUFLAG_PF
				};
				
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				Text::Text("flags").show();
				ImGui::TableNextColumn();

				CE::Decompiler::DataValue regValue;
				const auto reg = CE::Decompiler::Register(ZYDIS_REGISTER_RFLAGS, 0, 0x8);
				if (m_execCtx->getRegisterValue(reg, regValue)) {
					for (const auto flag : flags) {
						const auto flagName = CE::Decompiler::PCode::InstructionViewGenerator::GetFlagName(flag);
						const auto value = regValue >> flag & 0b1;
						Text::Text(flagName + ": " + std::to_string(value)).show();
						const auto events = GenericEvents(true);
						if (events.isHovered()) {
							ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
							ImGui::SetTooltip("Click to switch");
						}
						if (events.isClickedByLeftMouseBtn()) {
							m_execCtx->setRegisterValue(reg, regValue ^ static_cast<CE::Decompiler::DataValue>(0b1) << flag);
						}
						SameLine(5.0f);
					}
				}
				else {
					Text::Text("-").show();
					const auto events = GenericEvents(true);
					if (events.isHovered()) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						ImGui::SetTooltip("Click to fill");
					}
					if (events.isClickedByLeftMouseBtn()) {
						m_execCtx->setRegisterValue(reg, 0x0);
					}
				}
			}
		};

		class StackViewerPanel : public AbstractViewerPanel
		{
			CE::Decompiler::PCode::VmMemoryContext* m_memCtx;
			CE::Decompiler::DataValue m_stackPointerAddr = 0;
			bool m_needScrool = false;
		public:
			StackViewerPanel(CE::Decompiler::PCode::VmExecutionContext* execCtx, CE::Decompiler::PCode::VmMemoryContext* memCtx, CE::Decompiler::AbstractRegisterFactory* registerFactory)
				: AbstractViewerPanel("Stack Viewer", execCtx, registerFactory), m_memCtx(memCtx)
			{}
		private:
			void renderPanel() override {
				if (ImGui::BeginTable("##empty", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
					ImGui::TableHeadersRow();

					const auto reg = m_registerFactory->createStackPointerRegister();
					CE::Decompiler::DataValue stackPointerAddr;
					if (m_execCtx->getRegisterValue(reg, stackPointerAddr)) {
						if(m_needScrool)
							ImGui::SetScrollY(ImGui::GetScrollMaxY());
						m_needScrool = stackPointerAddr != m_stackPointerAddr;
						m_stackPointerAddr = stackPointerAddr;
						
						const auto rowsCount = 1000;
						ImGuiListClipper clipper;
						clipper.Begin(rowsCount);
						while (clipper.Step())
						{
							for (auto rowIdx = clipper.DisplayStart; rowIdx < clipper.DisplayEnd; rowIdx++) {
								const auto addr = stackPointerAddr + (rowsCount - rowIdx - 1) * 0x8;
								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								Text::Text(Helper::String::NumberToHex(addr, true)).show();
								ImGui::TableNextColumn();
								std::string hexValue;
								bool hasValue;
								CE::Decompiler::DataValue value;
								if ((hasValue = m_memCtx->getValue(addr, value))) {
									hexValue = Helper::String::NumberToHex(value, true);
								}
								else {
									hexValue = "----------------";
								}
								Text::Text(hexValue).show();
								renderValueEditor(hasValue ? hexValue : "0000000000000000", [&, addr](uint64_t value)
									{
										m_memCtx->setValue(addr, value);
									});
							}
						}
					}
					ImGui::EndTable();
				}

				Show(m_builtinWindow);
			}
		};

	protected:
		CE::Decompiler::RegisterFactoryX86 m_registerFactoryX86;
		CE::Decompiler::PCode::VmExecutionContext m_execCtx;
		CE::Decompiler::PCode::VmMemoryContext m_memCtx;
		CE::Decompiler::PCode::VirtualMachine m_vm;
		std::uintptr_t m_curAddr = 0;
		bool m_into = false;
		bool m_isStopped = false;
		bool m_isExit = false;

		StdWindow* m_execCtxViewerWin;
		StdWindow* m_stackViewerWin;
		EventHandler<uint64_t> m_locationHandler;
	public:
		enum class PCodeStepWidth
		{
			STEP_PCODE_INSTR,
			STEP_ORIGINAL_INSTR,
			STEP_CODE_LINE
		};

		// location
		CE::AddressSpace* m_addressSpace;
		CE::ImageDecorator* m_imageDec;
		CE::ComplexOffset m_offset;

		// other
		PCodeStepWidth m_stepWidth = PCodeStepWidth::STEP_ORIGINAL_INSTR;

		// objects at the current offset
		CE::Decompiler::PCode::Instruction* m_curInstr = nullptr;
		CE::Decompiler::PCodeBlock* m_curPCodeBlock = nullptr;
		CE::Decompiler::DecBlock::BlockTopNode* m_curBlockTopNode = nullptr;
		CE::Decompiler::DecompiledCodeGraph* m_curDecGraph = nullptr;
		
		// contexts
		bool m_showContexts = false;
		
		// for value viewer
		struct StackFrameInfo
		{
			SymbolValueMap m_symbolValueMap;
			CE::Decompiler::PCode::Instruction* m_lastExecutedInstr = nullptr;
			CE::Decompiler::DataValue m_lastExecutedInstrValue = 0;
		};
		std::map<std::uintptr_t, StackFrameInfo> m_stackFrameInfo;
		int m_relStackPointerValue = 0;
		PopupBuiltinWindow* m_valueViewerWin = nullptr;
		
		PCodeEmulator(CE::AddressSpace* addressSpace, CE::ImageDecorator* imageDec, CE::ComplexOffset startOffset)
			: m_addressSpace(addressSpace), m_imageDec(imageDec), m_offset(startOffset),  m_vm(&m_execCtx, &m_memCtx, false)
		{
			m_execCtxViewerWin = new StdWindow(new ExecContextViewerPanel(&m_execCtx, &m_registerFactoryX86));
			m_stackViewerWin = new StdWindow(new StackViewerPanel(&m_execCtx, &m_memCtx, &m_registerFactoryX86));
			m_execCtx.setRegisterValue(CE::Decompiler::Register(ZYDIS_REGISTER_RSP, 0), 0x1000000000);
		}

		~PCodeEmulator() {
			delete m_execCtxViewerWin;
			delete m_stackViewerWin;
			delete m_valueViewerWin;
		}

		bool isWorking() const { // todo: replace
			return !m_isStopped && !m_isExit;
		}

		void locationHandler(const std::function<void(uint64_t)>& handler) {
			m_locationHandler = handler;
		}

		void goDebug(CE::ImageDecorator* imageDec, CE::ComplexOffset offset) {
			m_imageDec = imageDec;
			m_offset = offset;
			defineCurPCodeInstruction();
		}

		virtual void sync() {
			std::map<int, CE::Decompiler::DataValue> registers;
			registers = m_execCtx.getRegisters();
			m_execCtx.syncWith(registers);
			m_execCtx.setRegisterValue(CE::Decompiler::Register(ZYDIS_REGISTER_RIP, 0), m_imageDec->getImage()->getAddress() + m_offset.getByteOffset());
			defineCurPCodeInstruction();
		}

		bool setLocationByAddress(std::uintptr_t addr, int orderId = 0) {
			m_imageDec = m_addressSpace->getImageDecoratorAt(addr);
			if (!m_imageDec) {
				m_offset = 0x0;
				m_isStopped = true;
				return false;
			}
			m_offset = CE::ComplexOffset(m_imageDec->getImage()->addressToOffset(addr), orderId);
			return true;
		}

		void updateSymbolValuesByDecGraph(CE::Decompiler::DecompiledCodeGraph* decGraph, CE::ComplexOffset offset,
		                                  bool after);

		void createValueViewer(const CE::Decompiler::ExprTree::StoragePath& storagePath, const std::string& name, CE::DataTypePtr dataType) {
			ValueViewerPanel::Location location;
			if (!getMemLocation(storagePath, location))
				return;
			
			delete m_valueViewerWin;
			const auto panel = new ValueViewerPanel(name, location, dataType, &getCurStackFrameInfo()->m_symbolValueMap, &m_execCtx, &m_memCtx);
			if (m_curPCodeBlock) {
				panel->m_takeValueFromRegister = m_offset == m_curPCodeBlock->m_funcPCodeGraph->getStartBlock()->getMinOffset();
			}
			m_valueViewerWin = new PopupBuiltinWindow(panel);
			m_valueViewerWin->m_closeTimerMs = 50;
			m_valueViewerWin->placeAfterItem();
			m_valueViewerWin->open();
		}

		virtual void renderDebugMenu() {
			if (isWorking()) {
				if (ImGui::MenuItem("Exit Debug")) {
					m_isExit = true;
				}

				if (ImGui::MenuItem("Step Over", "F8")) {
					stepOver();
				}

				if (ImGui::MenuItem("Step Into", "F7")) {
					stepInto();
				}

				if (ImGui::BeginMenu("Step Width"))
				{
					if (ImGui::MenuItem("PCode Instruction", nullptr, m_stepWidth == PCodeStepWidth::STEP_PCODE_INSTR)) {
						m_stepWidth = PCodeStepWidth::STEP_PCODE_INSTR;
					}
					if (ImGui::MenuItem("Orig. Instruction", nullptr, m_stepWidth == PCodeStepWidth::STEP_ORIGINAL_INSTR)) {
						m_stepWidth = PCodeStepWidth::STEP_ORIGINAL_INSTR;
					}
					if (ImGui::MenuItem("Code Line", nullptr, m_stepWidth == PCodeStepWidth::STEP_CODE_LINE)) {
						m_stepWidth = PCodeStepWidth::STEP_CODE_LINE;
					}
					ImGui::EndMenu();
				}
			} else {
				if (ImGui::MenuItem("Show Last Context", nullptr, m_showContexts)) {
					m_showContexts ^= true;
				}
			}
		}
	
	protected:
		void renderControl() override;

		void defineNextInstrOffset() {
			const auto jmp = m_into ? true : m_curInstr->m_id != CE::Decompiler::PCode::InstructionId::CALL;
			if (jmp && m_execCtx.m_nextInstrAddr != 0) {
				if(dynamic_cast<CE::Decompiler::ConstantVarnode*>(m_curInstr->m_input0)) {
					// any constant value in jump instructions (branch/call) is offset from start of the current image
					m_offset = m_execCtx.m_nextInstrAddr;
				} else {
					// jmp [rax] (can visit other image)
					setLocationByAddress(m_execCtx.m_nextInstrAddr);
				}
				m_execCtx.m_nextInstrAddr = 0;
				return;
			}
			
			const auto origInstr = m_curInstr->m_origInstruction;
			if (m_curInstr->m_orderId != origInstr->m_pcodeInstructions.size() - 1) {
				m_offset = CE::ComplexOffset(m_offset.getByteOffset(), m_offset.getOrderId() + 1);
			}
			else {
				m_offset = CE::ComplexOffset(origInstr->m_offset + origInstr->m_length, 0);
			}
		}

		bool defineCurPCodeInstruction();

		void stepNextPCodeInstr();

		void stepNextOrigInstr() {
			do {
				stepNextPCodeInstr();
				if (!isWorking())
					return;
			} while (!isNewOrigInstruction());
		}

		void stepNextBlockTopNode() {
			bool isNewBlockTopNode;
			do {
				const auto prevBlockTopNode = m_curBlockTopNode;
				stepNextPCodeInstr();
				if (!isWorking())
					return;
				isNewBlockTopNode = m_curBlockTopNode != prevBlockTopNode;
			} while (m_curBlockTopNode && !isNewBlockTopNode);
		}
		
		virtual void stepOver() {
			switch(m_stepWidth) {
			case PCodeStepWidth::STEP_PCODE_INSTR:
				stepNextPCodeInstr();
				break;
			case PCodeStepWidth::STEP_ORIGINAL_INSTR:
				stepNextOrigInstr();
				break;
			case PCodeStepWidth::STEP_CODE_LINE:
				stepNextBlockTopNode();
				break;
			}
		}

		void stepInto() {
			m_into = true;
			stepOver();
			m_into = false;
		}

		bool isNewOrigInstruction() const {
			return m_offset.getOrderId() == 0;
		}

		StackFrameInfo* getCurStackFrameInfo() {
			const auto curStackFrame = getCurrentStackFrame();
			return &m_stackFrameInfo[curStackFrame];
		}

		std::uintptr_t getCurrentStackFrame() const {
			const auto rsp = CE::Decompiler::Register(ZYDIS_REGISTER_RSP, 0);
			CE::Decompiler::DataValue stackPointer;
			if (!m_execCtx.getRegisterValue(rsp, stackPointer))
				return 0;
			return stackPointer - m_relStackPointerValue;
		}

		bool getMemLocation(const CE::Decompiler::ExprTree::StoragePath& storagePath, ValueViewerPanel::Location& location) {
			location = ValueViewerPanel::Location(storagePath.m_symbol);

			// offsets (pointer dereferencing)
			for (const auto offset : storagePath.m_offsets) {
				CE::Decompiler::DataValue baseAddr;
				if (location.m_isSymbol) {
					const auto regSymbol = dynamic_cast<CE::Decompiler::Symbol::RegisterVariable*>(location.m_symbol);
					if (regSymbol && regSymbol->m_register.isPointer()) {
						const auto& reg = regSymbol->m_register;
						if (!m_execCtx.getRegisterValue(reg, baseAddr))
							return false;
						if (reg.getType() == CE::Decompiler::Register::Type::StackPointer) {
							baseAddr -= m_relStackPointerValue;
						}
						else if (reg.getType() == CE::Decompiler::Register::Type::InstructionPointer) {
							baseAddr -= m_offset.getByteOffset();
						}
					} else {
						const auto stackFrameInfo = getCurStackFrameInfo();
						const auto it = stackFrameInfo->m_symbolValueMap.find(location.m_symbol->getHash());
						if (it == stackFrameInfo->m_symbolValueMap.end())
							return false;
						baseAddr = it->second.m_value;
					}
				}
				else {
					if (!m_memCtx.getValue(location.m_address, baseAddr))
						return false;
				}
				location = ValueViewerPanel::Location(baseAddr + offset);
			}
			return true;
		}
	};

	class Debugger : public Control
	{
		class PCodeEmulatorWithDebugSession : public PCodeEmulator
		{
			friend class Debugger;
			CE::IDebugSession* m_debugSession;
			bool m_isSyncing = false;
			bool m_isInto = false;
		public:
			PCodeEmulatorWithDebugSession(CE::AddressSpace* addressSpace, CE::IDebugSession* debugSession)
				: PCodeEmulator(addressSpace, nullptr, 0x0), m_debugSession(debugSession)
			{
				m_isStopped = true;
				m_showContexts = true;
				m_memCtx.setAddressSpaceCallaback([&](std::uintptr_t address, CE::Decompiler::DataValue& value)
					{
						if (!m_debugSession->isSuspended())
							return false;
						std::vector<uint8_t> data(sizeof(value));
						try {
							m_debugSession->readMemory(address, data);
						} catch(CE::DebugException&) {
							return false;
						}
						value = *reinterpret_cast<CE::Decompiler::DataValue*>(data.data());
						return true;
					});
			}

			void newOrigInstructionExecuted() {
				if (!m_isSyncing) {
					m_stackFrameInfo.clear();
				}
				
				m_execCtx.m_nextInstrAddr = 0;
				syncContext();

				const auto prevBlockTopNode = m_curBlockTopNode;
				defineCurPCodeInstruction();
				if(m_isSyncing) {
					m_isSyncing = false;
					if (m_stepWidth == PCodeStepWidth::STEP_CODE_LINE) {
						if(m_curInstr && m_curBlockTopNode == prevBlockTopNode) {
							if (m_isInto)
								stepInto();
							else stepOver();
						}
					}
				}
			}

			void renderDebugMenu() override {
				if (!m_isExit && m_isStopped) {
					if (ImGui::MenuItem("Exit Debug")) {
						m_isExit = true;
					}
				}
				
				PCodeEmulator::renderDebugMenu();

				if (m_debugSession->isWorking()) {
					if (m_debugSession->isSuspended()) {
						if (ImGui::MenuItem("Resume Process", "F5")) {
							m_debugSession->resume();
						}
					}
					else {
						if (ImGui::MenuItem("Pause Process", "F5")) {
							m_debugSession->pause();
						}
					}
				}
			}
		
		private:
			void renderControl() override;

			void stepOver() override {
				if (m_curInstr) {
					PCodeEmulator::stepOver();
				} else {
					step(m_into);
				}
			}

			void sync() override {
				step(m_into);
				m_isStopped = true;
				m_isSyncing = true;
				m_isInto = m_into;
			}

			void step(bool into) const {
				if (into) {
					m_debugSession->stepInto();
				}
				else {
					m_debugSession->stepOver();
				}
			}

			void syncContext() {
				std::map<int, CE::Decompiler::DataValue> registers;
				const auto debugRegisters = m_debugSession->getRegisters();
				for (const auto debugReg : debugRegisters) {
					const auto reg = CE::Decompiler::Register(debugReg.m_id, debugReg.m_index);
					registers[reg.getId()] = debugReg.m_value;
				}
				m_execCtx.syncWith(registers);
			}
		};
		
		CE::Project* m_project;
		CE::IDebugSession* m_debugSession;
		CE::AddressSpace* m_addressSpace;
		CE::AddressSpace* m_parentAddressSpace;
		CE::Symbol::GlobalSymbolTable* m_globalSymbolTable;
		CE::Symbol::GlobalSymbolTable* m_funcBodySymbolTable;
		std::uintptr_t m_curAddr = 0x0;
	public:
		std::map<std::uintptr_t, CE::ImageDecorator*> m_images;
		PCodeEmulatorWithDebugSession* m_emulator;
		
		Debugger(CE::Project* project, CE::IDebugSession* debugSession, CE::AddressSpace* parentAddressSpace = nullptr)
			: m_project(project), m_debugSession(debugSession), m_parentAddressSpace(parentAddressSpace)
		{
			const auto factory = m_project->getSymTableManager()->getFactory(false);
			m_globalSymbolTable = factory.createGlobalSymbolTable();
			m_funcBodySymbolTable = factory.createGlobalSymbolTable();
			m_addressSpace = m_project->getAddrSpaceManager()->createAddressSpace(debugSession->getProcess().m_name, "", false);
			m_addressSpace->m_debugSession = m_debugSession;
			m_project->getAddrSpaceManager()->m_debugAddressSpace = m_addressSpace;
			m_emulator = new PCodeEmulatorWithDebugSession(m_addressSpace, m_debugSession);
		}

		~Debugger() {
			delete m_globalSymbolTable;
			delete m_funcBodySymbolTable;
			for (const auto& [addr, imageDec] : m_images)
				delete imageDec;
			m_project->getAddrSpaceManager()->m_debugAddressSpace = nullptr;
			delete m_addressSpace;
		}

		bool isWorking() const {
			return m_debugSession->isWorking();
		}
	
	private:
		void renderControl() override {
			// todo: timer 1 sec
			updateModules();
			updateLocation();

			if(!m_debugSession->isSuspended()) {
				m_emulator->m_isStopped = true;
				m_curAddr = 0x0;
			}
		}

		void updateModules() {
			const auto modules = m_debugSession->getModules();
			for (const auto m : modules) {
				const auto it = m_images.find(m.m_baseAddress);
				if (it != m_images.end())
					continue;
				m_images[m.m_baseAddress] = createImage(m);
			}
		}

		void updateLocation() {
			if (m_debugSession->isSuspended()) {
				const auto address = m_debugSession->getInstructionAddress();
				if (address != m_curAddr) {
					if (m_emulator->setLocationByAddress(address)) {
						// for location handler
						m_emulator->m_isStopped = false;
						
						// if new instruction meet
						m_emulator->newOrigInstructionExecuted();
						
						// set false again if it was set true after newOrigInstructionExecuted
						m_emulator->m_isStopped = false;
					}
					m_curAddr = address;
				}
			}
		}

		CE::ImageDecorator* createImage(const CE::DebugModule& debugModule) const {
			const auto debugReader = new CE::DebugReader(m_debugSession, debugModule);

			// pause process to read memory and then resume
			m_debugSession->pause(true);
			debugReader->setCacheEnabled(true);
			debugReader->updateCache();
			m_debugSession->resume();
			
			auto type = CE::ImageDecorator::IMAGE_NONE;
			if (m_debugSession->getDebugger() == CE::Debugger::DebuggerEngine) {
				type = CE::ImageDecorator::IMAGE_PE;
			}

			const auto name = debugModule.m_path.filename().string();
			CE::ImageDecorator* imageDec = nullptr;
			if (m_parentAddressSpace) {
				for(const auto parentImageDec : m_parentAddressSpace->getImageDecorators()) {
					if (std::string(imageDec->getName()).find(parentImageDec->getName()) != std::string::npos) {
						imageDec = m_project->getImageManager()->createImageFromParent(m_addressSpace, parentImageDec, name, "", false);
						break;
					}
				}
			}
			if (!imageDec) {
				imageDec = m_project->getImageManager()->createImage(m_addressSpace, type, m_globalSymbolTable, m_funcBodySymbolTable, name, "", false);
			}
			imageDec->m_debugSession = m_debugSession;
			imageDec->createImage(debugReader);
			imageDec->getImage()->m_inVirtualMemory = true;
			return imageDec;
		}
	};
};