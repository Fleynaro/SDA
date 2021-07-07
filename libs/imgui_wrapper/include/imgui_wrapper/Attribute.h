#pragma once

namespace GUI
{
	namespace Attribute
	{
		class Pos
		{
		public:
			Pos(float posX = 0.f, float posY = 0.f)
				: m_posX(posX), m_posY(posY)
			{};

			float getPosX() {
				return m_posX;
			}

			float getPosY() {
				return m_posY;
			}

			void setPosX(float value) {
				m_posX = value;
			}

			void setPosY(float value) {
				m_posY = value;
			}

			void pushPosParam() {
				if (getPosX() == 0.f)
					return;
				ImGui::SetNextWindowPos(ImVec2(m_posX, m_posY));
			}
		protected:
			float m_posX;
			float m_posY;
		};

		class Width
		{
		public:
			Width(float width = 0.f)
				: m_width(width)
			{};

			float getWidth() {
				return m_width;
			}

			void setWidth(float value) {
				m_width = value;
			}

			void pushWidthParam() {
				if (getWidth() == 0.f)
					return;
				ImGui::PushItemWidth(getWidth());
			}

			void popWidthParam() {
				if (getWidth() == 0.f)
					return;
				ImGui::PopItemWidth();
			}
		protected:
			float m_width;
		};

		class Height
		{
		public:
			Height(float height = 0.f)
				: m_height(height)
			{};

			float getHeight() {
				return m_height;
			}

			void setHeight(float value) {
				m_height = value;
			}
		protected:
			float m_height;
		};

		class Font
		{
		public:
			Font(ImFont* font = nullptr)
				: m_font(font)
			{};

			ImFont* getFont() {
				return m_font;
			}

			void setFont(ImFont* font) {
				m_font = font;
			}

			void pushFontParam() {
				if (getFont() == nullptr)
					return;
				ImGui::PushFont(getFont());
			}

			void popFontParam() {
				if (getFont() == nullptr)
					return;
				ImGui::PopFont();
			}
		private:
			ImFont* m_font;
		};

		class Id
		{
			std::string m_id;
		public:
			Id(const std::string& id = "")
				: m_id(id)
			{
				setId(this);
			};

			std::string getId() {
				return m_id;
			}

			template<typename T>
			void setId(T id) {
				m_id = std::to_string(uint64_t(id));
			}

			void pushIdParam() {
				ImGui::PushID(m_id.c_str());
			}

			void popIdParam() {
				ImGui::PopID();
			}
		};

		template<typename T, T defFlag = 0>
		class Flags
		{
		public:
			Flags(T flags = defFlag)
				: m_flags(flags)
			{};

			bool isFlags(T flags) {
				return (m_flags & flags) != 0;
			}

			T getFlags() {
				return m_flags;
			}

			void setFlags(T flags) {
				m_flags = flags;
			}

			void addFlags(int flags, bool toggle = true) {
				setFlags(T(getFlags() & ~flags | flags * toggle));
			}
		private:
			T m_flags;
		};

		class Name
		{
		public:
			Name(const std::string& name)
				: m_name(name)
			{};

			const std::string& getName() {
				return m_name;
			}

			void setName(std::string value) {
				m_name = value;
			}
		private:
			std::string m_name;
		};

		/*
		class Collapse
		{
		public:
			Collapse(bool open) : m_open(open) {};

			bool isOpen() {
				return m_open;
			}

			T& setOpen(bool state) {
				m_open = state;
				return *(T*)this;
			}

			T* open() {
				setOpen(true);
				return (T*)this;
			}

			T* close() {
				setOpen(false);
				return (T*)this;
			}
		protected:
			bool m_open = true;
		};

		class Enable
		{
		public:
			Enable(bool state) : m_enabled(state) {};

			bool isEnabled() {
				return m_enabled;
			}

			T& setEnable(bool state) {
				m_enabled = state;
				return *(T*)this;
			}

			T* enable() {
				setEnable(true);
				return (T*)this;
			}

			T* disable() {
				setEnable(false);
				return (T*)this;
			}
		protected:
			bool m_enabled = true;
		};

		class Select
		{
		public:
			Select(bool state) : m_selected(state) {};

			bool isSelected() {
				return m_selected;
			}

			T& setSelected(bool state) {
				m_selected = state;
				return *(T*)this;
			}

			T* select() {
				setSelected(true);
				return (T*)this;
			}

			T* unselect() {
				setSelected(false);
				return (T*)this;
			}
		protected:
			bool m_selected = true;
		};

		class ScrollbarY
		{
		public:
			ScrollbarY(float scrollBarY = -1.f)
				: m_scrollBarY(scrollBarY)
			{};

			float getScrollbarY() {
				return m_scrollBarY;
			}

			T* setScrollbarY(float value) {
				m_scrollBarY = value;
				return (T*)this;
			}

			T* setScrollbarToTop() {
				return setScrollbarY(0.f);
			}

			T* setScrollbarToBottom() {
				return setScrollbarY(-2.f);
			}

			void setScrollbarYParam() {
				if (m_scrollBarY == -1.f)
					return;
				if (m_scrollBarY == -2.f)
					ImGui::SetScrollY(ImGui::GetScrollMaxY());
				else ImGui::SetScrollY(m_scrollBarY);
			}
		protected:
			float m_scrollBarY;
		};

		template<typename T>
		class ScrollbarX
		{
		public:
			ScrollbarX(float scrollBarX = -1.f)
				: m_scrollBarX(scrollBarX)
			{};

			float getScrollbarX() {
				return m_scrollBarX;
			}

			T* setScrollbarY(float value) {
				m_scrollBarX = value;
				return (T*)this;
			}
		protected:
			float m_scrollBarX;
		};

		template<typename T, int Length = 40>
		class Rename
		{
		public:
			Rename(std::string newName = "")
				: m_newName(newName)
			{}

			void renderInput() {
				if (ImGui::InputText("##dirToRename", getInputName().data(), Length, ImGuiInputTextFlags_EnterReturnsTrue)) {
					enterInput();
				}
			}

			virtual void enterInput() = 0;

			std::string getInputName() {
				return m_newName.c_str();
			}

			void setInputName(std::string name) {
				getInputName() = name;
			}
		protected:
			std::string m_newName;
		};

		template<typename T>
		class Hint
		{
		public:
			Hint(std::string text = "")
				: m_shortcutText(text)
			{}

			void showHint() {
				auto text = getHintText();
				if (!text.empty()) {
					ImGui::SetTooltip(getHintText().c_str());
				}
			}

			virtual std::string getHintText() {
				return m_shortcutText;
			}

			T* setHintText(std::string text) {
				m_shortcutText = text;
				return (T*)this;
			}
		protected:
			std::string m_shortcutText;
		};
		*/
	};
};