#ifndef _SLD_SPEEX_DECODER_H_
#define _SLD_SPEEX_DECODER_H_

#include "SldPlatform.h"
#include "SldTypes.h"
#include "ISldLayerAccess.h"
#include "SldError.h"
#include "Speex/speex.h"


#define MAX_FRAME_SIZE			(640)
#define SS_OUT_DATA_INTERLEAVE	(20)


/// Производит декодирование озвучки, сжатой speex
ESldError SpeexDecode(ISldLayerAccess* aLayerAccess, FSoundBuilderMethodPtr aBuilderPtr, const UInt8* aDataPtr, UInt32 aDataSize, UInt8 aIsLast, UInt32* aStartPos, UInt8 aBeginPlay = 0);

/// Класс для декодирования озвучки частями по требованию
class SldSpxDecoder
{
public:
	/// Конструктор
	SldSpxDecoder(void);
	/// Деструктор
	~SldSpxDecoder(void);

	/// Инициализация ресурсом с озвучкой
	ESldError Init(const CSDCReadMy::Resource &aRes);
	/// Инициализация ресурсом с озвучкой
	ESldError Init(const UInt8 *aData, UInt32 aSize);

	/// Установка позиции, с которой будет производится декодировка
	void SetPos(UInt32 aPos);

	/// Получение нового кусочка декодированных данных
	ESldError GetNextBuff(UInt8** aBuf, UInt32* aSize);

	/// Сброс декодировщика
	ESldError Reset(void);

	inline UInt16	GetPacketCount(void)	const	{return m_PacketCount;};
	inline UInt8	IsInitialized(void)		const	{return m_Initialized;};
	inline UInt32	GetSampleRate(void)		const	{return m_SampleRate;};

protected:
	/// Подготовка декодировщика
	ESldError PrepareDecoder(void);

protected:
	/// Начальная позиция декодирования
	/// в процентах
	UInt32	m_StartPos;
	/// в фреймах
	UInt16	m_StartFrame;
	/// Количество блоков для декодирования
	UInt16	m_PacketCount;
	/// Размер фрейма
	UInt32	m_FrameSize;
	/// Внутренний буфер для декодированных данных
	UInt8*	m_Buffer;
	/// Размер внутреннего буфера
	UInt32	m_BufferSize;
	/// Ресурсы с озвучкой из базы
	const UInt8*	m_DataPtr;
	///
	CSDCReadMy::Resource m_Res;
	/// Размер пакета декодированных данных
	UInt8 m_PacketSize;
	/// Bit-packing data structure representing (part of) a bit-stream.
	SpeexBits m_Bits;
	/// Speex Decoder
	void* m_St;
	/// Частота звукового потока
	UInt32 m_SampleRate;
	/// Размер собственно звуковых данных без заголовка TSoundFileHeader и специфического заголовка wav или spx
	UInt32 m_SoundDataSize;
	/// флаг инициализации
	UInt8 m_Initialized;
};

#endif //_SLD_SPEEX_DECODER_H_
