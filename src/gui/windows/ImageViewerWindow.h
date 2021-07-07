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
				void open(CE::AddressSpace* addrSpace)
				{
					m_addrSpace = addrSpace;
					openPopup();
				}
			
			private:
				void renderMenu() override
				{
					if (ImGui::MenuItem(m_addrSpace->getName().c_str(), nullptr))
					{
						int a = 5;
					}
				}
			};

			AddrSpaceContextWindow m_addrSpaceContextWindow;
		public:
			using ListViewGrouping<CE::ImageDecorator*>::ListViewGrouping;

		private:
			void renderControl() override
			{
				ListViewGrouping<CE::ImageDecorator*>::renderControl();
				m_addrSpaceContextWindow.show();
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
					m_addrSpaceContextWindow.open(addrSpace);
				}
				return isOpen;
			}

			void renderItem(const std::string& text, CE::ImageDecorator* const& imageDec, int n) override
			{
				//renderContextMenuForImage(imageDec);
				ListViewGrouping<CE::ImageDecorator*>::renderItem(text, imageDec, n);
				if (GenericEvents(true).isClickedByRightMouseBtn())
				{
					//ImGui::OpenPopup("ctx_menu_image");
					//ImGui::CloseCurrentPopup();
				}
			}

			void renderContextMenuForAddrSpace(CE::AddressSpace* addrSpace) const
			{
				if (ImGui::BeginPopupContextWindow("ctx_menu_addr_space"))
				{
					if (ImGui::MenuItem("AddrSpace", nullptr))
					{
						int a = 5;
					}
					ImGui::EndPopup();
				}
			}

			void renderContextMenuForImage(CE::ImageDecorator* imageDec) const
			{
				if (ImGui::BeginPopupContextWindow("ctx_menu_image"))
				{
					if (ImGui::MenuItem("Image", nullptr))
					{
						int a = 5;
					}
					ImGui::EndPopup();
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