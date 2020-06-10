//
// Created by mamedov on 31.01.2019.
//

#ifndef NATIVE_ENGINE_SOUNDBUILDER_H
#define NATIVE_ENGINE_SOUNDBUILDER_H

#include "../env_aware.h"


/// Начальный размер памяти, выделяемый для звуковых данных
#define SOUND_MEM_START_SIZE	1024*64
#define SOUND_MEM_INC_SIZE		1024*8

class SoundBuilder
{
  private:
    /// Размер памяти, выделенной для озвучки
    UInt32 m_SoundMemSize;
    /// Указатель на данные озвучки
    UInt8* m_SoundData;
    /// Размер данных озвучки
    UInt32 m_SoundDataSize;
    /// Частота дискретизации данных озвучки
    UInt32 m_SoundFrequency;
    /// Формат раскодированной озвучки
    UInt8 m_SoundDecodedFormat;
  public:
    SoundBuilder();

    ~SoundBuilder();

    const UInt8* GetSoundData() const;

    const UInt32 GetSoundDataSize() const;

    const UInt32 GetSoundFrequency() const;

    const UInt8 GetSoundDecodedFormat() const;

    ESldError StartBuilding(const UInt8* aBlockPtr);

    ESldError ContinueBuilding(const UInt8* aBlockPtr, UInt32 aBlockSize);

    ESldError FinishBuilding(UInt32 aFrequency);

};

#endif //NATIVE_ENGINE_SOUNDBUILDER_H
