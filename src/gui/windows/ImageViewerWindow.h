#pragma once
#include "controllers/ImageManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/Input.h"


namespace GUI
{
	class ImageViewerWindow : public Window
	{
		Input::TextInput m_search_input;

		class ImageListViewGrouping
			: public ListViewGrouping<CE::ImageDecorator*>
		{
		public:
			using ListViewGrouping<CE::ImageDecorator*>::ListViewGrouping;

		protected:
			bool groupBy(CE::ImageDecorator*& img1, CE::ImageDecorator*& img2) override
			{
				return img1->getAddressSpace()->getId() > img2->getAddressSpace()->getId();
			}

			bool renderGroupTop(CE::ImageDecorator*& img, int group_n) override
			{
				auto addrSpace = img->getAddressSpace();
				if(ImGui::TreeNodeEx(addrSpace->getName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					GenericEvents events(true);
					if(events.isClickedByRightMouseBtn())
					{
						int a = 5;
					}
					return true;
				}
				return false;
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
			m_listView = new ImageListViewGrouping(listView);
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