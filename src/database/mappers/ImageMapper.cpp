#include "ImageMapper.h"
#include <managers/ImageManager.h>
#include <managers/AddressSpaceManager.h>
#include <managers/SymbolTableManager.h>
#include <managers/TypeManager.h>
#include <decompiler/PCode/Decoders/DecPCodeDecoderX86.h>
#include <decompiler/Graph/DecPCodeGraph.h>

using namespace DB;
using namespace CE;
using namespace Decompiler;

ImageMapper::ImageMapper(IRepository* repository)
	: AbstractMapper(repository)
{}

void ImageMapper::loadAll() {
	auto& db = getManager()->getProject()->getDB();
	Statement query(db, "SELECT * FROM sda_images");
	load(&db, query);
}

Id ImageMapper::getNextId() {
	auto& db = getManager()->getProject()->getDB();
	return GenerateNextId(&db, "sda_images");
}

ImageManager* ImageMapper::getManager() const
{
	return static_cast<ImageManager*>(m_repository);
}

IDomainObject* ImageMapper::doLoad(Database* db, Statement& query) {
	int image_id = query.getColumn("image_id");
	int parent_image_id = query.getColumn("parent_image_id");
	auto type = static_cast<ImageDecorator::IMAGE_TYPE>(static_cast<int>(query.getColumn("type")));
	//std::uintptr_t addr = (int64_t)query.getColumn("addr");
	std::string name = query.getColumn("name");
	std::string comment = query.getColumn("comment");
	int addr_space_id = query.getColumn("addr_space_id");
	int global_table_id = query.getColumn("global_table_id");
	int func_body_table_id = query.getColumn("func_body_table_id");
	std::string json_instr_pool_str = query.getColumn("json_instr_pool");
	auto json_instr_pool = json::parse(json_instr_pool_str);
	std::string json_vfunc_calls_str = query.getColumn("json_vfunc_calls");
	auto json_vfunc_calls = json::parse(json_vfunc_calls_str);
	std::string json_func_graphs_str = query.getColumn("json_func_graphs");
	auto json_func_graphs = json::parse(json_func_graphs_str);
	
	ImageDecorator* imageDec = nullptr;
	auto project = getManager()->getProject();
	auto addrSpace = project->getAddrSpaceManager()->findAddressSpaceById(addr_space_id);

	if (parent_image_id != 0)
	{
		auto parentImage = getManager()->findImageById(parent_image_id);
		imageDec = getManager()->createImageFromParent(addrSpace, parentImage, name, comment, false);
		imageDec->load();
	}
	else {
		auto globalSymTable = project->getSymTableManager()->findSymbolTableById(global_table_id);
		auto funcBodySymTable = project->getSymTableManager()->findSymbolTableById(func_body_table_id);

		imageDec = getManager()->createImage(addrSpace, type, globalSymTable, funcBodySymTable, name, comment, false);
		imageDec->load();
		auto imgPCodeGraph = imageDec->getPCodeGraph();

		// load modified instructions for instr. pool
		for (const auto& json_mod_instr : json_instr_pool["mod_instructions"]) {
			auto offset = json_mod_instr["offset"].get<uint64_t>();
			auto mod = json_mod_instr["mod"].get<InstructionPool::MODIFICATOR>();
			imageDec->getInstrPool()->m_modifiedInstructions[offset] = mod;
		}
		// load virtual func. calls
		for (const auto& json_vfunc_call : json_vfunc_calls) {
			auto offset = json_vfunc_call["offset"].get<uint64_t>();
			auto sig_id = json_vfunc_call["sig_id"].get<Id>();
			auto funcSig = dynamic_cast<DataType::IFunctionSignature*>(getManager()->getProject()->getTypeManager()->findTypeById(sig_id));
			imageDec->getVirtFuncCalls()[offset] = funcSig;
		}
		// load pcode func. graphs
		for (const auto& json_func_graph : json_func_graphs) {
			auto funcGraph = imgPCodeGraph->createFunctionGraph();
			loadFuncPCodeGraphJson(json_func_graph, funcGraph, imageDec);
		}
		// load pcode func. graph connections
		for (const auto& json_func_graph : json_func_graphs) {
			auto start_block = json_func_graph["start_block"].get<uint64_t>();
			auto funcGraph = imgPCodeGraph->getFuncGraphAt(start_block);
			// load non-virt func calls
			for (const auto& json_nv_func : json_func_graph["nv_func_calls"]) {
				auto nonVirtFuncOffset = json_nv_func.get<uint64_t>();
				auto otherFuncGraph = imgPCodeGraph->getFuncGraphAt(nonVirtFuncOffset);
				funcGraph->addNonVirtFuncCall(otherFuncGraph);
			}
			// load virt func calls
			for (const auto& json_v_func : json_func_graph["v_func_calls"]) {
				auto virtFuncOffset = json_v_func.get<uint64_t>();
				auto otherFuncGraph = imgPCodeGraph->getFuncGraphAt(virtFuncOffset);
				funcGraph->addVirtFuncCall(otherFuncGraph);
			}
		}
		imgPCodeGraph->fillHeadFuncGraphs();
	}

	// add the image to its addr. space
	addrSpace->getImages()[imageDec->getImage()->getAddress()] = imageDec;

	imageDec->setId(image_id);
	return imageDec;
}

void ImageMapper::doInsert(TransactionContext* ctx, IDomainObject* obj) {
	doUpdate(ctx, obj);
}

void ImageMapper::doUpdate(TransactionContext* ctx, IDomainObject* obj) {
	auto imageDec = dynamic_cast<ImageDecorator*>(obj);
	Statement query(*ctx->m_db,
	                "REPLACE INTO sda_images (image_id, parent_image_id, type, name, comment, addr_space_id, global_table_id, func_body_table_id, json_instr_pool, json_vfunc_calls, json_func_graphs, save_id) VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12)");
	query.bind(1, imageDec->getId());
	bind(query, imageDec);
	query.bind(12, ctx->m_saveId);
	query.exec();
}

void ImageMapper::doRemove(TransactionContext* ctx, IDomainObject* obj) {
	const std::string action_query_text =
		ctx->m_notDelete ? "UPDATE sda_images SET deleted=1" : "DELETE FROM sda_images";
	Statement query(*ctx->m_db, action_query_text + " WHERE image_id=?1");
	query.bind(1, obj->getId());
	query.exec();
}

void ImageMapper::decodePCodeBlock(PCodeBlock* block, ImageDecorator* imageDec) {
	WarningContainer warningContainer;
	RegisterFactoryX86 registerFactoryX86;
	DecoderX86 decoder(&registerFactoryX86, imageDec ->getInstrPool(), &warningContainer);
	auto offset = block->getMinOffset() >> 8;
	while (offset < block->getMaxOffset() >> 8) {
		decoder.decode(imageDec->getImage()->getData() + offset, static_cast<int>(offset), imageDec->getImage()->getSize());
		if (!decoder.getOrigInstruction())
			break;
		for (auto instr : decoder.getDecodedPCodeInstructions()) {
			block->getInstructions().push_back(instr);
		}
		offset += decoder.getOrigInstruction()->m_length;
	}
}

void ImageMapper::loadFuncPCodeGraphJson(const json& json_func_graph, FunctionPCodeGraph* funcGraph, ImageDecorator* imageDec) {
	auto imgPCodeGraph = funcGraph->getImagePCodeGraph();

	// load blocks
	for (const auto& json_pcode_block : json_func_graph["blocks"]) {
		const auto level = json_pcode_block["level"].get<int>();
		const auto min_offset = json_pcode_block["min_offset"].get<int64_t>();
		const auto max_offset = json_pcode_block["max_offset"].get<int64_t>();

		const auto block = imgPCodeGraph->createBlock(min_offset, max_offset);
		block->m_level = level;
		funcGraph->addBlock(block);
		decodePCodeBlock(block, imageDec);
	}
	// load block connections
	for (const auto& json_pcode_block : json_func_graph["blocks"]) {
		const auto min_offset = json_pcode_block["min_offset"].get<int64_t>();
		auto block = imgPCodeGraph->getBlockAtOffset(min_offset);

		if (json_pcode_block.contains("next_near_block")) {
			const auto next_near_block = json_pcode_block["next_near_block"].get<int64_t>();
			const auto nextNearBlock = imgPCodeGraph->getBlockAtOffset(next_near_block);
			block->setNextNearBlock(nextNearBlock);
		}
		if (json_pcode_block.contains("next_far_block")) {
			const auto next_far_block = json_pcode_block["next_far_block"].get<int64_t>();
			const auto nextFarBlock = imgPCodeGraph->getBlockAtOffset(next_far_block);
			block->setNextNearBlock(nextFarBlock);
		}
	}
	// load start block
	const auto start_block = json_func_graph["start_block"].get<int64_t>();
	const auto startBlock = imgPCodeGraph->getBlockAtOffset(start_block);
	funcGraph->setStartBlock(startBlock);
}

json ImageMapper::createFuncPCodeGraphJson(FunctionPCodeGraph* funcPCodeGraph) {
	json json_func_graph;

	// save pcode blocks
	json json_pcode_blocks;
	for (auto pcodeBlock : funcPCodeGraph->getBlocks()) {
		json json_pcode_block;
		json_pcode_block["level"] = pcodeBlock->m_level;
		json_pcode_block["min_offset"] = static_cast<uint64_t>(pcodeBlock->getMinOffset());
		json_pcode_block["max_offset"] = static_cast<uint64_t>(pcodeBlock->getMaxOffset());
		if (pcodeBlock->getNextNearBlock())
			json_pcode_block["next_near_block"] = static_cast<uint64_t>(pcodeBlock->getNextNearBlock()->getMinOffset());
		if (pcodeBlock->getNextFarBlock())
			json_pcode_block["next_far_block"] = static_cast<uint64_t>(pcodeBlock->getNextFarBlock()->getMinOffset());
		json_pcode_blocks.push_back(json_pcode_block);
	}
	json_func_graph["blocks"] = json_pcode_blocks;

	// save non-virt func calls
	json json_nv_func_calls;
	for (auto funcGraph : funcPCodeGraph->getNonVirtFuncCalls()) {
		json_nv_func_calls.push_back(static_cast<uint64_t>(funcGraph->getStartBlock()->getMinOffset()));
	}
	json_func_graph["nv_func_calls"] = json_nv_func_calls;

	// save virt func calls
	json json_v_func_calls;
	for (auto funcGraph : funcPCodeGraph->getVirtFuncCalls()) {
		json_v_func_calls.push_back(static_cast<uint64_t>(funcGraph->getStartBlock()->getMinOffset()));
	}
	json_func_graph["v_func_calls"] = json_v_func_calls;

	// save other
	json_func_graph["start_block"] = static_cast<uint64_t>(funcPCodeGraph->getStartBlock()->getMinOffset());

	return json_func_graph;
}

void ImageMapper::bind(Statement& query, ImageDecorator* imageDec) {
	json json_instr_pool;
	json json_vfunc_calls;
	json json_func_graphs;

	// save modified instructions
	json json_mod_instrs;
	for (auto& pair : imageDec->getInstrPool()->m_modifiedInstructions) {
		json json_mod_instr;
		json_mod_instr["offset"] = pair.first;
		json_mod_instr["mod"] = pair.second;
		json_mod_instrs.push_back(json_mod_instr);
	}
	json_instr_pool["mod_instructions"] = json_mod_instrs;
	// save virtual func. calls
	for (const auto& [offset, sig] : imageDec->getVirtFuncCalls()) {
		json json_vfunc_call;
		json_vfunc_call["offset"] = static_cast<uint64_t>(offset);
		json_vfunc_call["sig_id"] = sig->getId();
		json_vfunc_calls.push_back(json_vfunc_call);
	}
	// save pcode func. graphs
	for (auto& funcGraph : imageDec->getPCodeGraph()->getFunctionGraphList()) {
		auto json_func_graph = createFuncPCodeGraphJson(&funcGraph);
		json_func_graphs.push_back(json_func_graph);
	}
	
	query.bind(2, imageDec->getParentImage() ? imageDec->getParentImage()->getId() : 0);
	query.bind(3, imageDec->getType());
	query.bind(4, imageDec->getName());
	query.bind(5, imageDec->getComment());
	query.bind(6, imageDec->getAddressSpace()->getId());
	query.bind(7, imageDec->getGlobalSymbolTable()->getId());
	query.bind(8, imageDec->getFuncBodySymbolTable()->getId());
	query.bind(9, json_instr_pool.dump());
	query.bind(10, json_vfunc_calls.dump());
	query.bind(11, json_func_graphs.dump());
}
