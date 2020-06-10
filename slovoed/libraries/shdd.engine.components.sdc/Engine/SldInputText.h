#pragma once
#include "SldInputBase.h"

/// Реализация класса получающего данные
class CSldInputText : public CSldInputBase
{
public:
	/// Производит получение текста(вплоть до терминирующего 0).
	ESldError GetText(UInt32 aDataType, sld2::Span<UInt16> aBuffer, UInt32 *aLength) override;
};
