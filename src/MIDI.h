#pragma once
#include "PolledResource.h"

class IScope;

class MIDI {
public:
	static void init(std::string_view filename);
	static bool active();
	static void poll();
	static IScope &getScope();
};
