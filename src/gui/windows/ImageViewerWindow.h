#pragma once
#include "controllers/ImageManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/Input.h"


namespace GUI
{
	class ImageViewerWindow : public Window
	{
		Input::TextInput m_search_input;

		class ImageListView
			: public ListViewGrouping<CE::ImageDecorator*>
		{
			class AddrSpaceContextWindow : public AbstractPopupContextWindow
			{
				CE::AddressSpace* m_addrSpace;
			public:
				AddrSpaceContextWindow(CE::AddressSpace* addrSpace)
					: m_addrSpace(addrSpace)
				{}
			
			private:		
				void renderWindow() override
				{
					if (ImGui::MenuItem(m_addrSpace->getName().c_str(), nullptr))
					{
						
					}
				}
			};

			class ImageContextWindow : public AbstractPopupContextWindow
			{
				CE::ImageDecorator* m_imageDec;
			public:
				ImageContextWindow(CE::ImageDecorator* imageDec)
					: m_imageDec(imageDec)
				{}

			private:
				void renderWindow() override
				{
					if (ImGui::MenuItem(m_imageDec->getName().c_str(), nullptr))
					{
						
					}
				}
			};

			AddrSpaceContextWindow* m_addrSpaceContextWindow = nullptr;
			ImageContextWindow* m_imageContextWindow = nullptr;
			PopupModalWindow* m_popupModalWin = nullptr;
		public:
			using ListViewGrouping<CE::ImageDecorator*>::ListViewGrouping;

		private:
			void renderControl() override
			{
				ListViewGrouping<CE::ImageDecorator*>::renderControl();
				Show(m_addrSpaceContextWindow);
				Show(m_imageContextWindow);
				Show(m_popupModalWin);
			}
			
			bool groupBy(CE::ImageDecorator* const& img1, CE::ImageDecorator* const& img2) override
			{
				return img1->getAddressSpace()->getId() > img2->getAddressSpace()->getId();
			}

			bool renderGroupTop(CE::ImageDecorator* const& img, int group_n) override
			{
				auto addrSpace = img->getAddressSpace();
				auto isOpen = ImGui::CollapsingHeader(addrSpace->getName().c_str(), ImGuiTreeNodeFlags_DefaultOpen);
				if (GenericEvents(true).isClickedByRightMouseBtn()) {
					if (m_addrSpaceContextWindow)
						delete m_addrSpaceContextWindow;
					m_addrSpaceContextWindow = new AddrSpaceContextWindow(addrSpace);
					m_addrSpaceContextWindow->open();
					
					m_popupModalWin = new PopupModalWindow("popup modal win");
					m_popupModalWin->open();
					m_popupModalWin->handler([&]()
						{
							Text::Text("opened!").show();
						});
				}
				return isOpen;
			}

			void renderItem(const std::string& text, CE::ImageDecorator* const& imageDec, int n) override
			{
				ListViewGrouping<CE::ImageDecorator*>::renderItem(text, imageDec, n);
				if (GenericEvents(true).isClickedByRightMouseBtn())
				{
					if (m_imageContextWindow)
						delete m_imageContextWindow;
					m_imageContextWindow = new ImageContextWindow(imageDec);
					m_imageContextWindow->open();
				}
			}
		};
	public:
		ImageManagerController m_controller;
		AbstractListView<CE::ImageDecorator*>* m_listView = nullptr;
		
		ImageViewerWindow(CE::ImageManager* manager)
			: Window("Image viewer"), m_controller(manager)
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

		~ImageViewerWindow() override
		{
			delete m_listView;
		}

	protected:
		void renderWindow() override
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