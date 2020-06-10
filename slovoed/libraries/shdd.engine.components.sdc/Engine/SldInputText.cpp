#include "SldInputText.h"

#include "SldTextDecoders.h"

/** ********************************************************************
* Производит получение текста(вплоть до терминирующего 0).
*
* @param[in]	aDataType	- указание декодеру какие данные ожидаются
* @param[out]	aBuffer		- буфер в который необходимо поместить строчку после декодирования
* @param[out]	aLength		- длина декодированой строки
*
* @return код ошибки
************************************************************************/
ESldError CSldInputText::GetText(UInt32 aDataType, sld2::Span<UInt16> aBuffer, UInt32 *aLength)
{
	return decode<sld2::decoders::CharStore>(nullptr, aBuffer, aLength);
}
