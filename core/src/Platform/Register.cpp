#include "SDA/Core/Platform/Register.h"
#include "SDA/Core/Platform/RegisterRepository.h"
#include <sstream>

using namespace sda;

Register::Register(Type type, size_t id, size_t index, utils::BitMask mask)
    : m_type(type), m_id(id), m_index(index), m_mask(mask), m_size(mask.getSize())
{}

Register::Type Register::getRegType() const {
    return m_type;
}

size_t Register::getRegId() const {
    return m_id;
}

size_t Register::getRegIndex() const {
    return m_index;
}

utils::BitMask Register::getMask() const {
    return m_mask;
}

size_t Register::getSize() const {
    return m_size;
}

size_t Register::getBitOffset() const {
    if (m_type == Register::Virtual)
        return m_mask.getOffset();
    return m_mask.getOffset() + m_index * 64;
}

std::string Register::toString(const RegisterRepository* regRepo, bool printSizeAndOffset) const {
    std::stringstream ss;
    if (m_type == Register::Virtual) {
        ss << "$U" << (m_index + 1);
    } else {
        if (m_type == Register::Flag) {
            ss << regRepo->getRegisterFlagName(m_mask);
        } else {
            ss << regRepo->getRegisterName(m_id);
        }
    }
    if (printSizeAndOffset) {
        if (m_type == Register::Vector || m_type == Register::Flag) {
            if (m_size == 4 || m_size == 8) {
                ss << ":";
                ss << (m_size == 4 ? "D" : "Q");
                ss << static_cast<char>('a' + static_cast<char>(getBitOffset() / (m_size * utils::BitsInBytes)));
            }
        } else {
            ss << ":" << m_size;
            if (auto offset = getBitOffset()) {
                ss << ":" << offset;
            }
        }
    }
    return ss.str();
}

Register Register::Create(const RegisterRepository* regRepo, size_t registerId, size_t size) {
    auto regType = regRepo->getRegisterType(registerId);
    auto regMask = utils::BitMask(size, 0);
    return sda::Register(regType, registerId, 0, regMask);
}
