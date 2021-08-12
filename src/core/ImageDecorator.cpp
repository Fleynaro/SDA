#include "ImageDecorator.h"
#include <managers/AddressSpaceManager.h>
#include <images/PEImage.h>
#include <decompiler/PCode/DecPCodeInstructionPool.h>
#include <decompiler/Graph/DecPCodeGraph.h>
#include <utilities/Helper.h>

using namespace CE;

ImageDecorator::ImageDecorator(ImageManager* imageManager, AddressSpace* addressSpace, IMAGE_TYPE type, Symbol::GlobalSymbolTable* globalSymbolTable, Symbol::GlobalSymbolTable* funcBodySymbolTable, const std::string& name, const std::string& comment)
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
	m_addressSpace->getImageDecorators().push_back(this);

	globalSymbolTable->m_imageDec = this;
	funcBodySymbolTable->m_imageDec = this;
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
	m_addressSpace->getImageDecorators().remove(this);
	delete m_image;

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
		const auto image = new PEImage(new SimpleReader((uint8_t*)buffer, size));
		m_image = image;
	}
}

void ImageDecorator::save() {
	std::vector<uint8_t> buffer(m_image->getSize());
	m_image->getReader()->read(0x0, buffer);
	Helper::File::SaveBufferIntoFile((char*)&buffer, static_cast<int>(buffer.size()), getFile());
}

bool ImageDecorator::hasLoaded() const {
	return m_image != nullptr;
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

Symbol::GlobalSymbolTable* ImageDecorator::getGlobalSymbolTable() const
{
	return m_globalSymbolTable;
}

Symbol::GlobalSymbolTable* ImageDecorator::getFuncBodySymbolTable() const
{
	return m_funcBodySymbolTable;
}

Decompiler::PCode::InstructionPool* ImageDecorator::getInstrPool() const
{
	return m_instrPool;
}

void ImageDecorator::setInstrPool(Decompiler::PCode::InstructionPool* instrPool) {
	delete m_instrPool;
	m_instrPool = instrPool;
}

Decompiler::ImagePCodeGraph* ImageDecorator::getPCodeGraph() const
{
	return m_imagePCodeGraph;
}

void ImageDecorator::setPCodeGraph(Decompiler::ImagePCodeGraph* imagePCodeGraph) {
	delete m_imagePCodeGraph;
	m_imagePCodeGraph = imagePCodeGraph;
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
	return m_addressSpace->getImagesDirectory() / fs::path(std::string(getName()) + ".bin");
}
