#ifndef _SLD_TOOLS_H_
#define _SLD_TOOLS_H_
#include "SldPlatform.h"
#include "SldTypes.h"
#include "ISldLayerAccess.h"
#include "ISldInput.h"
#include "SldUtil.h"


/// Инициализирует генератор случайных чисел.
void SldInitRandom(TRandomSeed *aRandomSeed, UInt32 aSeed);

/// Генерирует случайное число основываясь на начальном числе. Начальное число будет изменено.
UInt32 SldGetRandom(TRandomSeed *aRandomSeed);

/// Загружает регистрационные данные для текущего словаря.
ESldError SldLoadRegistrationData(UInt32 aDictID, TRegistrationData *aRegData, ISldLayerAccess *aLayerAccess);

/// Сохраняет регистрационные данные для указанного словаря.
ESldError SldSaveRegistrationData(UInt32 aDictID, const TRegistrationData *aRegData, TRandomSeed *aRandomSeed, ISldLayerAccess *aLayerAccess);

/// Макрос выбора указателя на нужную функцию сборки перевода либо озвучки в зависимости от регистрационных данных
#define SLD_VALIDATE_SECURITY(aRegData, aRandSeed, aFunc) { \
	UInt8 Index = 1; \
	if ((aRegData) && (((aRegData)->Number)>>SLD_SN_REGISTRATION_FLAG_SHIFT)) { \
		UInt32 rnd = ((SldGetRandom(aRandSeed))%(1<<SLD_SN_CLICKS_MAX)); \
		if ((rnd > (aRegData)->Clicks) || ((rnd&((1<<SLD_SN_CLICKS_MIN)-1))==0)) \
			Index = 2; \
	} \
	else \
		Index = 2; \
	aFunc[0] = aFunc[Index]; \
}

/// Макрос для изменения порядка байт
#define ByteSwap32(n) ( ((((UInt32)n) << 24) & 0xFF000000) | \
	((((UInt32)n) << 8) & 0x00FF0000) | \
	((((UInt32)n) >> 8) & 0x0000FF00) | \
	((((UInt32)n) >> 24) & 0x000000FF) )

/// Макрос для изменения порядка байт
#define ByteSwap16(n) ((((UInt16)n) << 8) | (((UInt16)n) >> 8))

namespace sld2 {

/**
 * Создаем массив элементов определенного типа копируя их заданной области памяти
 *
 * @param[in]  aData  - указатель на блок памяти откуда читать элементы массива
 * @param[in]  aCount - количество элементов массива
 * @param[in]  aSize  - размер одного *исходного* элемента массива
 * @param[out] aOut   - указатель по которому будет записана копия массива
 *
 * Основная идея функции в возможности создания копии где элементы "выходного" массива
 * *больше* исходных, т.е. sizeof(T) >= aSize.
 *
 * @return код ошибки:
 *           eOK                    - все ок, память выделена все скопировано
 *                                    (также возвращается при 0 размере исходного массива)
 *           eCommonWrongSizeOfData - размер исходных элементов больше размера элементов копии
 *           eMemoryNotEnoughMemory - ошибка выделения памяти под копию
 */
template <typename T>
static inline ESldError arrayCopy(const void *aData, UInt32 aCount, UInt32 aSize, sld2::DynArray<T> *aOut)
{
	if (aCount == 0)
		return eOK;

	if (aSize > sizeof(T))
		return eCommonWrongSizeOfData;

	if (!aOut->resize(aCount))
		return eMemoryNotEnoughMemory;

	if (aSize == sizeof(T))
	{
		sld2::memcopy_n(aOut->data(), (const T*)aData, aCount);
	}
	else
	{
		const UInt8 *data = (const UInt8*)aData;
		for (UInt32 i = 0; i < aCount; i++)
			sldMemCopy(&(*aOut)[i], data + i * aSize, aSize);
	}

	return eOK;
}

/// Создает объект чтения данных согласно методу сжатия(представления)
UniquePtr<ISldInput> CreateInput(UInt32 aCompressionMethod);

} // namespace sld2

#endif //_SLD_TOOLS_H_
