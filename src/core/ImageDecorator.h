#pragma once
#include "Function.h"
#include "decompiler/PCode/DecRegisterFactory.h"
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
	class ImageDecorator;
	class ImageManager;

	namespace Decompiler {
		namespace PCode {
			class InstructionPool;
		};
		class ImagePCodeGraph;
	};

	
	struct BookMark
	{
		ImageDecorator* m_imageDec;
		ComplexOffset m_offset;

		BookMark(ImageDecorator* imageDec = nullptr, ComplexOffset offset = InvalidOffset)
			: m_imageDec(imageDec), m_offset(offset)
		{}
	};
	
	struct BreakPoint
	{
		ImageDecorator* m_imageDec;
		Offset m_offset;

		BreakPoint(ImageDecorator* imageDec = nullptr, Offset offset = 0x0)
			: m_imageDec(imageDec), m_offset(offset)
		{}
	};

	// it is a symbolized image that decorates a raw image and can manipulate with high-level things (symbols)
	class ImageDecorator : public DB::DomainObject, public Description
	{
	public:
		enum IMAGE_TYPE {
			IMAGE_NONE,
			IMAGE_PE
		};

	private:
		ImageManager* m_imageManager;
		AddressSpace* m_addressSpace;
		AbstractImage* m_image = nullptr;
		IMAGE_TYPE m_type;
		Symbol::GlobalSymbolTable* m_globalSymbolTable;
		Symbol::GlobalSymbolTable* m_funcBodySymbolTable;
		Decompiler::PCode::InstructionPool* m_instrPool;
		Decompiler::ImagePCodeGraph* m_imagePCodeGraph;
		Decompiler::AbstractRegisterFactory* m_registerFactory = nullptr;
		std::map<Offset, BreakPoint>* m_breakPoints;
		std::map<ComplexOffset, BookMark>* m_bookMarks;
		std::map<ComplexOffset, DataType::IFunctionSignature*>* m_vfunc_calls;
		// need for making a clone that based on its parent image but haved own raw-image
		ImageDecorator* m_parentImageDec = nullptr;
		std::list<ImageDecorator*> m_childImageDecs;
	public:
		IDebugSession* m_debugSession = nullptr;
		
		ImageDecorator(
			ImageManager* imageManager,
			AddressSpace* addressSpace,
			IMAGE_TYPE type,
			Symbol::GlobalSymbolTable* globalSymbolTable,
			Symbol::GlobalSymbolTable* funcBodySymbolTable,
			const std::string& name,
			const std::string& comment = "");

		ImageDecorator(ImageManager* imageManager, AddressSpace* addressSpace, ImageDecorator* parentImageDec, const std::string& name, const std::string& comment = "");

		~ImageDecorator();

		void setImage(AbstractImage* image);

		void load();

		void copyImageFrom(IReader* reader);

		void save();

		bool hasLoaded() const;

		bool isDebug() const;

		ImageManager* getImageManager() const;

		AddressSpace* getAddressSpace() const;

		AbstractImage* getImage() const;

		IMAGE_TYPE getType() const;

		Function* getFunctionAt(Offset offset) const;

		Symbol::GlobalSymbolTable* getGlobalSymbolTable() const;

		Symbol::GlobalSymbolTable* getFuncBodySymbolTable() const;

		Decompiler::PCode::InstructionPool* getInstrPool() const;

		void setInstrPool(Decompiler::PCode::InstructionPool* instrPool);

		Decompiler::ImagePCodeGraph* getPCodeGraph() const;

		Decompiler::AbstractRegisterFactory* getRegisterFactory() const;

		void setPCodeGraph(Decompiler::ImagePCodeGraph* imagePCodeGraph);

		std::map<ComplexOffset, BookMark>& getBookmarks();

		std::map<ComplexOffset, DataType::IFunctionSignature*>& getVirtFuncCalls() const;

		ImageDecorator* getParentImage() const;

		const std::list<ImageDecorator*>& getChildImages() const;

		ImageDecorator* getCorrespondingDebugImage();

		void setBreakpoint(Offset offset, bool toggle);

		bool hasBreakpoint(Offset offset) const;

		fs::path getFile();

		void createImage(IReader* reader);
	};
};