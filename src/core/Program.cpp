#include "Program.h"
#include <Project.h>

CE::Program::Program()
{
	m_projectManager = new ProjectManager(this);
}
