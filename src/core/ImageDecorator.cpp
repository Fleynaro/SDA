#include "ImageDecorator.h"
#include <managers/AddressSpaceManager.h>
#include <images/PEImage.h>
#include <decompiler/PCode/DecPCodeInstructionPool.h>
#include <decompiler/Graph/DecPCodeGraph.h>
#include <utilities/Helper.h>

using namespace CE;

ImageDecorator::ImageDecorator(ImageManager* imageManager, AddressSpace* addressSpace, IMAGE_TYPE type, Symbol::SymbolTable* globalSymbolTable, Symbol::SymbolTable* funcBodySymbolTable, const std::string& name, const std::string& comment)
	:
	m_imageManager(imageManager),
	m_addressSpace(addressSpace),
	m_type(type),
	m_globalSymbolTable(globalSymbolTable),
	m_funcBodySymbolTable(funcBodySymbolTable),
	Description(name, comment)
{
	m_instrPool = new Decompiler::PCode::InstructionPool();
	m_imagePCodeGraph = new Decompiler::ImagePCodeGraph();
	m_vfunc_calls = new std::map<ComplexOffset, DataType::IFunctionSignature*>();
}

ImageDecorator::ImageDecorator(ImageManager* imageManager, AddressSpace* addressSpace, ImageDecorator* parentImageDec, const std::string& name, const std::string& comment)
	: ImageDecorator(
		imageManager,
		addressSpace,
		parentImageDec->m_type,
		parentImageDec->m_globalSymbolTable,
		parentImageDec->m_funcBodySymbolTable,
		name,
		comment
	)
{
	m_instrPool = parentImageDec->m_instrPool;
	m_imagePCodeGraph = parentImageDec->m_imagePCodeGraph;
	m_vfunc_calls = parentImageDec->m_vfunc_calls;
	m_parentImageDec = parentImageDec;
}

ImageDecorator::~ImageDecorator() {
	if (m_image) {
		delete m_image->getData();
		delete m_image;
	}

	if (!m_parentImageDec) {
		delete m_instrPool;
		delete m_imagePCodeGraph;
		delete m_vfunc_calls;
	}
}

void ImageDecorator::setImage(AbstractImage* image) {
	m_image = image;
}

void ImageDecorator::load() {
	char* buffer = nullptr;
	int size;
	Helper::File::LoadFileIntoBuffer(getFile(), &buffer, &size);

	if (m_type == IMAGE_PE) {
		m_image = new PEImage((int8_t*)buffer, size);
	}
}

void ImageDecorator::save() {
	Helper::File::SaveBufferIntoFile((char*)m_image->getData(), m_image->getSize(), getFile());
}

ImageManager* ImageDecorator::getImageManager() const
{
	return m_imageManager;
}

AddressSpace* ImageDecorator::getAddressSpace() const
{
	return m_addressSpace;
}

AbstractImage* ImageDecorator::getImage() const
{
	return m_image;
}

ImageDecorator::IMAGE_TYPE ImageDecorator::getType() const
{
	return m_type;
}

Function* ImageDecorator::getFunctionAt(Offset offset) const {
	if (const auto funcSymbol = dynamic_cast<Symbol::FunctionSymbol*>(getGlobalSymbolTable()->getSymbolAt(offset).second))
		return funcSymbol->getFunction();
	return nullptr;
}

Symbol::SymbolTable* ImageDecorator::getGlobalSymbolTable() const
{
	return m_globalSymbolTable;
}

Symbol::SymbolTable* ImageDecorator::getFuncBodySymbolTable() const
{
	return m_funcBodySymbolTable;
}

Decompiler::PCode::InstructionPool* ImageDecorator::getInstrPool() const
{
	return m_instrPool;
}

Decompiler::ImagePCodeGraph* ImageDecorator::getPCodeGraph() const
{
	return m_imagePCodeGraph;
}

std::map<ComplexOffset, DataType::IFunctionSignature*>& ImageDecorator::getVirtFuncCalls() const
{
	return *m_vfunc_calls;
}

ImageDecorator* ImageDecorator::getParentImage() const
{
	return m_parentImageDec;
}

fs::path ImageDecorator::getFile() {
	return m_addressSpace->getImagesDirectory() / fs::path(getName() + ".bin");
}
