#ifndef _VERSION_INFO_H_
#define _VERSION_INFO_H_

#include "Engine/SldVersionInfo.h"

/// ����� ������ �����������
#define COMPILER_BUILD		SLOVOED2_ENGINE_BUILD

enum CompilerVersion
{
	// � ���� ������ �������� ������
	COMPILER_VERSION__BASE = SLOVOED2_ENGINE_VERSION,

	/// ����� ��������� ������
	__COMPILER_VERSION__NEXT,
	/// ����� ������� ������
	COMPILER_VERSION__NOW = __COMPILER_VERSION__NEXT - 1,
};

#endif //_VERSION_INFO_H_
