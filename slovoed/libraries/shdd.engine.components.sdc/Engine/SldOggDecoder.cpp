#include "SldOggDecoder.h"


/** *********************************************************************
* Декодирует озвучку в формате mp3
*
* @return код ошибки
************************************************************************/
ESldError OggDecode(ISldLayerAccess* aLayerAccess, FSoundBuilderMethodPtr aBuilderPtr, const UInt8* aDataPtr, UInt32 aDataSize, UInt8 aIsLast, UInt32* aStartPos)
{
    const TSoundFileHeader* pSoundHeader = NULL;
    const UInt8* DataPtr = NULL;
    UInt32 StartPos = 0;
    ESldError error = eOK;

    if (!aLayerAccess || !aBuilderPtr)
        return eMemoryNullPointer;

    if (!aDataPtr || !aDataSize)
        return eSoundNoDataForDecoding;

    // Общий заголовок звукового файла
    pSoundHeader = (const TSoundFileHeader*)aDataPtr;

    // Проверка корректности данных
    if (pSoundHeader->structSize != sizeof(*pSoundHeader))
        return eSoundSpeexDataCorrupted;

    if (pSoundHeader->SoundFormat != SLD_SOUND_FORMAT_OGG)
        return eSoundWrongDataFormat;

    if (pSoundHeader->SoundDataSize != (aDataSize - pSoundHeader->structSize))
        return eSoundSpeexDataCorrupted;

    // Указатель на собственно звуковые данные
    DataPtr = aDataPtr + pSoundHeader->structSize;

    if (aStartPos)
        StartPos = *aStartPos;

    // Начинаем декодирование
    UInt8 OutputSoundFormat = (UInt8)SLD_SOUND_FORMAT_OGG;
    error = (aLayerAccess->*aBuilderPtr)(&OutputSoundFormat, 0, StartPos, pSoundHeader->SampleRate, SLD_SOUND_FLAG_START);
    if (error != eOK)
        return error;

    // Передаем данные одним блоком
    error = (aLayerAccess->*aBuilderPtr)(DataPtr, pSoundHeader->SoundDataSize, StartPos,
        pSoundHeader->SampleRate, SLD_SOUND_FLAG_CONTINUE);

    if (error != eOK)
        return error;

    StartPos += pSoundHeader->SoundDataSize;

    // Завершаем декодирование
    error = (aLayerAccess->*aBuilderPtr)(NULL, 0, StartPos,
        pSoundHeader->SampleRate, aIsLast ? SLD_SOUND_FLAG_FINISH : SLD_SOUND_FLAG_CONTINUE);

    if (error != eOK)
        return error;

    if (aStartPos)
        *aStartPos = StartPos;

    return eOK;
}
