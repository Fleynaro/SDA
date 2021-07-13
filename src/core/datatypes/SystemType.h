#pragma once
#include "AbstractType.h"

namespace CE
{
	namespace DataType
	{
		class SystemType : public AbstractType, virtual public DB::IDomainObject
		{
		public:
			enum Set
			{
				Undefined,
				Boolean,
				Integer,
				Real
			};

			enum Types : int
			{
				Void = 1,
				Bool,
				Byte,
				Int8,
				Int16,
				Int32,
				Int64,
				UInt16,
				UInt32,
				UInt64,
				UInt128,
				Float,
				Double,

				Char,
				WChar
			};

			SystemType()
				: AbstractType(nullptr, "", "")
			{}

			DB::Id getId() override {
				return getTypeId();
			}

			virtual Types getTypeId() = 0;

			virtual Set getSet() = 0;

			Group getGroup() override {
				return Simple;
			}

			std::string getDisplayName() override {
				return getName();
			}

			bool isUserDefined() override {
				return false;
			}
		};

		class Void : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Void;
			}

			const std::string getName() override {
				return "void";
			}

			const std::string getComment() override {
				return "void is a special return type for functions only";
			}

			Set getSet() {
				return Undefined;
			}

			int getSize() override {
				return 0;
			}
		};

		class Bool : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Bool;
			}

			const std::string getName() override {
				return "bool";
			}

			const std::string getComment() override {
				return "bool is a byte type";
			}

			Set getSet() {
				return Boolean;
			}

			std::string getViewValue(uint64_t value) override {
				return (bool&)value ? "true" : "false";
			}

			int getSize() override {
				return 1;
			}
		};

		class Byte : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Byte;
			}

			const std::string getName() override {
				return "byte";
			}

			const std::string getComment() override {
				return "byte is a byte type";
			}

			Set getSet() {
				return Integer;
			}

			int getSize() override {
				return 1;
			}
		};

		class Int8 : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Int8;
			}

			const std::string getName() override {
				return "int8_t";
			}

			const std::string getComment() override {
				return "int8_t is a byte type";
			}

			Set getSet() {
				return Integer;
			}

			bool isSigned() override {
				return true;
			}

			std::string getViewValue(uint64_t value) override {
				return std::to_string((int8_t&)value);
			}

			int getSize() override {
				return 1;
			}
		};

		class Int16 : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Int16;
			}

			const std::string getName() override {
				return "int16_t";
			}

			const std::string getComment() override {
				return "int16_t is 2 byte type";
			}

			Set getSet() {
				return Integer;
			}

			bool isSigned() override {
				return true;
			}

			std::string getViewValue(uint64_t value) override {
				return std::to_string((int16_t&)value);
			}

			int getSize() override {
				return 2;
			}
		};

		class Int32 : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Int32;
			}

			const std::string getName() override {
				return "int32_t";
			}

			const std::string getComment() override {
				return "int32_t is 4 byte type";
			}

			Set getSet() {
				return Integer;
			}

			bool isSigned() override {
				return true;
			}

			std::string getViewValue(uint64_t value) override {
				return std::to_string((int32_t&)value);
			}

			int getSize() override {
				return 4;
			}
		};

		class Int64 : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Int64;
			}

			const std::string getName() override {
				return "int64_t";
			}

			const std::string getComment() override {
				return "int64_t is 8 byte type";
			}

			Set getSet() {
				return Integer;
			}

			bool isSigned() override {
				return true;
			}

			std::string getViewValue(uint64_t value) override {
				return std::to_string((int64_t&)value);
			}

			int getSize() override {
				return 8;
			}
		};

		class UInt16 : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::UInt16;
			}

			const std::string getName() override {
				return "uint16_t";
			}

			const std::string getComment() override {
				return "int16_t is 2 byte unsigned type";
			}

			Set getSet() {
				return Integer;
			}

			int getSize() override {
				return 2;
			}
		};

		class UInt32 : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::UInt32;
			}

			const std::string getName() override {
				return "uint32_t";
			}

			const std::string getComment() override {
				return "int32_t is 4 byte unsigned type";
			}

			Set getSet() {
				return Integer;
			}

			int getSize() override {
				return 4;
			}
		};

		class UInt64 : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::UInt64;
			}

			const std::string getName() override {
				return "uint64_t";
			}

			const std::string getComment() override {
				return "int64_t is 8 byte unsigned type";
			}

			Set getSet() {
				return Integer;
			}

			int getSize() override {
				return 8;
			}
		};

		class UInt128 : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::UInt128;
			}

			const std::string getName() override {
				return "uint128_t";
			}

			const std::string getComment() override {
				return "int128_t is 16 byte unsigned type";
			}

			Set getSet() {
				return Integer;
			}

			int getSize() override {
				return 8; // need 16, but not supported
			}
		};

		class Float : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Float;
			}

			const std::string getName() override {
				return "float";
			}

			const std::string getComment() override {
				return "float is 4 byte type";
			}

			Set getSet() {
				return Real;
			}

			bool isSigned() override {
				return true;
			}

			std::string getViewValue(uint64_t value) override {
				return std::to_string((float&)value);
			}

			int getSize() override {
				return 4;
			}
		};

		class Double : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Double;
			}

			const std::string getName() override {
				return "double";
			}

			const std::string getComment() override {
				return "double is 8 byte type";
			}

			Set getSet() {
				return Real;
			}

			bool isSigned() override {
				return true;
			}

			std::string getViewValue(uint64_t value) override {
				return std::to_string((double&)value);
			}

			int getSize() override {
				return 8;
			}
		};

		class Char : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::Char;
			}

			const std::string getName() override {
				return "char";
			}

			const std::string getComment() override {
				return "char is a byte type that used for ASCII strings";
			}

			Set getSet() {
				return Integer;
			}

			int getSize() override {
				return 1;
			}
		};

		class WChar : public SystemType
		{
		public:
			Types getTypeId() override {
				return SystemType::WChar;
			}

			const std::string getName() override {
				return "wchar_t";
			}

			const std::string getComment() override {
				return "wchar_t is a byte type that used for UNICODE strings";
			}

			Set getSet() {
				return Integer;
			}

			int getSize() override {
				return 2;
			}
		};
	};
};