#include "SharedData.h"

std::map<size_t, void*> sda::bind::SharedData::Data;

void* sda::bind::SharedData::Get(size_t key) {
    auto it = Data.find(key);
    if (it != Data.end()) {
        return it->second;
    }
    return nullptr;
}

void sda::bind::SharedData::Set(size_t key, void* data) {
    if (data == nullptr) {
        Data.erase(key);
    } else {
        Data[key] = data;
    }
}