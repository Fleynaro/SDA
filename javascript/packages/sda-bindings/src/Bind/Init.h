#include "node.h"
#include <v8pp/json.hpp>
#include <v8pp/module.hpp>
#include <v8pp/object.hpp>
#include "Convert.h"
#include "ObjectExport.h"
#include "Call.h"

namespace sda::bind
{
    void InitModule(v8::Local<v8::Object> module, std::list<std::function<void(v8pp::module&)>> inits);
};