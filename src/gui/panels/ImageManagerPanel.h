#pragma once
#include "ItemSelector.h"
#include "BuiltinInputPanel.h"
#include "ImageAnalyzerPanel.h"
#include "controllers/AddressSpaceManagerController.h"
#include "controllers/ImageManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "imgui_wrapper/controls/Text.h"
#include "imgui_wrapper/widgets/file_dialog/FileDialog.h"


namespace GUI
{
	class ImageListViewGrouping
		: public ListViewGrouping<CE::ImageDecorator*>
	{
	public:
		using ListViewGrouping<CE::ImageDecorator*>::ListViewGrouping;

	protected:
		bool groupBy(CE::ImageDecorator* const& img1, CE::ImageDecorator* const& img2) override {
			return img1->getAddressSpace() > img2->getAddressSpace();
		}

		bool renderGroupTop(CE::ImageDecorator* const& img, int group_n) override {
			return renderGroupHeader(img->getAddressSpace());
		}

		virtual bool renderGroupHeader(CE::AddressSpace* addrSpace) {
			return ImGui::CollapsingHeader(addrSpace->getName(), ImGuiTreeNodeFlags_DefaultOpen);
		}
	};

	class ImageSelector : public ItemSelector<CE::ImageDecorator>
	{
		ImageManagerController m_controller;
		ImageListViewGrouping* m_imageListViewGrouping;
	public:
		
		ImageSelector(CE::ImageManager* manager)
			: m_controller(manager)
		{
			auto tableListView = new TableListView(&m_controller.m_listModel, {
				ColInfo("Image")
				});
			auto tableListViewMultiselector = new TableListViewMultiSelector(tableListView);
			m_selectedItems = &tableListViewMultiselector->m_selectedItems;
			m_imageListViewGrouping = new ImageListViewGrouping(tableListViewMultiselector);
			m_controller.update();
		}

		~ImageSelector() override
		{
			delete m_imageListViewGrouping;
		}

	private:
		void renderPopupBuiltinWindow() override
		{
			if(!m_controller.hasItems())
			{
				Text::Text("No images.").show();
			}
			m_imageListViewGrouping->show();
		}

		void textEntering(const std::string& text) override
		{
			m_controller.m_filter.m_name = text;
			m_controller.update();
		}
		
		void renderControl() override {
			ItemSelector<CE::ImageDecorator>::renderControl();
			if(m_selectedItems->empty())
				m_search_input.setPlaceHolderText("Click here to select image(-s)");
			else m_search_input.setPlaceHolderText("Selected "+ std::to_string(m_selectedItems->size()) +" image(-s)");
		}
	};
	
	class ImageManagerPanel : public AbstractPanel
	{
		Input::TextInput m_search_input;

		class ImageListView : public ImageListViewGrouping
		{
			class EmptyAddrSpaceListView
				: public AbstractListView<CE::AddressSpace*>
			{
				ImageListView* m_imageListView;
			public:
				EmptyAddrSpaceListView(ImageListView* imageListView)
					: AbstractListView<CE::AddressSpace*>(&imageListView->m_imageManagerPanel->m_addrSpaceController.m_listModel), m_imageListView(imageListView)
				{}

			protected:
				void renderItem(const std::string& text, CE::AddressSpace* const& data, int n) override {
					if (m_imageListView->renderGroupHeader(data))
						Text::Text("No images").show();
				}
			};
			
			class AddrSpaceContextPanel : public AbstractPanel
			{
				class ImageCreatorPanel : public AbstractPanel
				{
				protected:
					AddrSpaceContextPanel* m_ctx;
					Input::TextInput m_nameInput;
					Widget::FileDialog m_fileDialog;
				public:
					ImageCreatorPanel(AddrSpaceContextPanel* ctx)
						: AbstractPanel(std::string("Image Creator (Address space: ") + ctx->m_addrSpace->getName() + ")###ImageCreator"), m_ctx(ctx)
					{
						m_fileDialog = Widget::FileDialog("Choose Image File", ".*");
						m_nameInput.focus();
					}

				private:
					void renderPanel() override {
						Text::Text("Enter a new name for a new image:").show();
						m_nameInput.show();
						NewLine();
						Text::Text("Choose image:").show();
						m_fileDialog.show();
						NewLine();
						if (Button::StdButton("Ok").present()) {
							const auto name = m_nameInput.getInputText();
							try {
								m_ctx->m_imageManagerPanel->m_controller.createImage(m_ctx->m_addrSpace, name, m_fileDialog.getPath());
							} catch(WarningException& ex) {
								delete m_ctx->m_imageManagerPanel->m_messageWindow;
								m_ctx->m_imageManagerPanel->m_messageWindow = CreateMessageWindow(ex.what());
							}
							m_window->close();
						}
						SameLine();
						if (Button::StdButton("Cancel").present())
							m_window->close();
						
					}
				};

				ImageManagerPanel* m_imageManagerPanel;
				CE::AddressSpace* m_addrSpace;
				ImVec2 m_winPos;
			public:
				AddrSpaceContextPanel(CE::AddressSpace* addrSpace, ImageManagerPanel* imageManagerPanel, ImVec2 winPos)
					: m_addrSpace(addrSpace), m_imageManagerPanel(imageManagerPanel), m_winPos(winPos)
				{}
			
			private:		
				void renderPanel() override {
					if (m_addrSpace->isDebug()) {
						if (ImGui::MenuItem("Create Snapshot")) {
							clone("snapshot");
						}
					} else {
						if (ImGui::MenuItem("Clone")) {
							clone("cloned");
						}
						
						if (ImGui::MenuItem("Create Image")) {
							delete m_imageManagerPanel->m_popupModalWindow;
							m_imageManagerPanel->m_popupModalWindow = new PopupModalWindow(new ImageCreatorPanel(this));
							m_imageManagerPanel->m_popupModalWindow->open();
						}

						if (ImGui::MenuItem("Rename")) {
							delete m_imageManagerPanel->m_popupBuiltinWindow;
							const auto panel = new BuiltinTextInputPanel(m_addrSpace->getName());
							panel->handler([&, panel](const std::string& name)
								{
									m_addrSpace->setName(name);
									m_addrSpace->getAddrSpaceManager()->getProject()->getTransaction()->markAsDirty(m_addrSpace);
									panel->m_window->close();
								});
							m_imageManagerPanel->m_popupBuiltinWindow = new PopupBuiltinWindow(panel);
							m_imageManagerPanel->m_popupBuiltinWindow->getPos() = m_winPos;
							m_imageManagerPanel->m_popupBuiltinWindow->open();
						}
					}
				}

				void clone(const std::string& suffix) const {
					const auto clonedAddrSpace = m_addrSpace->getAddrSpaceManager()->createAddressSpace(std::string(m_addrSpace->getName()) + " " + suffix);
					for(const auto imageDec : m_addrSpace->getImageDecorators()) {
						const auto clonedImageDec = imageDec->getImageManager()->createImage(clonedAddrSpace, imageDec->getType(), imageDec->getName());
						clonedImageDec->copyImageFrom(imageDec->getImage()->getReader());
					}
				}
			};

			class ImageContextPanel : public AbstractPanel
			{
				ImageManagerPanel* m_imageManagerPanel;
				CE::ImageDecorator* m_imageDec;
				ImVec2 m_winPos;
			public:
				ImageContextPanel(CE::ImageDecorator* imageDec, ImageManagerPanel* imageManagerPanel, ImVec2 winPos)
					: m_imageDec(imageDec), m_imageManagerPanel(imageManagerPanel), m_winPos(winPos)
				{}

			private:
				void renderPanel() override {
					if (ImGui::MenuItem("Open")) {
						if (m_imageManagerPanel->m_selectImageEventHandler.isInit())
							m_imageManagerPanel->m_selectImageEventHandler(m_imageDec, true);
					}

					if (!m_imageDec->isDebug()) {
						if (ImGui::MenuItem("Rename")) {
							delete m_imageManagerPanel->m_popupBuiltinWindow;
							const auto panel = new BuiltinTextInputPanel(m_imageDec->getName());
							panel->handler([&, panel](const std::string& name)
								{
									m_imageDec->setName(name);
									m_imageDec->getImageManager()->getProject()->getTransaction()->markAsDirty(m_imageDec);
									panel->m_window->close();
								});
							m_imageManagerPanel->m_popupBuiltinWindow = new PopupBuiltinWindow(panel);
							m_imageManagerPanel->m_popupBuiltinWindow->getPos() = m_winPos;
							m_imageManagerPanel->m_popupBuiltinWindow->open();
						}

						if (ImGui::MenuItem("Analyze...")) {
							delete m_imageManagerPanel->m_popupModalWindow;
							m_imageManagerPanel->m_popupModalWindow = new PopupModalWindow(new ImageAnalyzerPanel(m_imageDec));
							m_imageManagerPanel->m_popupModalWindow->open();
						}
					}
				}
			};

			ImageManagerPanel* m_imageManagerPanel;
			EmptyAddrSpaceListView m_emptyAddrSpaceListView;
		public:
			bool m_isContextPanelOpened = false;
			
			ImageListView(StdListView<CE::ImageDecorator*>* listView, ImageManagerPanel* imageManagerPanel)
				: ImageListViewGrouping(listView), m_imageManagerPanel(imageManagerPanel), m_emptyAddrSpaceListView(this)
			{}

		private:
			void renderControl() override
			{
				m_isContextPanelOpened = false;
				ImageListViewGrouping::renderControl();
				m_emptyAddrSpaceListView.show();
			}

			void renderItem(const std::string& text, CE::ImageDecorator* const& imageDec, int n) override {
				ImageListViewGrouping::renderItem(text, imageDec, n);
				const auto events = GenericEvents(true);
				if (events.isClickedByLeftMouseBtn()) {
					if(m_imageManagerPanel->m_selectImageEventHandler.isInit())
						m_imageManagerPanel->m_selectImageEventHandler(imageDec, false);
				}
				if (events.isClickedByRightMouseBtn()) {
					delete m_imageManagerPanel->m_imageContextWindow;
					m_imageManagerPanel->m_imageContextWindow = new PopupContextWindow(new ImageContextPanel(imageDec, m_imageManagerPanel, GetLeftBottom()));
					m_imageManagerPanel->m_imageContextWindow->open();
					m_isContextPanelOpened = true;
				}
			}

			bool renderGroupHeader(CE::AddressSpace* addrSpace) override {
				const auto result = ImageListViewGrouping::renderGroupHeader(addrSpace);
				if (GenericEvents(true).isClickedByRightMouseBtn()) {
					delete m_imageManagerPanel->m_addrSpaceContextWindow;
					m_imageManagerPanel->m_addrSpaceContextWindow = new PopupContextWindow(new AddrSpaceContextPanel(addrSpace, m_imageManagerPanel, GetLeftBottom()));
					m_imageManagerPanel->m_addrSpaceContextWindow->open();
					m_isContextPanelOpened = true;
				}
				return result;
			}
		};

		class ImageManagerContextPanel : public AbstractPanel
		{
			class AddrSpaceCreatorPanel : public AbstractPanel
			{
			protected:
				ImageManagerContextPanel* m_ctx;
				Input::TextInput m_nameInput;
			public:
				AddrSpaceCreatorPanel(ImageManagerContextPanel* ctx)
					: AbstractPanel("Address Space Creator"), m_ctx(ctx)
				{
					m_nameInput.focus();
				}

			private:
				void renderPanel() override {
					Text::Text("Enter a new name for a new address space:").show();
					m_nameInput.show();
					NewLine();
					if (Button::StdButton("Ok").present()) {
						const auto name = m_nameInput.getInputText();
						m_ctx->m_imageManagerPanel->m_addrSpaceController.createAddrSpace(name);
						m_window->close();
					}
					SameLine();
					if (Button::StdButton("Cancel").present())
						m_window->close();
				}
			};
			
			ImageManagerPanel* m_imageManagerPanel;
		public:
			ImageManagerContextPanel(ImageManagerPanel* imageManagerPanel)
				: m_imageManagerPanel(imageManagerPanel)
			{}

		private:
			void renderPanel() override {
				if (ImGui::MenuItem("Create Address Space")) {
					delete m_imageManagerPanel->m_popupModalWindow;
					m_imageManagerPanel->m_popupModalWindow = new PopupModalWindow(new AddrSpaceCreatorPanel(this));
					m_imageManagerPanel->m_popupModalWindow->open();
				}
			}
		};

		PopupContextWindow* m_imageManagerContextWindow = nullptr;
		PopupContextWindow* m_addrSpaceContextWindow = nullptr;
		PopupContextWindow* m_imageContextWindow = nullptr;
		PopupModalWindow* m_popupModalWindow = nullptr;
		PopupBuiltinWindow* m_popupBuiltinWindow = nullptr;
		PopupModalWindow* m_messageWindow = nullptr;
		EventHandler<CE::ImageDecorator*, bool> m_selectImageEventHandler;
	public:
		ImageManagerController m_controller;
		AddressSpaceManagerController m_addrSpaceController;
		ImageListView* m_listView = nullptr;
		
		ImageManagerPanel(CE::ImageManager* manager)
			: AbstractPanel("Image viewer"), m_controller(manager), m_addrSpaceController(manager->getProject()->getAddrSpaceManager())
		{
			m_addrSpaceController.m_filter.m_showOnlyEmpty = true;
			const auto listView = new StdListView(&m_controller.m_listModel);
			m_listView = new ImageListView(listView, this);
		}

		~ImageManagerPanel() override
		{
			delete m_listView;
		}

		void selectImageEventHandler(const std::function<void(CE::ImageDecorator*, bool)>& eventHandler) {
			m_selectImageEventHandler = eventHandler;
		}

	protected:
		void renderPanel() override
		{
			m_controller.update();
			m_addrSpaceController.update();

			renderSearch();
			m_listView->show();
			imageManagerContextWindow();

			Show(m_imageManagerContextWindow);
			Show(m_addrSpaceContextWindow);
			Show(m_imageContextWindow);
			Show(m_popupModalWindow);
			Show(m_popupBuiltinWindow);
			Show(m_messageWindow);
		}

		void renderSearch() {
			if (m_controller.m_manager->getItemsCount() > 0) {
				if (m_search_input.isTextEntering()) {
					m_controller.m_filter.m_name = m_search_input.getInputText();
					m_controller.update();
				}
				m_search_input.show();
			}
		}

		void imageManagerContextWindow() {
			if (ImGui::IsWindowHovered() && !m_listView->m_isContextPanelOpened) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					delete m_imageManagerContextWindow;
					m_imageManagerContextWindow = new PopupContextWindow(new ImageManagerContextPanel(this));
					m_imageManagerContextWindow->open();
				}
			}
		}
	};
};