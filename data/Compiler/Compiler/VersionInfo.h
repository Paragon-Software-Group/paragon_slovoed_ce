#ifndef _VERSION_INFO_H_
#define _VERSION_INFO_H_

#include "Engine/SldVersionInfo.h"

/// Номер сборки компилятора
#define COMPILER_BUILD		SLOVOED2_ENGINE_BUILD

enum CompilerVersion
{
	// С этой версии начинаем отсчет
	COMPILER_VERSION__BASE = SLOVOED2_ENGINE_VERSION,

	/// Номер следующей версии
	__COMPILER_VERSION__NEXT,
	/// Номер текущей версии
	COMPILER_VERSION__NOW = __COMPILER_VERSION__NEXT - 1,
};

#endif //_VERSION_INFO_H_
