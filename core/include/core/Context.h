#pragma once
#include "Object.h"

namespace sda
{
    // Interface for context
    class IContext
    {
    public:
        // Get name of the module
        virtual std::string getName() const = 0;
    };

    class FunctionList;

    // Core context that contains all important entities
    class Context : public IContext
    {
        std::unique_ptr<FunctionList> m_functions;
    public:
        Context();

        // Get name of the module
        std::string getName() const override;

        // Get the list of functions
        FunctionList* getFunctions();
    };
};