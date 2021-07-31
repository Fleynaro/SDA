#pragma once
#include "controllers/DataTypeManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "managers/SymbolManager.h"
#include "panels/BuiltinInputPanel.h"


namespace GUI
{
	class UserDataTypeEditorPanel : public AbstractPanel
	{
		CE::DataType::IUserDefinedType* m_dataType;
		Input::TextInput m_nameInput;
		PopupBuiltinWindow* m_builtinWin = nullptr;
	public:
		UserDataTypeEditorPanel(CE::DataType::IUserDefinedType* dataType, const std::string& name)
			: AbstractPanel(name), m_dataType(dataType)
		{
			m_nameInput.setInputText(m_dataType->getName());
		}

		void createWindow(AbstractPanel* panel) {
			delete m_builtinWin;
			m_builtinWin = new PopupBuiltinWindow(panel);
			m_builtinWin->getPos() = GetLeftBottom();
			m_builtinWin->open();
		}
	
	protected:
		virtual void renderExtra() = 0;

		virtual void save() = 0;

		void saveName() {
			m_dataType->setName(m_nameInput.getInputText());
		}

	private:
		void renderPanel() override {
			Text::Text(GetGroupName(m_dataType) + " name:").show();
			m_nameInput.show();

			renderExtra();

			NewLine();
			if (Button::StdButton("Save").present()) {
				save();
				m_window->close();
			}
			SameLine();
			if (Button::StdButton("Cancel").present()) {
				m_window->close();
			}

			Show(m_builtinWin);
		}
	};

	class TypedefEditorPanel : public UserDataTypeEditorPanel
	{
		CE::DataType::Typedef* m_dataType;
		CE::DataTypePtr m_refDataType;
		PopupBuiltinWindow* m_builtinWin = nullptr;
	public:
		TypedefEditorPanel(CE::DataType::Typedef* dataType)
			: UserDataTypeEditorPanel(dataType, "Typedef Editor"), m_dataType(dataType)
		{
			m_refDataType = dataType->getRefType();
		}

	protected:
		void renderExtra() override;

		void save() override {
			saveName();
			m_dataType->setRefType(m_refDataType);
			m_dataType->getTypeManager()->getProject()->getTransaction()->markAsDirty(m_dataType);
		}
	};

	class EnumEditorPanel : public UserDataTypeEditorPanel
	{
		class FieldTableListView : public TableListView<int>
		{
			EnumEditorPanel* m_enumEditorPanel;
		public:
			FieldTableListView(EnumEditorPanel* enumEditorPanel)
				: TableListView<int>(&enumEditorPanel->m_listModel), m_enumEditorPanel(enumEditorPanel)
			{
				m_colsInfo = {
					ColInfo("Name"),
					ColInfo("Value", ImGuiTableColumnFlags_WidthFixed, 50.0f)
				};
			}

		private:
			void renderColumn(const std::string& colText, const ColInfo* colInfo, const int& value) override {
				ImGui::BeginGroup();
				ImGui::PushID(value);
				if (ImGui::Selectable(colText.c_str(), m_enumEditorPanel->m_selectedValue == value,
					ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_SpanAllColumns)) {
					m_enumEditorPanel->m_selectedValue = value;
				}
				ImGui::PopID();
				ImGui::EndGroup();

				const auto events = GenericEvents(true);
				if (events.isHovered()) {
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				}
				if (events.isClickedByLeftMouseBtn()) {
					if (colInfo->m_name == "Name") {
						const auto panel = new BuiltinTextInputPanel(colText);
						panel->handler([&, panel, value](const std::string& name)
							{
								m_enumEditorPanel->m_fields[value] = name;
								panel->m_window->close();
							});
						m_enumEditorPanel->createWindow(panel);
					}
					else if (colInfo->m_name == "Value") {
						const auto panel = new BuiltinIntegerInputPanel(value);
						panel->handler([&, panel, value](const int& newValue)
							{
								m_enumEditorPanel->m_fields[newValue] = m_enumEditorPanel->m_fields[value];
								m_enumEditorPanel->m_fields.erase(value);
								panel->m_window->close();
							});
						m_enumEditorPanel->createWindow(panel);
					}
				}
			}
		};
		
		CE::DataType::Enum* m_dataType;
		CE::DataType::Enum::FieldMapType m_fields;
		int m_selectedValue = 0;
		EnumFieldListModel m_listModel;
		FieldTableListView* m_tableListView;
	public:
		EnumEditorPanel(CE::DataType::Enum* dataType)
			: UserDataTypeEditorPanel(dataType, "Enum Editor"), m_dataType(dataType), m_listModel(&m_fields), m_fields(m_dataType->getFields())
		{
			m_tableListView = new FieldTableListView(this);
		}

		~EnumEditorPanel() {
			delete m_tableListView;
		}

	protected:
		void renderExtra() override {
			NewLine();
			Text::Text("Fields:").show();
			ImGui::BeginChild("fields", ImVec2(0, 200), true);
			m_tableListView->show();
			ImGui::EndChild();
			
			if (Button::StdButton("Add new field").present()) {
				addNewField();
			}
			if(m_fields.find(m_selectedValue) != m_fields.end()) {
				SameLine();
				if (Button::StdButton("x").present()) {
					m_fields.erase(m_selectedValue);
				}
			}
			NewLine();
			Text::Text("Click the left mouse button hovering on a value you wish to edit.").show();
		}

		void save() override {
			saveName();
			m_dataType->setFields(m_fields);
			m_dataType->getTypeManager()->getProject()->getTransaction()->markAsDirty(m_dataType);
		}

		void addNewField() {
			int freeValue = 0;
			if (!m_fields.empty())
				freeValue = m_fields.rbegin()->first + 1;
			m_fields[freeValue] = "newField_" + std::to_string(freeValue);
		}
	};

	class StructureEditorPanel : public UserDataTypeEditorPanel
	{
		class FieldTableListView : public TableListView<CE::Symbol::StructFieldSymbol*>
		{
			StructureEditorPanel* m_structEditorPanel;
		public:
			FieldTableListView(StructureEditorPanel* structEditorPanel)
				: TableListView<CE::Symbol::StructFieldSymbol*>(&structEditorPanel->m_listModel), m_structEditorPanel(structEditorPanel)
			{
				m_colsInfo = {
					ColInfo("Offset", ImGuiTableColumnFlags_WidthFixed, 60.0f),
					ColInfo("Length", ImGuiTableColumnFlags_WidthFixed, 60.0f),
					ColInfo("DataType"),
					ColInfo("Name")
				};
			}

		private:
			void renderColumn(const std::string& colText, const ColInfo* colInfo,
				CE::Symbol::StructFieldSymbol* const& field) override;
		};

		CE::DataType::IStructure* m_dataType;
		CE::DataType::IStructure* m_clonedDataType;
		Input::IntegerInput m_sizeInput;
		Input::BoolInput m_hexView;
		StructureFieldListModel m_listModel;
		FieldTableListView* m_tableListView;
		int m_selectedFieldOffset = -1;
		CE::DataTypePtr m_selectedBitFieldDataType;
		PopupModalWindow* m_errorWindow = nullptr;
		PopupBuiltinWindow* m_builtinWin = nullptr;
	public:
		// todo: slow work. solution is to use clipper
		
		StructureEditorPanel(CE::DataType::IStructure* dataType)
			: UserDataTypeEditorPanel(dataType, "Structure Editor"),
		m_dataType(dataType), m_clonedDataType(dataType->clone()), m_listModel(&m_clonedDataType->getFields())
		{
			m_sizeInput.setInputValue(dataType->getSize());
			m_tableListView = new FieldTableListView(this);
			m_hexView = Input::BoolInput("Hex view");
		}

		~StructureEditorPanel() {
			delete m_tableListView;
			delete m_errorWindow;
			delete m_builtinWin;
			delete m_clonedDataType;
		}

		CE::Symbol::StructFieldSymbol* getSelectedField() {
			if (m_selectedFieldOffset == -1)
				return nullptr;
			return m_clonedDataType->getFields()[m_selectedFieldOffset];
		}

	private:
		void renderExtra() override {
			Text::Text("Size of structure:").show();
			m_sizeInput.show();
			if(m_sizeInput.isValueEntering()) {
				const auto minSize = m_clonedDataType->getFields().getSizeByLastField();
				if(m_sizeInput.getInputValue() < minSize) {
					m_sizeInput.setInputValue(minSize);
				}
				else {
					m_clonedDataType->getFields().setSize(m_sizeInput.getInputValue());
				}
			}

			NewLine();
			Text::Text("Fields:").show();
			ImGui::BeginChild("fields", ImVec2(0, 200), true);
			m_tableListView->show();
			ImGui::EndChild();
			
			if (Button::StdButton("+").present()) {
				addNewField();
			}
			if (const auto selectedField = getSelectedField()) {
				SameLine();
				if (Button::ButtonArrow(ImGuiDir_Up).present()) {
					if (moveField(selectedField, -1))
						m_selectedFieldOffset = selectedField->getAbsBitOffset();
				}
				SameLine();
				if (Button::ButtonArrow(ImGuiDir_Down).present()) {
					if (moveField(selectedField, 1))
						m_selectedFieldOffset = selectedField->getAbsBitOffset();
				}
				SameLine();
				if (Button::StdButton("x").present()) {
					m_clonedDataType->getFields().removeField(selectedField->getAbsBitOffset());
					delete selectedField;
					m_selectedFieldOffset = -1;
				}
				else if(!selectedField->isBitField()) {
					const auto fieldBaseDataType = selectedField->getDataType()->getBaseType();
					if (fieldBaseDataType->getGroup() == CE::DataType::IType::Simple ||
						fieldBaseDataType->getGroup() == CE::DataType::IType::Enum) {
						SameLine();
						if (Button::StdButton("Make bitfield").present()) {
							selectedField->setBitSize(1);
						}
					}
				}
			}
			SameLine();
			m_hexView.show();
			if(m_hexView.isClicked()) {
				m_listModel.m_hexView = m_hexView.isSelected();
			}
			NewLine();
			Text::Text("Click the left mouse button hovering on a value you wish to edit.").show();
			Text::Text("The offset column for bit fields is presented as: {byte offset}:{bit offset}:{bit size}").show();
			
			Show(m_errorWindow);
			Show(m_builtinWin);
		}

		void save() override {
			saveName();

			for (const auto& [offset, field] : m_dataType->getFields())
				field->getManager()->getProject()->getTransaction()->markAsRemoved(field);
			
			for (const auto& [offset, field] : m_clonedDataType->getFields()) {
				field->getManager()->getProject()->getTransaction()->markAsNew(field);
			}
			
			m_dataType->apply(m_clonedDataType);
			m_dataType->getTypeManager()->getProject()->getTransaction()->markAsDirty(m_dataType);
		}

		void addNewField() {
			if (m_selectedFieldOffset != -1) {
				const auto newOffset = m_selectedFieldOffset;
				if (!m_selectedBitFieldDataType && m_clonedDataType->getFields().areEmptyFields(newOffset, 0x8)) {
					addNewField(newOffset);
				}
				else if (m_clonedDataType->getFields().areEmptyFields(newOffset, 0x1)) {
					// add bit field
					addNewField(newOffset, 0x1, m_selectedBitFieldDataType);
				}
				else {
					delete m_errorWindow;
					m_errorWindow = CreateMessageWindow("No space to place a new field.");
				}
				return;
			}

			const auto lastField = m_clonedDataType->getFields().getLastField();
			if (lastField) {
				const auto newOffset = lastField->getAbsBitOffset() + lastField->getBitSize();
				if (m_clonedDataType->getFields().areEmptyFields(newOffset, 0x8)) {
					addNewField(newOffset);
				}
				else {
					delete m_errorWindow;
					m_errorWindow = CreateMessageWindow("No space to place a new field.\nIncrease size of the structure.");
				}
			}
			else {
				addNewField(0x0);
			}
		}
		
		void addNewField(int absBitOffset, int bitSize = 0x8, CE::DataTypePtr fieldDataType = nullptr) {
			const auto factory = m_dataType->getTypeManager()->getProject()->getSymbolManager()->getFactory(false);
			if(!fieldDataType)
				fieldDataType = m_dataType->getTypeManager()->getDefaultType(0x1);
			const auto field = factory.createStructFieldSymbol(bitSize, fieldDataType, "newField");
			m_clonedDataType->getFields().addField(absBitOffset, field);
		}

		bool moveField(CE::Symbol::StructFieldSymbol* field, int dir) const {
			return m_clonedDataType->getFields().moveField(field, (field->isBitField() ? 0x1 : 0x8) * dir);
		}
	};

	class FuncSigEditorPanel : public UserDataTypeEditorPanel
	{
		class ParamTableListView : public TableListView<CE::Symbol::FuncParameterSymbol*>
		{
			FuncSigEditorPanel* m_sigEditorPanel;
		public:
			ParamTableListView(FuncSigEditorPanel* sigEditorPanel)
				: TableListView<CE::Symbol::FuncParameterSymbol*>(&sigEditorPanel->m_listModel), m_sigEditorPanel(sigEditorPanel)
			{
				m_colsInfo = {
					ColInfo("Index", ImGuiTableColumnFlags_WidthFixed, 50.0f),
					ColInfo("DataType"),
					ColInfo("Name"),
					ColInfo("Storage")
				};
			}

		private:
			void renderColumn(const std::string& colText, const ColInfo* colInfo,
			                  CE::Symbol::FuncParameterSymbol* const& param) override;
		};

		CE::DataType::IFunctionSignature* m_dataType;
		CE::DataType::IFunctionSignature* m_clonedDataType;
		CE::Symbol::FuncParameterSymbol* m_selectedParam = nullptr;
		ParamListModel m_listModel;
		ParamTableListView* m_tableListView;
	public:
		FuncSigEditorPanel(CE::DataType::IFunctionSignature* dataType)
			: UserDataTypeEditorPanel(dataType, "Function Signature Editor"),
		m_dataType(dataType), m_clonedDataType(dataType->clone()), m_listModel(&m_clonedDataType->getParameters())
		{
			m_tableListView = new ParamTableListView(this);
		}

		~FuncSigEditorPanel() {
			delete m_tableListView;
			delete m_clonedDataType;
		}

	protected:
		void renderExtra() override {
			NewLine();
			Text::Text("Params:").show();
			ImGui::BeginChild("params", ImVec2(0, 200), true);
			m_tableListView->show();
			ImGui::EndChild();

			if (Button::StdButton("+").present()) {
				addNewParam();
			}
			if (m_selectedParam) {
				SameLine();
				if (Button::ButtonArrow(ImGuiDir_Up).present()) {
					m_clonedDataType->getParameters().moveParameter(m_selectedParam->getParamIdx(), -1);
					m_clonedDataType->updateParameterStorages();
				}
				SameLine();
				if (Button::ButtonArrow(ImGuiDir_Down).present()) {
					m_clonedDataType->getParameters().moveParameter(m_selectedParam->getParamIdx(), 1);
					m_clonedDataType->updateParameterStorages();
				}
				SameLine();
				if (Button::StdButton("x").present()) {
					m_clonedDataType->getParameters().removeParameter(m_selectedParam->getParamIdx());
					delete m_selectedParam;
					m_clonedDataType->updateParameterStorages();
					m_selectedParam = nullptr;
				}
			}
			
			NewLine();
			Text::Text("Click the left mouse button hovering on a value you wish to edit.").show();
		}

		void save() override {
			saveName();
			
			for (int i = 0; i < m_dataType->getParameters().getParamsCount(); i++) {
				// todo: params could be owned by other objects then mark as removed(no delete). think about removing it correctly
				m_dataType->getTypeManager()->getProject()->getTransaction()->markAsRemoved(m_dataType->getParameters()[i]);
			}

			for (int i = 0; i < m_clonedDataType->getParameters().getParamsCount(); i++) {
				const auto param = m_clonedDataType->getParameters()[i];
				m_dataType->getTypeManager()->getProject()->getTransaction()->markAsNew(param);
			}
			
			m_dataType->apply(m_clonedDataType);
			m_dataType->getTypeManager()->getProject()->getTransaction()->markAsDirty(m_dataType);
		}

		void addNewParam() {
			const auto name = "param_" + std::to_string(m_clonedDataType->getParameters().getParamsCount() + 1);
			const auto dataType = m_dataType->getTypeManager()->getDefaultType(0x4);
			const auto factory = m_clonedDataType->getTypeManager()->getProject()->getSymbolManager()->getFactory(false);
			const auto param = factory.createFuncParameterSymbol(dataType, name);
			m_clonedDataType->getParameters().addParameter(param);
			m_clonedDataType->updateParameterStorages();
		}
	};

	static UserDataTypeEditorPanel* CreateDataTypeEditorPanel(CE::DataType::IUserDefinedType* dataType) {
		if(const auto Typedef = dynamic_cast<CE::DataType::Typedef*>(dataType))
			return new TypedefEditorPanel(Typedef);
		if (const auto Enum = dynamic_cast<CE::DataType::Enum*>(dataType))
			return new EnumEditorPanel(Enum);
		if (const auto Structure = dynamic_cast<CE::DataType::IStructure*>(dataType))
			return new StructureEditorPanel(Structure);
		if (const auto Signature = dynamic_cast<CE::DataType::IFunctionSignature*>(dataType))
			return new FuncSigEditorPanel(Signature);
		return nullptr;
	}
};
