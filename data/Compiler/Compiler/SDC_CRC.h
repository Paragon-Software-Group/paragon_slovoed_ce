#pragma once

#include "Engine/SDC.h"

/// ��������� �������� ��� ���������� CRC32
#define SDC_CRC32_START_VALUE	(0xFFFFFFFFUL)


/// ������� ���������� ����������� �����.
UInt32 CRC32(const UInt8 *buf, UInt32 size, UInt32 start_value, bool inverse_flag = true);
