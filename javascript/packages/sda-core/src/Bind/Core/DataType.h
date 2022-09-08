#pragma once
#include "Core/DataType/VoidDataType.h"
#include "Core/DataType/PointerDataType.h"
#include "Core/DataType/ArrayDataType.h"
#include "Core/DataType/ScalarDataType.h"
#include "Core/DataType/TypedefDataType.h"
#include "Core/DataType/EnumDataType.h"
#include "Core/DataType/StructureDataType.h"
#include "Core/DataType/SignatureDataType.h"

namespace sda::bind
{
    class DataTypeBind : public ContextObjectBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<DataType> cl(module.isolate());
            cl
                .inherit<ContextObject>()
                .property("baseType", &DataType::getBaseType)
                .property("isVoid", &DataType::isVoid)
                .property("isPointer", &DataType::isPointer)
                .property("isFloatingPoint", [](DataType& self) { return self.isScalar(ScalarType::FloatingPoint); })
                .property("isSigned", [](DataType& self) { return self.isScalar(ScalarType::SignedInt); })
                .property("isUnsigned", [](DataType& self) { return self.isScalar(ScalarType::UnsignedInt); })
                .property("size", &DataType::getSize)
                .method("getPointerTo", &DataType::getPointerTo)
                .method("getArrayOf", &DataType::getArrayOf);
            module.class_("DataType", cl);
        }
    };

    class VoidDataTypeBind : public DataTypeBind
    {
        static auto New(Context* ctx) {
            return new VoidDataType(ctx);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<VoidDataType> cl(module.isolate());
            cl
                .inherit<DataType>()
                .static_method("New", &New);
            module.class_("VoidDataType", cl);
        }
    };

    class PointerDataTypeBind : public DataTypeBind
    {
        static auto New(Context* ctx, DataType* pointedType) {
            return new PointerDataType(ctx, nullptr, pointedType);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<PointerDataType> cl(module.isolate());
            cl
                .inherit<DataType>()
                .property("pointedType", &PointerDataType::getPointedType)
                .static_method("New", &New);
            module.class_("PointerDataType", cl);
        }
    };

    class ArrayDataTypeBind : public DataTypeBind
    {
        static auto New(Context* ctx, DataType* elementType, const std::list<size_t>& dimensions) {
            return new ArrayDataType(ctx, nullptr, elementType, dimensions);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<ArrayDataType> cl(module.isolate());
            cl
                .inherit<DataType>()
                .property("elementType", &ArrayDataType::getElementType)
                .property("dimensions", &ArrayDataType::getDimensions)
                .static_method("New", &New);
            module.class_("ArrayDataType", cl);
        }
    };

    class ScalarDataTypeBind : public DataTypeBind
    {
        static auto New(Context* ctx, const std::string& name, ScalarType scalarType, size_t size) {
            return new ScalarDataType(ctx, nullptr, name, scalarType, size);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<ScalarDataType> cl(module.isolate());
            cl
                .inherit<DataType>()
                .property("scalarType", &ScalarDataType::getScalarType)
                .static_method("New", &New);
            module.class_("ScalarDataType", cl);
        }
    };

    class TypedefDataTypeBind : public DataTypeBind
    {
        static auto New(Context* ctx, const std::string& name, DataType* refType) {
            return new TypedefDataType(ctx, nullptr, name, refType);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<TypedefDataType> cl(module.isolate());
            cl
                .inherit<DataType>()
                .property("refType", &TypedefDataType::getReferenceType)
                .static_method("New", &New);
            module.class_("TypedefDataType", cl);
        }
    };

    class EnumDataTypeBind : public DataTypeBind
    {
        static auto New(Context* ctx, const std::string& name, const std::map<EnumDataType::Key, std::string>& fields) {
            return new EnumDataType(ctx, nullptr, name, fields);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<EnumDataType> cl(module.isolate());
            cl
                .inherit<DataType>()
                .property("fields", &EnumDataType::getFields, &EnumDataType::setFields)
                .static_method("New", &New);
            module.class_("EnumDataType", cl);
        }
    };

    class StructureDataTypeBind : public DataTypeBind
    {
        static auto New(Context* ctx, const std::string& name, size_t size) {
            auto symbolTable = new StandartSymbolTable(ctx);
            return new StructureDataType(ctx, nullptr, name, size, symbolTable);
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<StructureDataType> cl(module.isolate());
            cl
                .inherit<DataType>()
                .property("size", &StructureDataType::getSize, &StructureDataType::setSize)
                .property("symbolTable", &StructureDataType::getSymbolTable)
                .static_method("New", &New);
            module.class_("StructureDataType", cl);
        }
    };

    // class SignatureDataTypeBind : public DataTypeBind
    // {
    //     static auto New(Context* ctx, const std::string& name, DataType* returnType, const std::vector<FunctionParameterSymbol*>& parameters) {
    //         return new SignatureDataType(ctx, nullptr, returnType, argumentTypes);
    //     }
    // public:
    //     static void Init(v8pp::module& module) {
    //         v8pp::class_<SignatureDataType> cl(module.isolate());
    //         cl
    //             .inherit<DataType>()
    //             .property("returnType", &SignatureDataType::getReturnType)
    //             .property("argumentTypes", &SignatureDataType::getArgumentTypes)
    //             .static_method("New", &New);
    //         module.class_("SignatureDataType", cl);
    //     }
    // };
};