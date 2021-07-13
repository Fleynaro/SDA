#include "DecExecContext.h"
#include "PrimaryDecompiler.h"

using namespace CE;
using namespace Decompiler;

void RegisterExecContext::clear() {
	for (auto& pair : m_registers) {
		auto& registers = pair.second;
		for (auto& regInfo : registers)
			delete regInfo.m_expr;
	}
}

ExprTree::INode* RegisterExecContext::requestRegister(const Register& reg) {
	BitMask64 needReadMask = reg.m_valueRangeMask;
	auto regParts = findRegisterParts(reg.getId(), needReadMask);
	if (!needReadMask.isZero()) {
		const auto symbol = new Symbol::RegisterVariable(reg);
		m_decompiler->m_decompiledGraph->addSymbol(symbol);
		RegisterPart part;
		part.m_regMask = reg.m_valueRangeMask;
		part.m_maskToChange = needReadMask;
		part.m_expr = new ExprTree::SymbolLeaf(symbol);
		regParts.push_back(part);
	}

	return CreateExprFromRegisterParts(regParts, reg.m_valueRangeMask);
}

void RegisterExecContext::setRegister(const Register& reg, ExprTree::INode* newExpr) {
	std::list<TopNode*> oldTopNodes;

	auto it = m_registers.find(reg.getId());
	if (it != m_registers.end()) {
		auto& registers = it->second;
		// write rax -> remove eax/ax/ah/al
		auto it2 = registers.begin();
		while(it2 != registers.end()) {
			if (reg.intersect(it2->m_register)) {
				oldTopNodes.push_back(it2->m_expr);
				it2 = registers.erase(it2);
			} else
			{
				++it2;
			}
		}
	}
	else {
		m_registers[reg.getId()] = std::list<RegisterInfo>();
	}

	// add the register
	RegisterInfo registerInfo;
	registerInfo.m_register = reg;
	registerInfo.m_expr = new TopNode(newExpr);
	registerInfo.m_srcExecContext = m_execContext;
	m_registers[reg.getId()].push_back(registerInfo);

	// delete only here because new expr may be the same as old expr: mov rax, rax
	for (auto topNode : oldTopNodes) {
		delete topNode;
	}
}

void RegisterExecContext::copyFrom(RegisterExecContext* ctx) {
	clear();

	const auto decompiler = m_decompiler;
	const auto execCtx = m_execContext;
	*this = *ctx;
	m_decompiler = decompiler;
	m_execContext = execCtx;
	m_isFilled = true;

	for (auto& pair : m_registers) {
		auto& registers = pair.second;
		for (auto& regInfo : registers) {
			regInfo.m_expr = new TopNode(regInfo.m_expr->getNode());
		}
	}
}

void RegisterExecContext::join(RegisterExecContext* ctx) {
	for (auto& pair : ctx->m_registers) {
		auto regId = pair.first;

		auto it = m_registers.find(regId);
		if (it != m_registers.end()) {
			std::list<RegisterInfo> neededRegs;
			auto regs1 = pair.second; // from ctx->m_registers
			auto& regs2 = it->second; // from m_registers

			// find equal registers with equal top nodes (expr), they are mutual
			for (auto it1 = regs1.begin(); it1 != regs1.end(); it1++) {
				bool hasFound = false;
				for (auto it2 = regs2.begin(); it2 != regs2.end(); it2++) {
					if (it1->m_register == it2->m_register && it1->m_expr->getNode() == it2->m_expr->getNode()) {
						neededRegs.push_back(*it2);
						regs1.erase(it1);
						regs2.erase(it2);
						hasFound = true;
						break;
					}
				}
				if (hasFound)
					break;
			}

			// if there are registers which come from different blocks
			if (!regs1.empty() && !regs2.empty()) {
				// WAY 1 (single mask)
				auto mask1 = calculateMaxMask(regs1);
				auto mask2 = calculateMaxMask(regs2);
				auto resultMask = mask1 & mask2;
				auto resultMasks = { resultMask };

				// WAY 2 (multiple masks)
				// calculate masks which belongs to different groups with non-intersected registers.
				// need for XMM registers to split low and high parts into two independent local vars
				// auto resultMasks = CalculateMasks(regs1, regs2); // todo: does it need?

				for (auto resultMask : resultMasks)
				{
					// new register
					auto& sampleReg = regs1.begin()->m_register;
					auto newRegister = Register(sampleReg.getGenericId(), sampleReg.getIndex(), resultMask, sampleReg.getType());
					auto newUsing = RegisterInfo::REGISTER_NOT_USING;

					// parent contexts
					std::set<ExecContext*> newExecCtxs;

					// iterate over all register parts in two parent contexts
					Symbol::LocalVariable* existingLocalVar = nullptr;
					for (auto& regs : { regs1, regs2 }) {
						for (auto& regInfo : regs) {
							// add contexts
							if (regInfo.m_srcExecContext != m_execContext)
								newExecCtxs.insert(regInfo.m_srcExecContext);

							// change using state
							if (newUsing == RegisterInfo::REGISTER_NOT_USING) {
								if (regInfo.m_using == RegisterInfo::REGISTER_PARTIALLY_USING) {
									newUsing = RegisterInfo::REGISTER_PARTIALLY_USING;
								} else if (regInfo.m_using == RegisterInfo::REGISTER_FULLY_USING) {
									newUsing = RegisterInfo::REGISTER_FULLY_USING;
								}
							}
							else if (newUsing == RegisterInfo::REGISTER_FULLY_USING) {
								if (regInfo.m_using <= RegisterInfo::REGISTER_PARTIALLY_USING) {
									newUsing = RegisterInfo::REGISTER_PARTIALLY_USING;
								}
							}

							if (!existingLocalVar) {
								// find an exitsting symbol with need size for re-using
								std::list<ExprTree::SymbolLeaf*> symbolLeafs;
								GatherSymbolLeafsFromNode(regInfo.m_expr->getNode(), symbolLeafs);
								for (auto symbolLeaf : symbolLeafs) {
									if (auto localVar = dynamic_cast<Symbol::LocalVariable*>(symbolLeaf->m_symbol)) {
										auto& localVarInfo = m_decompiler->m_localVars[localVar];

										if (localVarInfo.m_register.intersect(newRegister)) {
											if (localVarInfo.m_register.m_valueRangeMask == newRegister.m_valueRangeMask) {
												existingLocalVar = localVar;
											}
											else {
												// when eax=localVar and ax=5 coming in
												if (regInfo.m_srcExecContext == m_execContext) {
													for (auto ctx : localVarInfo.m_execCtxs)
														newExecCtxs.insert(ctx);
												}
											}
											break;
										}
									}
								}
							}
						}
					}

					// new register info
					RegisterInfo registerInfo;
					registerInfo.m_register = newRegister;
					auto localVar = existingLocalVar;
					if (!localVar) {
						// new local var
						localVar = new Symbol::LocalVariable(resultMask.getSize());
						// info for par. assignments
						PrimaryDecompiler::LocalVarInfo localVarInfo;
						localVarInfo.m_register = newRegister;
						m_decompiler->m_localVars[localVar] = localVarInfo;
					}
					registerInfo.m_expr = new TopNode(new ExprTree::SymbolLeaf(localVar));
					registerInfo.m_srcExecContext = m_execContext;
					registerInfo.m_using = newUsing;

					// add parent contexts where par. assignments (localVar = 5) will be created
					auto& localVarInfo = m_decompiler->m_localVars[localVar];
					for (auto ctx : newExecCtxs)
						localVarInfo.m_execCtxs.insert(ctx);

					// add new register info
					neededRegs.push_back(registerInfo);
				}
			}

			// remove non-mutual registers from m_registers
			for (auto& regInfo : regs2) {
				delete regInfo.m_expr;
			}
			regs2.clear();

			// insert needed registers into m_registers
			regs2.insert(regs2.begin(), neededRegs.begin(), neededRegs.end());
		}
	}
}

std::list<RegisterExecContext::RegisterPart> RegisterExecContext::findRegisterParts(int regId, BitMask64& needReadMask) {
	std::list<RegisterInfo*> sameRegisters;

	//select same registeres
	auto it = m_registers.find(regId);
	if (it != m_registers.end()) {
		auto& registers = it->second;
		for (auto& regInfo : registers) {
			sameRegisters.push_back(&regInfo);
		}
	}

	//sort asc
	sameRegisters.sort([](RegisterInfo* a, RegisterInfo* b) {
		return a->m_register.m_valueRangeMask < b->m_register.m_valueRangeMask;
		});

	//gather need parts
	std::list<RegisterPart> regParts;
	for (auto sameRegInfo : sameRegisters) {
		auto sameRegExceptionMask = GetValueRangeMaskWithException(sameRegInfo->m_register); //for x86 only!!!
																							 //if the masks intersected
		if (!(needReadMask & sameRegExceptionMask).isZero()) {
			sameRegInfo->m_using = RegisterInfo::REGISTER_FULLY_USING;

			RegisterPart part;
			part.m_regMask = sameRegInfo->m_register.m_valueRangeMask;
			part.m_maskToChange = needReadMask & sameRegExceptionMask;
			part.m_expr = sameRegInfo->m_expr->getNode();
			regParts.push_back(part);
			needReadMask = needReadMask & ~sameRegExceptionMask;
		}

		if (needReadMask == 0)
			break;
	}

	return regParts;
}

BitMask64 RegisterExecContext::calculateMaxMask(const std::list<RegisterInfo>& regs) {
	BitMask64 mask;
	for (auto reg : regs) {
		mask = mask | reg.m_register.m_valueRangeMask;
	}
	return mask;
}

std::list<BitMask64> RegisterExecContext::FindNonIntersectedMasks(const std::list<RegisterInfo>& regs) {
	std::list<BitMask64> masks;
	for (const auto& reg : regs) {
		masks.push_back(reg.m_register.m_valueRangeMask);
	}
	for (auto it1 = masks.begin(); it1 != masks.end(); it1++) {
		for (auto it2 = std::next(it1); it2 != masks.end(); it2++) {
			if (!(*it1 & *it2).isZero()) {
				*it1 = *it1 | *it2;
				masks.erase(it2);
			}
		}
	}
	return masks;
}

std::set<BitMask64> RegisterExecContext::CalculateMasks(const std::list<RegisterInfo>& regs1, const std::list<RegisterInfo>& regs2) {
	auto masks1 = FindNonIntersectedMasks(regs1);
	auto masks2 = FindNonIntersectedMasks(regs2);
	std::set<BitMask64> resultMasks;
	for (auto mask1 : masks1) {
		for (auto mask2 : masks2) {
			if (!(mask1 & mask2).isZero())
				resultMasks.insert(mask1 & mask2);
		}
	}
	return resultMasks;
}

ExprTree::INode* RegisterExecContext::CreateExprFromRegisterParts(std::list<RegisterPart> regParts, BitMask64 requestRegMask) {
	ExprTree::INode* resultExpr = nullptr;

	//descending sort
	regParts.sort([](RegisterPart& a, RegisterPart& b) {
		return b.m_regMask < a.m_regMask;
		});

	//in most cases bitRightShift = 0
	const int bitRightShift = requestRegMask.getOffset();
	for (auto& regPart : regParts) {
		auto regExpr = regPart.m_expr;
		const int bitLeftShift = regPart.m_regMask.getOffset(); //e.g. if we requiest only AH,CH... registers.
		const auto bitShift = bitRightShift - bitLeftShift;

		//regMask = 0xFFFFFFFF, maskToChange = 0xFFFF0000: expr(eax) | expr(ax) => (expr1 & 0xFFFF0000) | expr2
		if ((regPart.m_regMask & regPart.m_maskToChange) != regPart.m_regMask) {
			auto mask = (regPart.m_regMask & regPart.m_maskToChange) >> bitLeftShift;
			regExpr = new ExprTree::OperationalNode(regExpr, new ExprTree::NumberLeaf(mask.getValue(), regExpr->getSize()), ExprTree::And);
		}

		if (bitShift != 0) {
			regExpr = new ExprTree::OperationalNode(regExpr, new ExprTree::NumberLeaf(static_cast<uint64_t>(abs(bitShift)), regExpr->getSize()), bitShift > 0 ? ExprTree::Shr : ExprTree::Shl);
		}

		if (resultExpr) {
			resultExpr = new ExprTree::OperationalNode(resultExpr, regExpr, ExprTree::Or);
		}
		else {
			resultExpr = regExpr;
		}
	}
	return resultExpr;
}

ExecContext::~ExecContext() {
	m_startRegisterExecCtx.clear();
	m_registerExecCtx.clear();

	for (auto& pair : m_symbolVarnodes) {
		const auto topNode = pair.second;
		delete topNode;
	}
}

ExprTree::INode* ExecContext::requestVarnode(Varnode* varnode) {
	if (const auto registerVarnode = dynamic_cast<RegisterVarnode*>(varnode)) {
		return m_registerExecCtx.requestRegister(registerVarnode->m_register);
	}
	if (const auto symbolVarnode = dynamic_cast<SymbolVarnode*>(varnode)) {
		const auto it = m_symbolVarnodes.find(symbolVarnode);
		if (it != m_symbolVarnodes.end()) {
			const auto topNode = it->second;
			return topNode->getNode();
		}
	}
	if (auto varnodeConstant = dynamic_cast<ConstantVarnode*>(varnode)) {
		return new ExprTree::NumberLeaf(varnodeConstant->m_value, varnodeConstant->getMask().getSize());
	}
	return nullptr;
}

void ExecContext::setVarnode(Varnode* varnode, ExprTree::INode* newExpr) {
	if (const auto registerVarnode = dynamic_cast<RegisterVarnode*>(varnode)) {
		m_registerExecCtx.setRegister(registerVarnode->m_register, newExpr);
	}
	if (const auto symbolVarnode = dynamic_cast<SymbolVarnode*>(varnode)) {
		TopNode* topNode = nullptr;
		const auto it = m_symbolVarnodes.find(symbolVarnode);
		if (it != m_symbolVarnodes.end()) {
			topNode = it->second;
		}

		m_symbolVarnodes[symbolVarnode] = new TopNode(newExpr);

		// if {newExpr} == {topNode->getNode()}
		if (topNode)
			delete topNode;
	}
}

void ExecContext::join(ExecContext* ctx) {
	if (!m_registerExecCtx.m_isFilled) {
		m_registerExecCtx.copyFrom(&ctx->m_registerExecCtx);
	}
	else {
		m_registerExecCtx.join(&ctx->m_registerExecCtx);
	}
}
