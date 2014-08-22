#pragma once

//#define XGAME_API
#ifdef EXPORT_DLL
#define XGAME_API		__declspec(dllexport)
#else
#define XGAME_API
#endif