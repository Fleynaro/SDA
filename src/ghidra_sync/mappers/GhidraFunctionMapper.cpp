#include "GhidraFunctionMapper.h"
#include "GhidraSignatureTypeMapper.h"
#include <Manager/TypeManager.h>
#include <Manager/FunctionManager.h>

using namespace CE;
using namespace CE::Ghidra;

FunctionMapper::FunctionMapper(CE::FunctionManager* functionManager, DataTypeMapper* dataTypeMapper)
	: m_functionManager(functionManager), m_dataTypeMapper(dataTypeMapper)
{}

void FunctionMapper::load(packet::SDataFullSyncPacket* dataPacket) {
	for (auto funcDesc : dataPacket->functions) {
		auto function = m_functionManager->findFunctionByGhidraId(funcDesc.id);
		/*if (function == nullptr) {
			auto mainModule = m_functionManager->getProject()->getProcessModuleManager()->getMainModule();
			auto signatureType = m_functionManager->getProject()->getTypeManager()->createSignature("", "");
			function = m_functionManager->createFunction("", mainModule, {}, signatureType);
		}*/
		changeFunctionByDesc(function, funcDesc);
	}
}

void markObjectAsSynced(SyncContext* ctx, Function* func) {
	SQLite::Statement query(*ctx->m_db, "UPDATE sda_func_defs SET ghidra_sync_id=?1 WHERE def_id=?2");
	query.bind(1, ctx->m_syncId);
	query.bind(2, func->getId());
	query.exec();
}

void FunctionMapper::upsert(SyncContext* ctx, IObject* obj) {
	auto func = dynamic_cast<Function*>(obj);
	ctx->m_dataPacket->functions.push_back(buildDesc(func));
	markObjectAsSynced(ctx, func);
}

void FunctionMapper::remove(SyncContext* ctx, IObject* obj) {
	auto func = dynamic_cast<Function*>(obj);
	ctx->m_dataPacket->removed_functions.push_back(func->getGhidraId());
	markObjectAsSynced(ctx, func);
}

void FunctionMapper::changeFunctionByDesc(Function* function, const function::SFunction& funcDesc) {
	function->setName(funcDesc.name);
	function->setComment(funcDesc.comment);
	/*function->getAddressRangeList().clear();
	function->getAddressRangeList() = getRangesFromDesc(funcDesc.ranges);
	m_dataTypeMapper->m_signatureTypeMapper->changeSignatureByDesc(function->getSignature(), funcDesc.signature);*/
}

function::SFunction FunctionMapper::buildDesc(Function* function) {
	function::SFunction funcDesc;
	funcDesc.__set_id(function->getGhidraId());

	auto spliter = function->getName().find("::");
	if (spliter != std::string::npos) {
		std::string funcName = function->getName();
		funcName[spliter] = '_';
		funcName[spliter + 1] = '_';
		funcDesc.__set_name(funcName);
	}
	else {
		funcDesc.__set_name(function->getName());
	}
	funcDesc.__set_comment(function->getComment());

	/*for (auto& range : function->getAddressRangeList()) {
		function::SFunctionRange rangeDesc;
		rangeDesc.__set_minOffset(m_functionManager->getProject()->getProcessModuleManager()->getMainModule()->toRelAddr(range.getMinAddress()));
		rangeDesc.__set_maxOffset(m_functionManager->getProject()->getProcessModuleManager()->getMainModule()->toRelAddr(range.getMaxAddress()));
		funcDesc.ranges.push_back(rangeDesc);
	}*/

	//funcDesc.__set_signature(m_dataTypeMapper->m_signatureTypeMapper->buildDesc(function->getSignature()));
	return funcDesc;
}
