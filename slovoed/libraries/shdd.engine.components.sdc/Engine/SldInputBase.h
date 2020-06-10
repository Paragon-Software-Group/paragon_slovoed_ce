#ifndef _SLD_INPUT_BASE_H_
#define _SLD_INPUT_BASE_H_

#include "ISldInput.h"
#include "SldBitInput.h"

class CSldInputBase : public ISldInput
{
public:
	CSldInputBase() : m_HASH(0) { sld2::memzero(m_QAHeader); }

	/// Инициализация
	ESldError Init(CSDCReadMy &aData, UInt32 aDataType, UInt32 aAssistType, UInt32 aAssistCount, UInt32 aQAType, UInt32 aListVersion) override;

	/// Устанавливаем HASH для декодирования данного списка слов.
	void SetHASH(UInt32 aHASH) override;

	/// Получаем количество точек по которым может производится поиск
	UInt32 GetSearchPointCount() override;

	/// Получаем текст для указанной точки
	ESldError GetSearchPointText(UInt32 aPointIndex, const UInt16 **aText) override;

	/// Получаем номер слова для указанной точки
	ESldError GetSearchPointIndex(UInt32 aPointIndex, Int32 *aIndex) override;

	/// Получаем начало слова для указанной точки
	ESldError GetSearchPointText(UInt32 aPointIndex, UInt16 *aWord) override;

	/// Производим переход внутреннего состояния в позицию точки из таблицы быстрого поиска.
	ESldError GoTo(UInt32 aPointIndex) override;

	/// Производит получение данных, до 32 бит.
	ESldError GetData(UInt32 *aDataBuffer, UInt32 aDataSize) override;

	/// Возвращает текущее положение в битах.
	UInt32 GetCurrentPosition() override;

	/// Перемещаем указатель текущей позиции чтения в указанную позицию
	ESldError SetCurrentPosition(UInt32 aShift) override;

protected:
	/// HASH который используется для правильного перемещения по закодированным данным.
	UInt32 m_HASH;

	/// Заголовок таблицы быстрого поиска
	TQAHeader	m_QAHeader;
	/// Данные таблицы быстрого поиска (реальный тип зависит от m_QAHeader.Type)
	sld2::DynArray<UInt8>	m_QA;

	/// Объект получения бинарных данных.
	CSldBitInput	m_input;

	template <typename Decoder>
	ESldError decode(const void *decoderData, sld2::Span<UInt16> aBuffer, UInt32 *aLength)
	{
		struct BitInput
		{
			ESldError error;
			CSldBitInput &input;

			BitInput(CSldBitInput &input) : error(eOK), input(input) {}

			inline UInt32 read(unsigned size)
			{
				UInt32 data;
				error = input.GetData(&data, size);
				return error == eOK ? data : ~0u;
			}
		} bits(m_input);

		const UInt32 length = Decoder::decode(decoderData, bits, aBuffer);
		if (length == 0)
			return bits.error != eOK ? bits.error : eCommonWrongCharIndex;

		// decoders return the count of decoded symbols *including* the nul-terminator
		// so we have to substract 1 as we return the "string length" here
		if (aLength)
			*aLength = length - 1;

		return eOK;
	}
};

#endif // _SLD_INPUT_BASE_H_

