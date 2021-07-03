#include "ImageManager.h"
#include <database/Mappers/ImageMapper.h>
#include <managers/SymbolTableManager.h>

using namespace CE;

CE::ImageManager::ImageManager(Project* project)
	: AbstractItemManager(project)
{
	m_imageMapper = new DB::ImageMapper(this);
}

ImageDecorator* CE::ImageManager::createImage(AddressSpace* addressSpace, ImageDecorator::IMAGE_TYPE type, Symbol::SymbolTable* globalSymbolTable, Symbol::SymbolTable* funcBodySymbolTable, const std::string& name, const std::string& comment, bool markAsNew) {
	auto imageDec = new ImageDecorator(this, addressSpace, type, globalSymbolTable, funcBodySymbolTable, name, comment);
	imageDec->setMapper(m_imageMapper);
	if (markAsNew) {
		getProject()->getTransaction()->markAsNew(imageDec);
	}
	return imageDec;
}

ImageDecorator* CE::ImageManager::createImage(AddressSpace* addressSpace, ImageDecorator::IMAGE_TYPE type, const std::string& name, const std::string& comment, bool markAsNew) {
	auto factory = getProject()->getSymTableManager()->getFactory();
	auto globalSymbolTable = factory.createSymbolTable(Symbol::SymbolTable::GLOBAL_SPACE);
	auto funcBodySymbolTable = factory.createSymbolTable(Symbol::SymbolTable::GLOBAL_SPACE);
	return createImage(addressSpace, type, globalSymbolTable, funcBodySymbolTable, name, comment, markAsNew);
}

ImageDecorator* CE::ImageManager::createImageFromParent(AddressSpace* addressSpace, ImageDecorator* parentImageDec, const std::string& name, const std::string& comment, bool markAsNew) {
	auto imageDec = new ImageDecorator(this, addressSpace, parentImageDec, name, comment);
	imageDec->setMapper(m_imageMapper);
	if (markAsNew) {
		imageDec->setId(m_imageMapper->getNextId());
		getProject()->getTransaction()->markAsNew(imageDec);
	}
	return imageDec;
}

void CE::ImageManager::loadImages() const
{
	m_imageMapper->loadAll();
}

ImageDecorator* CE::ImageManager::findImageById(DB::Id id) {
	return dynamic_cast<ImageDecorator*>(find(id));
}

ImageDecorator* CE::ImageManager::findImageByName(const std::string& name) {
	Iterator it(this);
	while (it.hasNext()) {
		auto item = it.next();
		if (item->getName() == name) {
			return item;
		}
	}
	throw ItemNotFoundException();
}
