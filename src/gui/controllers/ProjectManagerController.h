#pragma once
#include "imgui_wrapper/controls/List.h"
#include "Exception.h"
#include "ProjectManager.h"

namespace GUI
{
	class ProjectManagerController
	{
	public:
		class ProjectLoadException : public WarningException {
		public: ProjectLoadException(const char* message) : WarningException(message) {}
		};
		
		class ProjectListModel : public IListModel<CE::ProjectManager::ProjectEntry*>
		{
			CE::ProjectManager* m_projectManager;
		public:
			ProjectListModel(CE::ProjectManager* projectManager)
				: m_projectManager(projectManager)
			{}
		private:
			class ProjectIterator : public Iterator
			{
				std::list<CE::ProjectManager::ProjectEntry>::iterator m_it;
				std::list<CE::ProjectManager::ProjectEntry>* m_list;
			public:
				ProjectIterator(std::list<CE::ProjectManager::ProjectEntry>* list)
					: m_list(list), m_it(list->begin())
				{}

				void getNextItem(std::string* text, CE::ProjectManager::ProjectEntry** data) override {
					*text = m_it->m_dir.string();
					*data = &*m_it;
					++m_it;
				}

				bool hasNextItem() override {
					return m_it != m_list->end();
				}
			};

			void newIterator(const IteratorCallback& callback) override {
				ProjectIterator iterator(&m_projectManager->m_projectEntries);
				callback(&iterator);
			}
		};

		ProjectListModel m_listModel;
		CE::Program* m_program;

		ProjectManagerController(CE::Program* program);

		void load() const;

		bool hasProjects() const;

		CE::Project* openProject(CE::ProjectManager::ProjectEntry* projectEntry) const;

		CE::Project* createNewProject(const fs::path& dir) const;

		fs::path findDefaultPath() const;
	};
};
