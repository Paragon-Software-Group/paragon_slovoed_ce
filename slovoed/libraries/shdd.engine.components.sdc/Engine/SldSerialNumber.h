#pragma once
#include "SldError.h"
#include "SldDefines.h"
#include "SldTypes.h"
#include "SldSDCReadMy.h"

/// Класс работы со строками.
class CSldSerialNumber
{
public:

	/// Сравнение 2 строк с учетом регистра
	static ESldError ParseSerialNumber(const UInt16 *aSerialNumber,
		UInt16 *aPlatformID, UInt32 *aDictID, UInt32 *aNumber, UInt32 *aHASH, UInt32 *aDate, UInt32 *aIsBundle);

	/// Расчитывает CRC для одного символа.
	static UInt32 QuasiCRC32(UInt32 n);
private:

	static void Decimal2Long(const UInt8 *str, const UInt16 str_len, UInt8 *d_array, UInt16 d_array_len);
	static void DataDemix(UInt32 Mix, UInt16 *CRC, UInt16 *Index);
	static UInt16 CRCcount(UInt16 ch);
};
