//
// Created by mamedov on 31.01.2019.
//

#include "SoundBuilder.h"

SoundBuilder::SoundBuilder()
{
  m_SoundData = NULL;
  m_SoundDataSize = 0;
  m_SoundMemSize = 0;
  m_SoundDecodedFormat = 0;
  m_SoundFrequency = 0;
}

SoundBuilder::~SoundBuilder()
{
  if (m_SoundData)
  {
    sldMemFree(m_SoundData);
    m_SoundData = NULL;
  }
  m_SoundDataSize = 0;
  m_SoundMemSize = 0;
  m_SoundDecodedFormat = 0;
  m_SoundFrequency = 0;
}

const UInt8* SoundBuilder::GetSoundData() const
{
  return m_SoundData;
}

const UInt32 SoundBuilder::GetSoundDataSize() const
{
  return m_SoundDataSize;
}

const UInt32 SoundBuilder::GetSoundFrequency() const
{
  return m_SoundFrequency;
}

const UInt8 SoundBuilder::GetSoundDecodedFormat() const
{
  return m_SoundDecodedFormat;
}

ESldError SoundBuilder::StartBuilding(const UInt8* aBlockPtr)
{
  if (m_SoundData)
  {
    sldMemFree(m_SoundData);
    m_SoundData = NULL;
  }
  m_SoundDataSize = 0;
  m_SoundFrequency = 0;

  m_SoundMemSize = SOUND_MEM_START_SIZE;
  m_SoundData = (UInt8*)sldMemNew(m_SoundMemSize);
  if (!m_SoundData)
    return eMemoryNotEnoughMemory;

  // Здесь может передаваться тип формата, который получается при раскодировании озвучки
  // по умолчанию - wav
  if (aBlockPtr)
    m_SoundDecodedFormat = *aBlockPtr;
  else
    m_SoundDecodedFormat = (UInt8) SLD_SOUND_FORMAT_WAV;

  return eOK;
}

ESldError SoundBuilder::ContinueBuilding(const UInt8* aBlockPtr, UInt32 aBlockSize)
{
  if (!aBlockPtr)
    return eMemoryNullPointer;

  // Увеличиваем количество выделенной памяти если необходимо
  if (m_SoundDataSize + aBlockSize > m_SoundMemSize)
  {
    UInt32 delta = m_SoundDataSize + aBlockSize - m_SoundMemSize;
    UInt32 IncMemSize = (delta < SOUND_MEM_INC_SIZE) ? SOUND_MEM_INC_SIZE : delta;

    m_SoundMemSize += IncMemSize;
    UInt8* ptr = (UInt8*)sldMemNew(m_SoundMemSize);
    if (!ptr)
      return eMemoryNotEnoughMemory;

    sldMemMove(ptr, m_SoundData, m_SoundDataSize);
    sldMemFree(m_SoundData);
    m_SoundData = ptr;
  }

  sldMemMove(m_SoundData + m_SoundDataSize, aBlockPtr, aBlockSize);
  m_SoundDataSize += aBlockSize;

  return eOK;
}

ESldError SoundBuilder::FinishBuilding(UInt32 aFrequency)
{
  m_SoundFrequency = aFrequency;
  return eOK;
}