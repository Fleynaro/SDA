#pragma once
#include "AbstractManager.h"
#include <ImageDecorator.h>

namespace DB {
	class ImageMapper;
};

namespace CE
{
	class ImageManager : public AbstractItemManager
	{
	public:
		using Iterator = AbstractIterator<ImageDecorator>;

		ImageManager(Project* project);

		ImageDecorator* createImage(AddressSpace* addressSpace, ImageDecorator::IMAGE_TYPE type, Symbol::GlobalSymbolTable* globalSymbolTable, Symbol::GlobalSymbolTable* funcBodySymbolTable, const std::string& name, const std::string& comment = "", bool markAsNew = true);

		ImageDecorator* createImage(AddressSpace* addressSpace, ImageDecorator::IMAGE_TYPE type, const std::string& name, const std::string& comment = "", bool markAsNew = true);

		ImageDecorator* createImageFromParent(AddressSpace* addressSpace, ImageDecorator* parentImageDec, const std::string& name, const std::string& comment = "", bool markAsNew = true);

		void loadImages() const;

		ImageDecorator* findImageById(DB::Id id);

		ImageDecorator* findImageByName(const std::string& name);
	private:
		DB::ImageMapper* m_imageMapper;
	};
};