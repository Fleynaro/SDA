#pragma once
#include "controllers/ImageManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"


namespace GUI
{
	class ImageListViewGrouping
		: public ListViewGrouping<CE::ImageDecorator*>
	{
	public:
		using ListViewGrouping<CE::ImageDecorator*>::ListViewGrouping;

	protected:
		bool groupBy(CE::ImageDecorator* const& img1, CE::ImageDecorator* const& img2) override
		{
			return img1->getAddressSpace()->getId() > img2->getAddressSpace()->getId();
		}

		bool renderGroupTop(CE::ImageDecorator* const& img, int group_n) override
		{
			auto addrSpace = img->getAddressSpace();
			auto isOpen = ImGui::CollapsingHeader(addrSpace->getName().c_str(), ImGuiTreeNodeFlags_DefaultOpen);
			return isOpen;
		}
	};

	class ImageSelector : public Control
	{
		ImageManagerController m_controller;
		Input::TextInput m_search_input;
		PopupBuiltinWindow* m_popupBuiltinWindow;
		ImageListViewGrouping* m_imageListViewGrouping;
		bool m_focusOnSearchInput = false;
	public:
		std::set<CE::ImageDecorator*>* m_selectedImages;
		
		ImageSelector(CE::ImageManager* manager)
			: m_controller(manager)
		{
			auto tableListView = new TableListView(&m_controller.m_listModel, "images table", {
				ColInfo("Image")
				});
			auto tableListViewMultiselector = new TableListViewMultiSelector(tableListView);
			m_selectedImages = &tableListViewMultiselector->m_selectedItems;
			m_imageListViewGrouping = new ImageListViewGrouping(tableListViewMultiselector);
			m_popupBuiltinWindow = new PopupBuiltinWindow(new StdPanel([&]()
				{
					m_imageListViewGrouping->show();
				}));
			m_controller.update();
		}

		~ImageSelector() override
		{
			delete m_popupBuiltinWindow;
			delete m_imageListViewGrouping;
		}

		void renderControl() override {
			m_popupBuiltinWindow->show();
			
			Text::Text("Select images:").show();
			SameLine();
			if (m_focusOnSearchInput && m_popupBuiltinWindow->isOpened()) {
				ImGui::SetKeyboardFocusHere();
				m_focusOnSearchInput = false;
			}
			m_search_input.show();
			m_popupBuiltinWindow->placeAfterItem();

			if (m_search_input.isTextEntering() || m_search_input.isClickedByLeftMouseBtn()) {
				m_focusOnSearchInput = true;
				m_controller.m_filter.m_name = m_search_input.getInputText();
				m_controller.update();
				m_popupBuiltinWindow->open();
			}
		}
	};
	
	class ImageViewerPanel : public AbstractPanel
	{
		Input::TextInput m_search_input;

		class ImageListView
			: public ImageListViewGrouping
		{
			class AddrSpaceContextPanel : public AbstractPanel
			{
				CE::AddressSpace* m_addrSpace;
			public:
				AddrSpaceContextPanel(CE::AddressSpace* addrSpace)
					: m_addrSpace(addrSpace)
				{}
			
			private:		
				void renderPanel() override
				{
					if (ImGui::MenuItem(m_addrSpace->getName().c_str(), nullptr))
					{
						
					}
				}
			};

			class ImageContextPanel : public AbstractPanel
			{
				CE::ImageDecorator* m_imageDec;
			public:
				ImageContextPanel(CE::ImageDecorator* imageDec)
					: m_imageDec(imageDec)
				{}

			private:
				void renderPanel() override
				{
					if (ImGui::MenuItem(m_imageDec->getName().c_str(), nullptr))
					{
						int a = 5;
					}
				}
			};

			PopupContextWindow* m_addrSpaceContextWindow = nullptr;
			PopupContextWindow* m_imageContextWindow = nullptr;
		public:
			using ImageListViewGrouping::ImageListViewGrouping;

		private:
			void renderControl() override
			{
				ImageListViewGrouping::renderControl();
				Show(m_addrSpaceContextWindow);
				Show(m_imageContextWindow);
			}

			bool renderGroupTop(CE::ImageDecorator* const& img, int group_n) override
			{
				auto isOpen = ImageListViewGrouping::renderGroupTop(img, group_n);
				if (GenericEvents(true).isClickedByRightMouseBtn()) {
					if(m_addrSpaceContextWindow)
						delete m_addrSpaceContextWindow;
					m_addrSpaceContextWindow = new PopupContextWindow(new AddrSpaceContextPanel(img->getAddressSpace()));
					m_addrSpaceContextWindow->open();
				}
				return isOpen;
			}

			void renderItem(const std::string& text, CE::ImageDecorator* const& imageDec, int n) override
			{
				ImageListViewGrouping::renderItem(text, imageDec, n);
				if (GenericEvents(true).isClickedByRightMouseBtn())
				{
					if(m_imageContextWindow)
						delete m_imageContextWindow;
					m_imageContextWindow = new PopupContextWindow(new ImageContextPanel(imageDec));
					m_imageContextWindow->open();
				}
			}
		};
	public:
		ImageManagerController m_controller;
		AbstractListView<CE::ImageDecorator*>* m_listView = nullptr;
		
		ImageViewerPanel(CE::ImageManager* manager)
			: AbstractPanel("Image viewer"), m_controller(manager)
		{
			/*auto tableListView = new TableListView(&m_controller.m_tableListModel, "images table", {
				ColInfo("AddrSpace"),
				ColInfo("Image")
			});
			auto tableListViewMultiselector = new TableListViewMultiSelector(tableListView);
			m_listView = new ImageListViewGrouping(tableListViewMultiselector);*/

			auto listView = new StdListView(&m_controller.m_listModel);
			m_listView = new ImageListView(listView);
			m_controller.update();
		}

		~ImageViewerPanel() override
		{
			delete m_listView;
		}

	protected:
		void renderPanel() override
		{
			if (m_search_input.isTextEntering()) {
				m_controller.m_filter.m_name = m_search_input.getInputText();
				m_controller.update();
			}
			m_search_input.show();

			m_listView->show();
		}
	};
};