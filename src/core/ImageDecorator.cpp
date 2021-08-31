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
	m_breakPoints = new std::map<Offset, BreakPoint>();
	m_bookMarks = new std::map<ComplexOffset, BookMark>();
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
	m_breakPoints = parentImageDec->m_breakPoints;
	m_bookMarks = parentImageDec->m_bookMarks;
	m_vfunc_calls = parentImageDec->m_vfunc_calls;
	m_parentImageDec = parentImageDec;
	parentImageDec->m_childImageDecs.push_back(this);
}

ImageDecorator::~ImageDecorator() {
	m_addressSpace->getImageDecorators().remove(this);
	delete m_image;
	delete m_registerFactory;

	if (!m_parentImageDec) {
		delete m_instrPool;
		delete m_imagePCodeGraph;
		delete m_breakPoints;
		delete m_bookMarks;
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
	createImage(new DataPointerReader((uint8_t*)buffer, size));
}

void ImageDecorator::copyImageFrom(IReader* reader) {
	std::vector<uint8_t> buffer(reader->getSize());
	reader->read(0x0, buffer);
	createImage(new VectorReader(buffer));
}

void ImageDecorator::save() {
	std::vector<uint8_t> buffer(m_image->getSize());
	m_image->getReader()->read(0x0, buffer);
	Helper::File::SaveBufferIntoFile((char*)&buffer, static_cast<int>(buffer.size()), getFile());
}

bool ImageDecorator::hasLoaded() const {
	return m_image != nullptr;
}

bool ImageDecorator::isDebug() const {
	return m_debugSession != nullptr;
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

Decompiler::AbstractRegisterFactory* ImageDecorator::getRegisterFactory() const {
	return m_registerFactory;
}

void ImageDecorator::setPCodeGraph(Decompiler::ImagePCodeGraph* imagePCodeGraph) {
	delete m_imagePCodeGraph;
	m_imagePCodeGraph = imagePCodeGraph;
}

std::map<ComplexOffset, BookMark>& ImageDecorator::getBookmarks() {
	return *m_bookMarks;
}

std::map<ComplexOffset, DataType::IFunctionSignature*>& ImageDecorator::getVirtFuncCalls() const
{
	return *m_vfunc_calls;
}

ImageDecorator* ImageDecorator::getParentImage() const
{
	return m_parentImageDec;
}

const std::list<ImageDecorator*>& ImageDecorator::getChildImages() const {
	return m_childImageDecs;
}

ImageDecorator* ImageDecorator::getCorrespondingDebugImage() {
	if (isDebug()) {
		return this;
	}
	for (const auto childImageDec : m_childImageDecs) {
		if (const auto debugImageDec = childImageDec->getCorrespondingDebugImage())
			return debugImageDec;
	}
	return nullptr;
}

void ImageDecorator::setBreakpoint(Offset offset, bool toggle) {
	if (toggle) {
		(*m_breakPoints)[offset] = BreakPoint(this, offset);
	}
	else {
		m_breakPoints->erase(offset);
	}
	if (const auto debugImageDec = getCorrespondingDebugImage()) {
		const auto address = debugImageDec->getImage()->getAddress() + offset;
		if (toggle) {
			debugImageDec->m_debugSession->addBreakpoint(address);
		}
		else {
			debugImageDec->m_debugSession->removeBreakpoint(address);
		}
	}
}

bool ImageDecorator::hasBreakpoint(Offset offset) const {
	return m_breakPoints->find(offset) != m_breakPoints->end();
}

fs::path ImageDecorator::getFile() {
	return m_addressSpace->getImagesDirectory() / fs::path(std::string(getName()) + ".bin");
}

void ImageDecorator::createImage(IReader* reader) {
	if (m_type == IMAGE_PE) {
		m_image = new PEImage(reader);
		m_registerFactory = new Decompiler::RegisterFactoryX86;
	}
}
