#pragma once
#include "ItemSelector.h"
#include "controllers/ImageManagerController.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Input.h"
#include "imgui_wrapper/controls/Text.h"


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
					delete m_imageContextWindow;
					m_imageContextWindow = new PopupContextWindow(new ImageContextPanel(imageDec));
					m_imageContextWindow->open();
				}
			}
		};
	public:
		ImageManagerController m_controller;
		AbstractListView<CE::ImageDecorator*>* m_listView = nullptr;
		
		ImageManagerPanel(CE::ImageManager* manager)
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

		~ImageManagerPanel() override
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