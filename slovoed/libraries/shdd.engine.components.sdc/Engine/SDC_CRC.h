﻿#ifndef _SLD_SDC_CRC_H_
#define _SLD_SDC_CRC_H_

#include "SDC.h"

/// Начальное значение при вычислении CRC32
#define SDC_CRC32_START_VALUE	(0xFFFFFFFFUL)


/// Функция вычисления контрольной суммы.
UInt32 CRC32(const UInt8 *buf, UInt32 size, UInt32 start_value, bool inverse_flag = true);

#endif