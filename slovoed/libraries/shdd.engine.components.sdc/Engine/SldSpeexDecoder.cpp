#include "SldSpeexDecoder.h"

/***********************************************************************
* Производит декодирование озвучки, сжатой speex
*
* @param aLayerAccess	- Указатель на данные, необходимые для работы прослойки
* @param aBuilderPtr	- Указатель на функцию сборки озвучки
* @param aDataPtr		- Указатель на сами данные
* @param aDataSize		- Размер переданных данных
* @param aIsLast		- флаг, если установлен, значит данное слово 
*						  будет последним при декодировании звука и прослойка 
*						  получит команду начать воспроизведение
* @param aStartPos		- указатель на переменную, в которой хранится позиция, 
*						  в которую нужно начинать помещать декодированный звук.
*						  Если будет озвучено 1 слово или каждый раз данные будут 
*						  озвучиваться отдельно, можно передавать NULL.
* @param aBeginPlay		- обозначает с какого места проигрывать озвучку [0;100], в %
*
* @return код ошибки
************************************************************************/
ESldError SpeexDecode(ISldLayerAccess* aLayerAccess, FSoundBuilderMethodPtr aBuilderPtr, const UInt8* aDataPtr, UInt32 aDataSize, UInt8 aIsLast, UInt32* aStartPos, UInt8 aBeginPlay)
{
	Int32 frame_size = 0;
	UInt32 fsize = 0;
	UInt32 shift = 0;
	UInt32 StartPos = 0;
	Int16 packet_count = 0;
	UInt8 packet_size = 0;
	Int16* outputptr = NULL;
	const TSoundFileHeader* pSoundHeader = NULL;
	const TSpeexHeader* pSpeexHeader = NULL;
	const UInt8* DataPtr = NULL;
	Int16 output[MAX_FRAME_SIZE] = {0};
	Int16 i = 0;
	ESldError error = eOK;
	SpeexBits bits;
	void* st = NULL;

#ifdef _PALM_OS_DEFINE_
#include "speex_wb_mode"
#endif

	if (!aLayerAccess || !aBuilderPtr)
		return eMemoryNullPointer;
	
	if (!aDataPtr || !aDataSize)
		return eSoundNoDataForDecoding;

	// Общий заголовок звукового файла
	pSoundHeader = (const TSoundFileHeader*)aDataPtr;

	// Проверка корректности данных
	if (pSoundHeader->structSize != sizeof(*pSoundHeader))
		return eSoundSpeexDataCorrupted;

	if (pSoundHeader->SoundFormat != SLD_SOUND_FORMAT_SPX)
		return eSoundWrongDataFormat;

	// Специфический заголовок speex
	pSpeexHeader = (const TSpeexHeader*)(aDataPtr + pSoundHeader->structSize);

	// Проверка корректности данных
	if (pSpeexHeader->structSize != sizeof(*pSpeexHeader))
		return eSoundSpeexDataCorrupted;

	if (pSoundHeader->SoundDataSize != (aDataSize - pSoundHeader->structSize - pSpeexHeader->structSize))
		return eSoundSpeexDataCorrupted;

	// Указатель на собственно звуковые данные
	DataPtr = aDataPtr + pSoundHeader->structSize + pSpeexHeader->structSize;
	
	if (aStartPos)
		StartPos = *aStartPos;

	// Выделяем память для декодированных данных
	outputptr = (Int16*)sldMemNew(sizeof(Int16)*MAX_FRAME_SIZE*SS_OUT_DATA_INTERLEAVE);
	if (!outputptr)
		return eMemoryNotEnoughMemory;

	// Размер пакета декодированных данных
	if (pSpeexHeader->Version == SPEEX_FORMAT_VERSION_QUALITY || pSpeexHeader->Version == SPEEX_FORMAT_VERSION_OLD)
	{
		switch (pSpeexHeader->PacketSize)
		{
		case(10) : {packet_size = 106; break; }
		case(9) : {packet_size = 86; break; }
		case(8) : {packet_size = 70; break; }
		case(7) : {packet_size = 60; break; }
		case(6) : {packet_size = 52; break; }
		case(5) : {packet_size = 43; break; }
		case(4) : {packet_size = 33; break; }
		case(3) : {packet_size = 25; break; }
		case(2) : {packet_size = 20; break; }
		case(1) : {packet_size = 15; break; }
		default: return eSoundWrongSpeexPacketSize;
		}
	}
	else
	{
		packet_size = pSpeexHeader->PacketSize;
	}

	if (!(st = speex_decoder_init(&speex_wb_mode)))
		return eSoundErrorSpeexInit;
	i = 1;
	speex_decoder_ctl(st, SPEEX_GET_FRAME_SIZE, &frame_size);

	speex_bits_init(&bits);

	if (pSoundHeader->SoundDataSize%packet_size)
		return eSoundSpeexDataCorrupted;
	packet_count = (UInt16)(pSoundHeader->SoundDataSize/packet_size);

	// Начинаем декодирование
	UInt8 OutputSoundFormat = (UInt8)SLD_SOUND_FORMAT_WAV;
	if (!StartPos) {
		error = (aLayerAccess->*aBuilderPtr)(&OutputSoundFormat, 0, StartPos, pSoundHeader->SampleRate, SLD_SOUND_FLAG_START);
		if (error != eOK)
		{
			speex_decoder_destroy(st);
			speex_bits_destroy(&bits);
			sldMemFree(outputptr);
			return error;
		}
	}
	// Размер одного декодированного фрейма данных
	fsize = sizeof(Int16)*frame_size;
	
	Int16 beginIndex = (aBeginPlay * packet_count) / 100;
	// Декодируем все блоки данных
	for (i=beginIndex;i<packet_count;i++)
	{
		// @todo : Количество памяти выделенной под буфер отличается от того сколько на самом деле используется. Используется всегда в 2 раза меньше. Надо проверить.
		speex_bits_read_from(&bits, (char*)((UInt4Ptr)DataPtr+packet_size*i), packet_size);
		speex_decode(st, &bits, output);

		shift = (i%SS_OUT_DATA_INTERLEAVE);
		sldMemMove(((UInt8*)outputptr)+shift*fsize, output, fsize);
		
		if (((i+1)%SS_OUT_DATA_INTERLEAVE)==0)
		{
			error = (aLayerAccess->*aBuilderPtr)((UInt8*)outputptr, SS_OUT_DATA_INTERLEAVE*fsize, StartPos, 
				pSoundHeader->SampleRate, SLD_SOUND_FLAG_CONTINUE);
				
			StartPos += SS_OUT_DATA_INTERLEAVE*fsize;
			
			if (error != eOK)
			{
				speex_decoder_destroy(st);
				speex_bits_destroy(&bits);
				sldMemFree(outputptr);
				return error;
			}
		}
	}
	
	// Передаем остатки данных, если есть
	shift = (i%SS_OUT_DATA_INTERLEAVE);
	if (shift)
	{
		error = (aLayerAccess->*aBuilderPtr)((UInt8*)outputptr, shift*fsize, StartPos, 
			pSoundHeader->SampleRate, SLD_SOUND_FLAG_CONTINUE);
			
		StartPos += shift*fsize;
		
		if (error != eOK)
		{
			speex_decoder_destroy(st);
			speex_bits_destroy(&bits);
			sldMemFree(outputptr);
			return error;
		}
	}
		
	// Завершаем декодирование
	error = (aLayerAccess->*aBuilderPtr)(NULL, 0, StartPos, 
		pSoundHeader->SampleRate, aIsLast?SLD_SOUND_FLAG_FINISH:SLD_SOUND_FLAG_CONTINUE);
	
	if (error != eOK)
	{
		speex_decoder_destroy(st);
		speex_bits_destroy(&bits);
		sldMemFree(outputptr);
		return error;
	}

	if (aStartPos)
		*aStartPos = StartPos;

	speex_decoder_destroy(st);
	speex_bits_destroy(&bits);
	sldMemFree(outputptr);

	return eOK;
}


/// Конструктор
SldSpxDecoder::SldSpxDecoder(void) : m_Buffer(NULL), m_DataPtr(NULL), m_St(NULL)
{
	sldMemZero(&m_Bits, sizeof(SpeexBits));
}

/// Деструктор
SldSpxDecoder::~SldSpxDecoder(void)
{
	if(m_Buffer != NULL)
		sldMemFree(m_Buffer);
	if(m_St != NULL)
	{
		speex_decoder_destroy(m_St);
		speex_bits_destroy(&m_Bits);
	}
}

/***********************************************************************
* Инициализация
*
* @param aRes		- Ресурс с озвучкой
*
* @return код ошибки
************************************************************************/
ESldError SldSpxDecoder::Init(const CSDCReadMy::Resource &aRes)
{
	if (aRes.empty())
		return eMemoryNullPointer;

	ESldError error = Init(aRes.ptr(), aRes.size());
	if (error == eOK)
		m_Res = aRes;
	return error;
}

/***********************************************************************
* Инициализация
*
* @param aData		- Указатель на данные с озвучкой
* @param aSize		- Размер данных озвучки
*
* @return код ошибки
************************************************************************/
ESldError SldSpxDecoder::Init(const UInt8 *aData, UInt32 aSize)
{
	if (!aData || !aSize)
		return eSoundNoDataForDecoding;

	// Общий заголовок звукового файла
	const TSoundFileHeader* pSoundHeader = (const TSoundFileHeader*)aData;

	// Проверка корректности данных
	if (pSoundHeader->structSize != sizeof(*pSoundHeader))
		return eSoundSpeexDataCorrupted;

	if (pSoundHeader->SoundFormat != SLD_SOUND_FORMAT_SPX)
		return eSoundWrongDataFormat;

	// Специфический заголовок speex
	const TSpeexHeader* pSpeexHeader = (const TSpeexHeader*)(aData + pSoundHeader->structSize);

	// Проверка корректности данных
	if (pSpeexHeader->structSize != sizeof(*pSpeexHeader))
		return eSoundSpeexDataCorrupted;

	if (pSoundHeader->SoundDataSize != (aSize - pSoundHeader->structSize - pSpeexHeader->structSize))
		return eSoundSpeexDataCorrupted;

	// Указатель на собственно звуковые данные
	m_DataPtr = aData + pSoundHeader->structSize + pSpeexHeader->structSize;

	// Размер пакета декодированных данных
	if (pSpeexHeader->Version == SPEEX_FORMAT_VERSION_QUALITY || pSpeexHeader->Version == SPEEX_FORMAT_VERSION_OLD)
	{
		switch (pSpeexHeader->PacketSize)
		{
		case(10) : {m_PacketSize = 106; break; }
		case(9) : {m_PacketSize = 86; break; }
		case(8) : {m_PacketSize = 70; break; }
		case(7) : {m_PacketSize = 60; break; }
		case(6) : {m_PacketSize = 52; break; }
		case(5) : {m_PacketSize = 43; break; }
		case(4) : {m_PacketSize = 33; break; }
		case(3) : {m_PacketSize = 25; break; }
		case(2) : {m_PacketSize = 20; break; }
		case(1) : {m_PacketSize = 15; break; }
		default: return eSoundWrongSpeexPacketSize;
		}
	}
	else
	{
		m_PacketSize = pSpeexHeader->PacketSize;
	}

	m_SampleRate = pSoundHeader->SampleRate;
	m_SoundDataSize = pSoundHeader->SoundDataSize;

	ESldError error = PrepareDecoder();
	if(error != eOK)
		return error;

	m_Initialized = 1;
	return eOK;
}

/// Сброс декодировщика
ESldError SldSpxDecoder::Reset(void)
{
	m_Res = CSDCReadMy::Resource();
	if(m_Buffer != NULL)
		sldMemFree(m_Buffer);
	m_Buffer = NULL;
	m_DataPtr = NULL;
	if(m_St != NULL)
	{
		speex_decoder_destroy(m_St);
		m_St = NULL;
		speex_bits_destroy(&m_Bits);
		sldMemZero(&m_Bits, sizeof(SpeexBits));
	}
	return eOK;
}

/// Подготовка декодировщика
ESldError SldSpxDecoder::PrepareDecoder(void)
{
	// Выделяем память для декодированных данных
	m_Buffer = (UInt8*)sldMemNew(sizeof(UInt8)*MAX_FRAME_SIZE*SS_OUT_DATA_INTERLEAVE);
	if (!m_Buffer)
		return eMemoryNotEnoughMemory;

	// Инициализация декодировщика
	speex_bits_init(&m_Bits);

	m_St = speex_decoder_init(&speex_wb_mode);
	if (!m_St)
		return eSoundErrorSpeexInit;

	speex_decoder_ctl(m_St, SPEEX_GET_FRAME_SIZE, &m_FrameSize);

	if (m_SoundDataSize%m_PacketSize)
		return eSoundSpeexDataCorrupted;
	m_PacketCount = (UInt16)(m_SoundDataSize/m_PacketSize);

	m_StartFrame = (m_StartPos * m_PacketCount) / 100;

	return eOK;
}

/***********************************************************************
* Установка позиции, с которой будет производится декодировка
*
* @param aPos		- Стартовая позиция (в %)
*
* @return
************************************************************************/
void SldSpxDecoder::SetPos(UInt32 aPos)
{ 
	m_StartPos = aPos; 
	m_StartFrame = (m_StartPos * m_PacketCount) / 100;
}

/***********************************************************************
* Установка позиции, с которой будет производится декодировка
*
* @param aBuf[out]	- указатель на буфер с декодированными данными
* @param aSize[out]	- размер буфера
*
* @return код ошибки
************************************************************************/
ESldError SldSpxDecoder::GetNextBuff(UInt8** aBuf, UInt32* aSize)
{
	// Размер одного декодированного фрейма данных
	UInt32 fsize = sizeof(UInt16)*m_FrameSize;
	Int16 output[MAX_FRAME_SIZE] = {0};
	UInt32 shift = 0;
	// Декодируем данные
	for (UInt32 i = m_StartFrame; i < m_PacketCount; i++)
	{
		speex_bits_read_from(&m_Bits, (char*)((UInt4Ptr)m_DataPtr+m_PacketSize*i), m_PacketSize);
		speex_decode(m_St, &m_Bits, output);

		shift = (i%SS_OUT_DATA_INTERLEAVE);
		sldMemMove(((UInt8*)m_Buffer)+shift*fsize, output, fsize);
		m_StartFrame++;
		if (((i+1)%SS_OUT_DATA_INTERLEAVE)==0)
		{
			*aBuf = m_Buffer;
			*aSize = SS_OUT_DATA_INTERLEAVE*fsize;
			return eOK;
		}
	}

	// Передаем остатки данных, если есть
	shift = (m_PacketCount%SS_OUT_DATA_INTERLEAVE);
	if (shift)
	{
		*aBuf = m_Buffer;
		*aSize = shift*fsize;
		return eSoundDecoderEnd;
	}

	return eSoundDecoderEnd;
}
