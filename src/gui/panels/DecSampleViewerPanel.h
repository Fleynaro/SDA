#pragma once
#include "DecompiledCodeViewerPanel.h"
#include "decompiler_test_lib.h"
#include "ImageContentViewerPanel.h"
#include "Project.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/List.h"

namespace GUI
{
	class DecSampleViewerPanel : public AbstractPanel
	{
		class ListModel : public IListModel<CE::DecTestSamplesPool::Sample*>
		{
			CE::DecTestSamplesPool* m_decTestSamplesPool;
		public:
			class Iterator : public IListModel<CE::DecTestSamplesPool::Sample*>::Iterator
			{
				std::list<CE::DecTestSamplesPool::Sample*>::iterator m_it;
			protected:
				ListModel* m_listModel;
			public:
				Iterator(ListModel* listModel)
					: m_listModel(listModel), m_it(listModel->m_decTestSamplesPool->m_samples.begin())
				{}

				void getNextItem(std::string* text, CE::DecTestSamplesPool::Sample** data) override {
					*text = (*m_it)->m_name + ", " + (*m_it)->m_comment;
					*data = *m_it;
					++m_it;
				}

				bool hasNextItem() override {
					return m_it != m_listModel->m_decTestSamplesPool->m_samples.end();
				}
			};

			ListModel(CE::DecTestSamplesPool* decTestSamplesPool)
				: m_decTestSamplesPool(decTestSamplesPool)
			{}

			void newIterator(const IteratorCallback& callback) override {
				Iterator iterator(this);
				callback(&iterator);
			}
		};
		
		CE::Project* m_project;
		CE::DecTestSamplesPool* m_decTestSamplesPool;
		StdWindow* m_imageContentViewerWindow = nullptr;
		ListModel* m_listModel;
		TableListViewSelector<CE::DecTestSamplesPool::Sample*>* m_tableListView;
	public:
		DecSampleViewerPanel(CE::Project* project)
			: AbstractPanel("Decompiler samples viewer"), m_project(project)
		{
			m_decTestSamplesPool = new CE::DecTestSamplesPool(project);
			m_decTestSamplesPool->fillByTests();

			m_listModel = new ListModel(m_decTestSamplesPool);
			m_tableListView = new TableListViewSelector(new TableListView(m_listModel, {
				ColInfo("Name"),
				ColInfo("Comment")
			}));
			m_tableListView->handler([&](CE::DecTestSamplesPool::Sample* sample)
				{
					selectSample(sample);
				});
		}

		~DecSampleViewerPanel() override {
			delete m_decTestSamplesPool;
			delete m_listModel;
			delete m_tableListView;
		}

		StdWindow* createStdWindow() {
			return new StdWindow(this/*, ImGuiWindowFlags_MenuBar*/);
		}

	private:
		void renderPanel() override {
			Show(m_imageContentViewerWindow);
			m_tableListView->show();
		}

		void selectSample(CE::DecTestSamplesPool::Sample* sample) {
			/*const auto decCodeGraph = m_decTestSamplesPool->decompile(sample);
			const auto sdaCodeGraph = m_decTestSamplesPool->symbolize(sample, decCodeGraph);
			CE::Decompiler::Optimization::MakeFinalGraphOptimization(sdaCodeGraph);
			delete m_decompiledCodeViewerWindow;
			auto panel = new DecompiledCodeViewerPanel(sdaCodeGraph);
			m_decompiledCodeViewerWindow = panel->createStdWindow();*/
			if (!sample->m_funcGraph) {
				m_decTestSamplesPool->decode(sample);
			}
			
			delete m_imageContentViewerWindow;
			auto panel = new ImageContentViewerPanel(sample->m_imageDec);
			m_imageContentViewerWindow = panel->createStdWindow();
		}
	};
};