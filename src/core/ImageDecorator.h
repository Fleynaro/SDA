#pragma once
#include <images/AbstractImage.h>
#include <database/DomainObject.h>
#include <utilities/Description.h>
#include <datatypes/FunctionSignature.h>
#include <SymbolTable.h>
#include <filesystem>
namespace fs = std::filesystem;

namespace CE
{
	class AddressSpace;
	class ImageManager;

	namespace Decompiler {
		namespace PCode {
			class InstructionPool;
		};
		class ImagePCodeGraph;
	};

	// it is a symbolized image that decorates a raw image and can manipulate with high-level things (symbols)
	class ImageDecorator : public DB::DomainObject, public Description
	{
	public:
		enum IMAGE_TYPE {
			IMAGE_PE
		};

	private:
		ImageManager* m_imageManager;
		AddressSpace* m_addressSpace;
		AbstractImage* m_image = nullptr;
		IMAGE_TYPE m_type;
		Symbol::SymbolTable* m_globalSymbolTable;
		Symbol::SymbolTable* m_funcBodySymbolTable;
		Decompiler::PCode::InstructionPool* m_instrPool;
		Decompiler::ImagePCodeGraph* m_imagePCodeGraph;
		std::map<int64_t, CE::DataType::IFunctionSignature*>* m_vfunc_calls;
		// need for making a clone that based on its parent image but haved own raw-image
		ImageDecorator* m_parentImageDec = nullptr;
		
	public:
		ImageDecorator(
			ImageManager* imageManager,
			AddressSpace* addressSpace,
			IMAGE_TYPE type,
			Symbol::SymbolTable* globalSymbolTable,
			Symbol::SymbolTable* funcBodySymbolTable,
			const std::string& name,
			const std::string& comment = "");

		ImageDecorator(ImageManager* imageManager, AddressSpace* addressSpace, ImageDecorator* parentImageDec, const std::string& name, const std::string& comment = "");

		~ImageDecorator();

		void load();

		void save();

		ImageManager* getImageManager() const;

		AddressSpace* getAddressSpace() const;

		AbstractImage* getImage() const;

		IMAGE_TYPE getType() const;

		Symbol::SymbolTable* getGlobalSymbolTable() const;

		Symbol::SymbolTable* getFuncBodySymbolTable() const;

		Decompiler::PCode::InstructionPool* getInstrPool() const;

		Decompiler::ImagePCodeGraph* getPCodeGraph() const;

		std::map<int64_t, CE::DataType::IFunctionSignature*>& getVirtFuncCalls() const;

		ImageDecorator* getParentImage() const;

		fs::path getFile();
	};
};