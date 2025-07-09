#pragma once

#pragma warning(disable: 4251)

#if defined(_WIN32) || defined(_WIN64)
	#ifdef ENGINE_BUILD_DLL
		#define EngineAPI __declspec(dllexport)
	#else
		#define EngineAPI __declspec(dllimport)
	#endif
#else
	#error only windows supported!
#endif