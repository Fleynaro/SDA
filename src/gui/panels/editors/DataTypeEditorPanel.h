#pragma once
#include "controllers/DataTypeManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "managers/SymbolManager.h"


namespace GUI
{
	class UserDataTypeEditorPanel : public AbstractPanel
	{
		CE::DataType::IUserDefinedType* m_dataType;
		Input::TextInput m_nameInput;
	public:
		UserDataTypeEditorPanel(CE::DataType::IUserDefinedType* dataType, const std::string& name)
			: AbstractPanel(name), m_dataType(dataType)
		{
			m_nameInput.setInputText(m_dataType->getName());
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
		class FieldEditorPanel : public AbstractPanel
		{
			CE::DataType::Enum::FieldMapType* m_fields;
			int m_value;
			bool m_newObject;
			Input::TextInput m_nameInput;
			Input::IntegerInput m_valueInput;
		public:
			FieldEditorPanel(CE::DataType::Enum::FieldMapType* fields, int value, bool newObject = false)
				: AbstractPanel("Field Editor"), m_fields(fields), m_value(value), m_newObject(newObject)
			{
				if(!m_newObject)
					m_nameInput.setInputText((*m_fields)[value]);
				m_valueInput.setInputValue(value);
			}

		private:
			void renderPanel() override {
				Text::Text("Enter name:").show();
				m_nameInput.show();
				NewLine();
				Text::Text("Enter value:").show();
				m_valueInput.show();
				NewLine();
				if (Button::StdButton("Ok").present()) {
					const auto value = m_valueInput.getInputValue();
					const auto name = m_nameInput.getInputText();
					if(!m_newObject)
						m_fields->erase(m_value);
					(*m_fields)[value] = name;
					m_window->close();
				}
				if (!m_newObject) {
					SameLine();
					if (Button::StdButton("Remove").present()) {
						m_fields->erase(m_value);
						m_window->close();
					}
				}
				SameLine();
				if (Button::StdButton("Cancel").present())
					m_window->close();

			}
		};
		
		CE::DataType::Enum* m_dataType;
		PopupModalWindow* m_fieldEditorWin = nullptr;
		CE::DataType::Enum::FieldMapType m_fields;
		EnumFieldListModel m_listModel;
		TableListViewSelector<int>* m_tableListView;
	public:
		EnumEditorPanel(CE::DataType::Enum* dataType)
			: UserDataTypeEditorPanel(dataType, "Enum Editor"), m_dataType(dataType), m_listModel(&m_fields)
		{
			m_fields = m_dataType->getFields();
			const auto listView = new TableListView(&m_listModel, {
				ColInfo("Name"),
				ColInfo("Value", ImGuiTableColumnFlags_WidthFixed, 50.0f)
				});
			m_tableListView = new TableListViewSelector<int>(listView, new Button::StdButton("Edit"));
			m_tableListView->handler([&](int value)
				{
					delete m_fieldEditorWin;
					const auto panel = new FieldEditorPanel(&m_fields, value);
					m_fieldEditorWin = new PopupModalWindow(panel);
					m_fieldEditorWin->open();
				});
		}

		~EnumEditorPanel() {
			delete m_tableListView;
		}

	protected:
		void renderExtra() override {
			Text::Text("Fields:").show();
			m_tableListView->show();
			if (Button::StdButton("Add new field").present()) {
				delete m_fieldEditorWin;
				const auto panel = new FieldEditorPanel(&m_fields, 0, true);
				m_fieldEditorWin = new PopupModalWindow(panel);
				m_fieldEditorWin->open();
			}
			Show(m_fieldEditorWin);
		}

		void save() override {
			saveName();
			m_dataType->setFields(m_fields);
			m_dataType->getTypeManager()->getProject()->getTransaction()->markAsDirty(m_dataType);
		}
	};

	class StructureEditorPanel : public UserDataTypeEditorPanel
	{
		class FieldTableListView : public TableListView<CE::DataType::IStructure::Field*>
		{
			StructureEditorPanel* m_structEditorPanel;
		public:
			FieldTableListView(StructureEditorPanel* structEditorPanel)
				: TableListView<CE::DataType::IStructure::Field*>(&structEditorPanel->m_listModel), m_structEditorPanel(structEditorPanel)
			{
				m_colsInfo = {
					ColInfo("Offset"),
					ColInfo("Bit Offset"),
					ColInfo("Length"),
					ColInfo("DataType"),
					ColInfo("Name")
				};
			}

		private:
			void renderColumn(const std::string& colText, const ColInfo* colInfo,
			                  CE::DataType::IStructure::Field* const& field) override;

			void createWindow(AbstractPanel* panel) {
				delete m_structEditorPanel->m_builtinWin;
				m_structEditorPanel->m_builtinWin = new PopupBuiltinWindow(panel);
				m_structEditorPanel->m_builtinWin->getPos() = GetLeftBottom();
				m_structEditorPanel->m_builtinWin->open();
			}
		};

		CE::DataType::IStructure* m_dataType;
		PopupModalWindow* m_fieldEditorWin = nullptr;
		CE::DataType::IStructure::FieldMapType m_fields;
		StructureFieldListModel m_listModel;
		FieldTableListView* m_tableListView;
		CE::DataType::IStructure::Field* m_selectedField = nullptr;
		std::set<CE::DataType::IStructure::Field*> m_newFields;
		std::set<CE::DataType::IStructure::Field*> m_removedFields;
		PopupBuiltinWindow* m_builtinWin = nullptr;
	public:
		StructureEditorPanel(CE::DataType::IStructure* dataType)
			: UserDataTypeEditorPanel(dataType, "Structure Editor"), m_dataType(dataType), m_listModel(&m_fields)
		{
			m_fields = m_dataType->getFields();
			m_tableListView = new FieldTableListView(this);
		}

		~StructureEditorPanel() {
			delete m_tableListView;
		}

	protected:
		void renderExtra() override {
			Text::Text("Fields:").show();

			ImGui::BeginChild("fields", ImVec2(0, 200));
			m_tableListView->show();
			ImGui::EndChild();
			
			if (Button::StdButton("+").present()) {
				int newOffset = 0;
				if (!m_fields.empty()) {
					const auto lastField = m_fields.rbegin()->second;
					newOffset = lastField->getAbsBitOffset() + lastField->getBitSize();
				}
				const auto factory = m_dataType->getTypeManager()->getProject()->getSymbolManager()->getFactory(false);
				const auto defType = m_dataType->getTypeManager()->getDefaultType(0x1);
				const auto field = factory.createStructFieldSymbol(newOffset, defType->getSize() * 0x8, m_dataType, defType, "newField");
				m_newFields.insert(field);
				m_fields[newOffset] = field;
			}
			if (m_selectedField) {
				SameLine();
				if (Button::ButtonArrow(ImGuiDir_Up).present()) {

				}
				SameLine();
				if (Button::ButtonArrow(ImGuiDir_Down).present()) {

				}
				SameLine();
				if (Button::StdButton("x").present()) {
					m_fields.erase(m_selectedField->getAbsBitOffset());
					if (m_newFields.find(m_selectedField) != m_newFields.end()) {
						m_newFields.erase(m_selectedField);
						delete m_selectedField;
					} else {
						m_removedFields.insert(m_selectedField);
					}
					m_selectedField = nullptr;
				}
			}
			Show(m_fieldEditorWin);
			Show(m_builtinWin);
		}

		void save() override {
			saveName();
			m_dataType->setFields(m_fields);
			for (const auto newField : m_newFields)
				newField->getManager()->getProject()->getTransaction()->markAsNew(newField);
			for (const auto newField : m_removedFields)
				newField->getManager()->getProject()->getTransaction()->markAsRemoved(newField);
			m_dataType->getTypeManager()->getProject()->getTransaction()->markAsDirty(m_dataType);
		}
	};

	static UserDataTypeEditorPanel* CreateDataTypeEditorPanel(CE::DataType::IUserDefinedType* dataType) {
		if(const auto Typedef = dynamic_cast<CE::DataType::Typedef*>(dataType))
			return new TypedefEditorPanel(Typedef);
		if (const auto Enum = dynamic_cast<CE::DataType::Enum*>(dataType))
			return new EnumEditorPanel(Enum);
		if (const auto Structure = dynamic_cast<CE::DataType::IStructure*>(dataType))
			return new StructureEditorPanel(Structure);
		return nullptr;
	}
};
