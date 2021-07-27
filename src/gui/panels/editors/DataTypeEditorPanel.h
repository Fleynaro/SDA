#pragma once
#include "controllers/DataTypeManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"


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

		void saveName(UserDataTypeController* controller) {
			controller->rename(m_nameInput.getInputText());
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
			auto controller = TypedefController(m_dataType);
			saveName(&controller);
			controller.changeRefType(m_refDataType);
		}
	};

	class EnumEditorPanel : public UserDataTypeEditorPanel
	{
		class FieldListModel : public IListModel<int>
		{
			EnumEditorPanel* m_enumEditorPanel;
		public:
			class FieldIterator : public Iterator
			{
				CE::DataType::Enum::FieldMapType::iterator m_it;
				CE::DataType::Enum::FieldMapType* m_fields;
			public:
				FieldIterator(CE::DataType::Enum::FieldMapType* fields)
					: m_fields(fields), m_it(fields->begin())
				{}

				void getNextItem(std::string* text, int* data) override {
					*text = m_it->second + "," + std::to_string(m_it->first);
					*data = m_it->first;
					++m_it;
				}

				bool hasNextItem() override {
					return m_it != m_fields->end();
				}
			};

			FieldListModel(EnumEditorPanel* enumEditorPanel)
				: m_enumEditorPanel(enumEditorPanel)
			{}

			void newIterator(const IteratorCallback& callback) override
			{
				FieldIterator iterator(&m_enumEditorPanel->m_fields);
				callback(&iterator);
			}
		};

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
		FieldListModel m_listModel;
		TableListViewSelector<int>* m_tableListView;
	public:
		EnumEditorPanel(CE::DataType::Enum* dataType)
			: UserDataTypeEditorPanel(dataType, "Enum Editor"), m_dataType(dataType), m_listModel(this)
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
			auto controller = EnumController(m_dataType);
			saveName(&controller);
			controller.change(m_fields);
		}
	};

	static UserDataTypeEditorPanel* CreateDataTypeEditorPanel(CE::DataType::IUserDefinedType* dataType) {
		if(const auto Typedef = dynamic_cast<CE::DataType::Typedef*>(dataType))
			return new TypedefEditorPanel(Typedef);
		if (const auto Enum = dynamic_cast<CE::DataType::Enum*>(dataType))
			return new EnumEditorPanel(Enum);
		return nullptr;
	}
};
