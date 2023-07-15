#pragma once
#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/Symbol/FunctionSymbol.h"
#include "SDA/Core/DataType/ScalarDataType.h"
#include "SDA/Core/DataType/PointerDataType.h"
#include "SDA/Core/DataType/ArrayDataType.h"
#include "SDA/Core/DataType/StructureDataType.h"

namespace sda::semantics
{
    struct SemanticsPropagationContext {
        const ircode::Operation* operation = nullptr;
        std::map<ircode::Function*, std::list<const ircode::Operation*>> nextOperations;

        void addNextOperation(const ircode::Operation* operation) {
            auto func = operation->getBlock()->getFunction();
            auto it = nextOperations.find(func);
            if (it == nextOperations.end()) {
                it = nextOperations.insert(
                    std::make_pair(func, std::list<const ircode::Operation*>())).first;
            }
            auto& ops = it->second;
            if (std::find(ops.begin(), ops.end(), operation) == ops.end()) {
                ops.push_back(operation);
            }
        }

        void markValueAsAffected(std::shared_ptr<ircode::Value> value) {
            for (auto op : value->getOperations()) {
                addNextOperation(op);
            }
        }

        void collect(std::function<void()> collector) {
            while(!nextOperations.empty()) {
                auto it = nextOperations.begin();
                while (it != nextOperations.end()) {
                    auto& ops = it->second;
                    auto it2 = ops.begin();
                    while (it2 != ops.end()) {
                        operation = *it2;
                        collector();
                        it2 = ops.erase(it2);
                    }
                    it = nextOperations.erase(it);
                }
            }
        }
    };
};
