#include "Class.h"

using namespace CE;
using namespace DataType;

AbstractType::Group Class::getGroup() {
	return Group::Class;
}

Class::MethodListType& Class::getMethods() {
	return m_methods;
}

void Class::addMethod(Function* method) {
	getMethods().push_back(method);
}

std::list<Class*> Class::getClassesInHierarchy() {
	if (m_base != nullptr) {
		auto result = getClassesInHierarchy();
		result.push_back(this);
		return result;
	}
	return { this };
}

Class* Class::getBaseClass() const
{
	return m_base;
}

void Class::setBaseClass(Class* base, bool createBaseClassField) {

}

Class::MethodIterator::MethodIterator(Class* Class)
	//: m_vtable(Class->getVtable())
{
	m_classes = Class->getClassesInHierarchy();
	updateIterator();
}

bool Class::MethodIterator::hasNext() {
	if (!(m_classes.size() != 0 && m_iterator != m_end))
		return false;
	if (m_signatures.count((*m_iterator)->getSignature()->getSigName()) != 0) {
		next();
		return hasNext();
	}
	return true;
}

Function* Class::MethodIterator::next() {
	//vtable...

	if (m_iterator == m_end) {
		m_classes.pop_front();
		updateIterator();
	}

	const auto method = *m_iterator;
	m_iterator++;
	m_signatures.insert(method->getSignature()->getSigName());
	return method;
}

void Class::MethodIterator::updateIterator() {
	m_iterator = m_classes.front()->getMethods().begin();
	m_end = m_classes.front()->getMethods().begin();
}
