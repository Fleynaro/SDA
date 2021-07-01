#pragma once
#include <images/IImage.h>
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
	class ImageDecorator : public DB::DomainObject, public Description, public IImage
	{
	public:
		enum IMAGE_TYPE {
			IMAGE_PE
		};

	private:
		ImageManager* m_imageManager;
		AddressSpace* m_addressSpace;
		IImage* m_image = nullptr;
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

		ImageManager* getImageManager();

		AddressSpace* getAddressSpace();

		IMAGE_TYPE getType();

		Symbol::SymbolTable* getGlobalSymbolTable();

		Symbol::SymbolTable* getFuncBodySymbolTable();

		Decompiler::PCode::InstructionPool* getInstrPool();

		Decompiler::ImagePCodeGraph* getPCodeGraph();

		std::map<int64_t, CE::DataType::IFunctionSignature*>& getVirtFuncCalls();

		ImageDecorator* getParentImage();

		fs::path getFile();

		int8_t* getData() override;

		int getSize() override;

		int getOffsetOfEntryPoint() override;

		SegmentType defineSegment(int offset) override;

		int toImageOffset(int offset) override;

		int addrToImageOffset(uint64_t addr) override;

		std::uintptr_t getAddress() override;
	};
};