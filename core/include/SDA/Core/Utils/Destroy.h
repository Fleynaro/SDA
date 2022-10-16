#pragma once

namespace utils
{
    // Interface for objects that can be destroyed
    class IDestroyable
    {
    public:
        // Destroy the object
        virtual void destroy() = 0;
    };
};