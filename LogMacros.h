#pragma once
#include <iostream>
#include <format>

#define LOGGING

#ifdef LOGGING
	#define UE_LOG(Category, Level, ...) { cout << "IodineV2:" << Category << ":" Level << ": " << format(__VA_ARGS__) << "\n"; }
#else
	#define UE_LOG(Category, Level, ...) {}
#endif // LOGGING

