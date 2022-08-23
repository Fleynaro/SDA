#include "Core/IRcode/IRcodeLinearExpr.h"
#include "Core/IRcode/IRcodeValue.h"
#include "Core/DataType/PointerDataType.h"
#include <stdexcept>

using namespace sda;
using namespace sda::ircode;

LinearExpression::LinearExpression(size_t constTermValue)
    : m_constTermValue(constTermValue)
{}

LinearExpression::LinearExpression(std::shared_ptr<Value> value, size_t factor) {
    m_terms.push_back({ value, factor });
}

const std::list<LinearExpression::Term>& LinearExpression::getTerms() const {
    return m_terms;
}

size_t LinearExpression::getConstTermValue() const {
    return m_constTermValue;
}

bool LinearExpression::isDivisibleBy(size_t factor, bool exceptConstTerm) const {
    if (!exceptConstTerm && m_constTermValue % factor != 0)
        return false;
    bool isBaseTermConsidered = false;
    for (auto& term : m_terms) {
        if (!isBaseTermConsidered && term.factor == 1) {
            isBaseTermConsidered = true;
            continue;
        }
        if (term.factor % factor != 0)
            return false;
    }
    return true;
}

bool LinearExpression::isArrayType() const {
    return m_terms.size() > 1;
}

LinearExpression LinearExpression::operator+(const LinearExpression& other) const {
    LinearExpression result = *this;
    if (!other.m_terms.empty()) {
        auto termMap = result.getTermMap();
        for (const auto& term : other.m_terms) {
            auto it = termMap.find(term.value->getHash());
            if (it == termMap.end()) {
                result.m_terms.push_back(term);
            } else {
                it->second->factor += term.factor;
            }
        }
    }
    result.m_constTermValue += other.m_constTermValue;
    return result;
}

LinearExpression LinearExpression::operator*(const LinearExpression& other) const {
    if (other.getTerms().empty())
        return *this * other.getConstTermValue();
    if (m_terms.empty())
        return other * m_constTermValue;
    throw std::runtime_error("Only scalar multiplication is supported");
}

LinearExpression LinearExpression::operator*(size_t factor) const {
    LinearExpression result = *this;
    for (auto& term : result.m_terms) {
        term.factor *= factor;
    }
    result.m_constTermValue *= factor;
    return result;
}

std::map<Hash, LinearExpression::Term*> LinearExpression::getTermMap() {
    std::map<Hash, Term*> result;
    for (auto& term : m_terms) {
        result[term.value->getHash()] = &term;
    }
    return result;
}