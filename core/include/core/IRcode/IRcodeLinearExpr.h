#pragma once
#include <list>
#include <map>
#include <memory>

namespace sda::ircode
{
    using Hash = size_t;

    class Value;
    class LinearExpression {
        struct Term {
            std::shared_ptr<Value> value;
            size_t factor = 1;
        };
        std::list<Term> m_terms;
        size_t m_constTermValue = 0;
    public:
        LinearExpression() = default;

        LinearExpression(size_t constTermValue);

        LinearExpression(std::shared_ptr<Value> value, size_t factor = 1);

        const std::list<Term>& getTerms() const;

        size_t getConstTermValue() const;

        bool isDivisibleBy(size_t factor, bool exceptConstTerm = false) const;

        bool isArrayType() const;

        LinearExpression operator+(const LinearExpression& other) const;

        LinearExpression operator*(const LinearExpression& other) const;

        LinearExpression operator*(size_t factor) const;

    private:
        std::map<Hash, Term*> getTermMap();
    };
};