#pragma once
#include <utility>

#include "ImageDecorator.h"
#include "decompiler/PCode/DecPCodeInstructionPool.h"
#include "decompiler/PCode/DecPCodeVirtualMachine.h"
#include "decompiler/PCode/DecRegisterFactory.h"
#include "decompiler/Graph/DecPCodeGraph.h"
#include "decompiler/Graph/DecCodeGraphBlock.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Text.h"
#include "panels/BuiltinInputPanel.h"
#include "utilities/Helper.h"

namespace GUI
{
	class ValueViewerPanel : public AbstractPanel
	{
	public:
		struct Location
		{
			bool m_isRegister;
			CE::Decompiler::Register m_register;
			std::uintptr_t m_address = 0x0;

			Location(const CE::Decompiler::Register& reg)
				: m_register(reg), m_isRegister(true)
			{}

			Location(std::uintptr_t address = 0x0)
				: m_address(address), m_isRegister(false)
			{}
		};
	
	private:
		std::string m_name;
		Location m_location;
		CE::DataTypePtr m_dataType;
		CE::Decompiler::PCode::VmExecutionContext* m_execCtx;
		CE::Decompiler::PCode::VmMemoryContext* m_memCtx;
		bool m_hexView = false;

		PopupBuiltinWindow* m_builtinWindow = nullptr;
	
	public:
		ValueViewerPanel(const std::string& name, const Location& location, CE::DataTypePtr dataType, CE::Decompiler::PCode::VmExecutionContext* execCtx, CE::Decompiler::PCode::VmMemoryContext* memCtx)
			: m_name(name), m_location(location), m_dataType(std::move(dataType)), m_execCtx(execCtx), m_memCtx(memCtx)
		{}

		~ValueViewerPanel() {
			delete m_builtinWindow;
		}

	private:
		void renderPanel() override {
			if (ImGui::BeginTable("##empty", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
			{
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
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
					if (!location.m_isRegister) {
						// todo: show unknown fields also
						for (const auto& [offset, field] : Structure->getFields()) {
							if (field->isBitField())
								continue;
							const auto fieldName = name + "." + field->getName();
							const auto fieldLoc = Location(location.m_address + field->getOffset());
							
							if (!field->getDataType()->isPointer() && dynamic_cast<CE::DataType::IStructure*>(field->getDataType()->getType())) {
								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								Text::Text(fieldName).show();
								const auto events = GenericEvents(true);
								const auto builtinWinPos = GetLeftBottom();
								
								ImGui::TableNextColumn();
								Text::Text(field->getDataType()->getName()).show();
								
								if (events.isHovered()) {
									delete m_builtinWindow;
									const auto panel = new ValueViewerPanel(name, fieldLoc, field->getDataType(), m_execCtx, m_memCtx);
									m_builtinWindow = new PopupBuiltinWindow(panel);
									m_builtinWindow->m_closeTimerMs = 50;
									m_builtinWindow->getPos() = builtinWinPos;
									m_builtinWindow->open();
								}
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
			
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			Text::Text(name).show();
			const auto events = GenericEvents(true);
			const auto builtinWinPos = GetLeftBottom();
			
			ImGui::TableNextColumn();
			CE::Decompiler::DataValue value;
			if (getValue(location, value)) {
				if(isPointer) {
					const auto addrStr = "0x" + Helper::String::NumberToHex(value);
					Text::Text("-> " + addrStr).show();
				}
				else {
					const auto valueStr = ValueToStr(value, dataType->getType(), m_hexView);
					Text::Text(valueStr).show();
					renderEditor(valueStr, location, dataType);
				}

				if (isPointer) {
					if (events.isHovered()) {
						const auto derefDataType = GetUnit(dataType->getType());
						derefDataType->removePointerLevelOutOfFront();
						delete m_builtinWindow;
						const auto panel = new ValueViewerPanel(name, Location(value), derefDataType, m_execCtx, m_memCtx);
						m_builtinWindow = new PopupBuiltinWindow(panel);
						m_builtinWindow->m_closeTimerMs = 50;
						m_builtinWindow->getPos() = builtinWinPos;
						m_builtinWindow->open();
					}
				}
			}
			else {
				Text::Text("cannot read").show();
				renderEditor("0", location, dataType);
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
		
		bool getValue(const Location& location, CE::Decompiler::DataValue& value) const {
			if (location.m_isRegister) {
				return m_execCtx->getRegisterValue(location.m_register, value);
			}
			return m_memCtx->getValue(location.m_address, value);
		}

		void setValue(const Location& location, CE::Decompiler::DataValue value) const {
			if (location.m_isRegister) {
				m_execCtx->setRegisterValue(location.m_register, value);
			}
			else {
				m_memCtx->setValue(location.m_address, value);
			}
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
	
	class Debugger : public Control
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
						if(stackPointerAddr != m_stackPointerAddr)
							ImGui::SetScrollY(ImGui::GetScrollMaxY());
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

		CE::Decompiler::RegisterFactoryX86 m_registerFactoryX86;
		CE::Decompiler::PCode::VmExecutionContext m_execCtx;
		CE::Decompiler::PCode::VmMemoryContext m_memCtx;
		CE::Decompiler::PCode::VirtualMachine m_vm;
		CE::ComplexOffset m_offset;

		StdWindow* m_execCtxViewerWin;
		StdWindow* m_stackViewerWin;
		EventHandler<bool> m_instrHandler;
	public:
		enum class StepWidth
		{
			STEP_PCODE_INSTR,
			STEP_ORIGINAL_INSTR,
			STEP_CODE_LINE
		};
		
		CE::ImageDecorator* m_imageDec;
		StepWidth m_stepWidth = StepWidth::STEP_PCODE_INSTR;
		CE::Decompiler::PCode::Instruction* m_curInstr = nullptr;
		CE::Decompiler::PCodeBlock* m_curPCodeBlock = nullptr;
		CE::Decompiler::DecBlock::BlockTopNode* m_curBlockTopNode = nullptr;
		bool m_isStopped = false;
		bool m_showContexts = false;
		PopupBuiltinWindow* m_valueViewerWin = nullptr;
		
		Debugger(CE::ImageDecorator* imageDec, CE::ComplexOffset startOffset)
			: m_imageDec(imageDec), m_offset(startOffset), m_vm(&m_execCtx, &m_memCtx, false)
		{
			m_execCtxViewerWin = new StdWindow(new ExecContextViewerPanel(&m_execCtx, &m_registerFactoryX86));
			m_stackViewerWin = new StdWindow(new StackViewerPanel(&m_execCtx, &m_memCtx, &m_registerFactoryX86));

			m_execCtx.setRegisterValue(CE::Decompiler::Register(ZYDIS_REGISTER_RSP), 0x1000000000);
			m_execCtx.setRegisterValue(CE::Decompiler::Register(ZYDIS_REGISTER_RIP), 0x2000000000);
		}

		~Debugger() {
			delete m_execCtxViewerWin;
			delete m_stackViewerWin;
			delete m_valueViewerWin;
		}

		void instrHandler(const std::function<void(bool)>& handler) {
			m_instrHandler = handler;
		}

		void goDebug(CE::ComplexOffset offset) {
			m_offset = offset;
			defineCurInstrutction();
		}

		void defineCurInstrutction() {
			const auto prevPCodeGraph = m_curPCodeBlock ? m_curPCodeBlock->m_funcPCodeGraph : nullptr;

			m_curInstr = m_imageDec->getInstrPool()->getPCodeInstructionAt(m_offset);
			if (!m_curInstr) {
				m_curPCodeBlock = nullptr;
				m_curBlockTopNode = nullptr;
				m_isStopped = true;
				return;
			}
			m_curPCodeBlock = m_imageDec->getPCodeGraph()->getBlockAtOffset(m_offset);

			if (m_instrHandler.isInit()) {
				const auto isNewGraph = prevPCodeGraph != m_curPCodeBlock->m_funcPCodeGraph;
				m_instrHandler(isNewGraph);
			}
		}

		void createValueViewer(const CE::Decompiler::StoragePath& storagePath, const std::string& name, CE::DataTypePtr dataType) {
			ValueViewerPanel::Location location;
			if (!getLocation(storagePath, location))
				return;
			
			delete m_valueViewerWin;
			const auto panel = new ValueViewerPanel(name, location, dataType, &m_execCtx, &m_memCtx);
			m_valueViewerWin = new PopupBuiltinWindow(panel);
			m_valueViewerWin->m_closeTimerMs = 50;
			m_valueViewerWin->placeAfterItem();
			m_valueViewerWin->open();
		}

		void renderDebugMenu() {
			if (!m_isStopped) {
				if (ImGui::MenuItem("Stop Debug")) {
					m_isStopped = true;
				}

				if (ImGui::MenuItem("Step Over", "F8")) {
					stepOver(true);
				}

				if (ImGui::MenuItem("Step Into", "F7")) {
					stepInto();
				}

				if (ImGui::BeginMenu("Step Width"))
				{
					if (ImGui::MenuItem("PCode Instruction", nullptr, m_stepWidth == StepWidth::STEP_PCODE_INSTR)) {
						m_stepWidth = StepWidth::STEP_PCODE_INSTR;
					}
					if (ImGui::MenuItem("Orig. Instruction", nullptr, m_stepWidth == StepWidth::STEP_ORIGINAL_INSTR)) {
						m_stepWidth = StepWidth::STEP_ORIGINAL_INSTR;
					}
					if (ImGui::MenuItem("Code Line", nullptr, m_stepWidth == StepWidth::STEP_CODE_LINE)) {
						m_stepWidth = StepWidth::STEP_CODE_LINE;
					}
					ImGui::EndMenu();
				}
			} else {
				if (ImGui::MenuItem("Show Last Context", nullptr, m_showContexts)) {
					m_showContexts ^= true;
				}
			}
		}
	
	private:
		void renderControl() override {
			if (!m_isStopped) {
				if (ImGui::IsKeyPressed(VK_F8)) { // todo: linux, change imgui_impl_win32.cpp
					stepOver(true);
				}
				else if (ImGui::IsKeyPressed(VK_F7)) {
					stepInto();
				}
			}

			if (!m_isStopped || m_showContexts) {
				Show(m_execCtxViewerWin);
				Show(m_stackViewerWin);
				Show(m_valueViewerWin);
			}
		}

		void defineNextInstrOffset(bool jmp) {
			if (jmp && m_execCtx.m_nextInstrOffset != 0) {
				m_offset = m_execCtx.m_nextInstrOffset;
				m_execCtx.m_nextInstrOffset = 0;
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

		void stepNextPCodeInstr(bool into) {
			m_vm.execute(m_curInstr);
			defineNextInstrOffset(into ? true : m_curInstr->m_id != CE::Decompiler::PCode::InstructionId::CALL);
			defineCurInstrutction();
			if (m_isStopped)
				return;
			if (isNewOrigInstruction())
				sync();
		}

		void stepNextOrigInstr(bool into) {
			do {
				stepNextPCodeInstr(into);
				if (m_isStopped)
					return;
			} while (!isNewOrigInstruction());
		}

		void stepNextBlockTopNode(bool into) {
			bool isNewBlockTopNode;
			do {
				const auto prevBlockTopNode = m_curBlockTopNode;
				stepNextPCodeInstr(into);
				if (m_isStopped)
					return;
				isNewBlockTopNode = m_curBlockTopNode != prevBlockTopNode;
			} while (m_curBlockTopNode && !isNewBlockTopNode);
		}
		
		void stepOver(bool into) {
			switch(m_stepWidth) {
			case StepWidth::STEP_PCODE_INSTR:
				stepNextPCodeInstr(into);
				break;
			case StepWidth::STEP_ORIGINAL_INSTR:
				stepNextOrigInstr(into);
				break;
			case StepWidth::STEP_CODE_LINE:
				stepNextBlockTopNode(into);
				break;
			}
		}

		void stepInto() {
			stepOver(true);
		}

		bool isNewOrigInstruction() const {
			return m_curInstr->m_orderId == 0;
		}
		
		void sync() {
			std::map<int, CE::Decompiler::DataValue> registers;
			registers = m_execCtx.getRegisters();
			m_execCtx.syncWith(registers);
		}

		bool getLocation(const CE::Decompiler::StoragePath& storagePath, ValueViewerPanel::Location& location) {
			const auto type = storagePath.m_storage.getType();
			if (type == CE::Decompiler::Storage::STORAGE_NONE)
				return false;
			
			if (type == CE::Decompiler::Storage::STORAGE_REGISTER) {
				const auto reg = CE::Decompiler::Register(storagePath.m_storage.getRegisterId());
				location = ValueViewerPanel::Location(reg);
			}
			else {
				CE::Decompiler::DataValue baseAddr = 0;
				const auto reg = type == CE::Decompiler::Storage::STORAGE_STACK ?
					CE::Decompiler::Register(ZYDIS_REGISTER_RSP) : CE::Decompiler::Register(ZYDIS_REGISTER_RIP);
				if (!m_execCtx.getRegisterValue(reg, baseAddr))
					return false;
				location = ValueViewerPanel::Location(baseAddr + storagePath.m_storage.getOffset());
			}

			// offsets (pointer dereferencing)
			for (const auto offset : storagePath.m_offsets) {
				CE::Decompiler::DataValue baseAddr;
				if (location.m_isRegister) {
					if (!m_execCtx.getRegisterValue(location.m_register, baseAddr))
						return false;
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
};