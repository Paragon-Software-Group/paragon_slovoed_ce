//
// Created by Ivan M. Zhdanov on 18.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_DESCRIPTOR_READER_H
#define SEARCH_ALL_DICTIONARY_DESCRIPTOR_READER_H

#include <jni.h>
#include "../env_aware.h"

class DescriptorReader final : public EnvAwareSdcReader
{
    int m_fd;
    jlong m_offset, m_size;
  public:
    /*constexpr */DescriptorReader( int fd, jlong offset, jlong size ) noexcept
        : m_fd( fd ), m_offset( ( offset >= 0L ) ? offset : 0L ), m_size( size )
    {}

    Int8 IsOpened() const override;

    UInt32 Read( void* aDestPtr, UInt32 aSize, UInt32 aOffset ) override;

    UInt32 GetSize() const override;
};

#endif //SEARCH_ALL_DICTIONARY_DESCRIPTOR_READER_H
