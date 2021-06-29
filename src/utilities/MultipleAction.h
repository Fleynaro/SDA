#pragma once
#include <main.h>

namespace Utils
{
	template<typename T>
	static void actionForList(std::initializer_list<T*> eventHandlers, const std::function<void(T*)>& callback) {
		for (auto eventHandler : eventHandlers) {
			if (eventHandler == nullptr)
				continue;
			callback(eventHandler);
		}
	}
};