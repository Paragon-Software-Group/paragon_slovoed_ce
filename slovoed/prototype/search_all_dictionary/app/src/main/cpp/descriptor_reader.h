//
// Created by Ivan M. Zhdanov on 18.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_DESCRIPTOR_READER_H
#define SEARCH_ALL_DICTIONARY_DESCRIPTOR_READER_H

#include <ISDCFile.h>
#include <unistd.h>
#include <sys/stat.h>

class DescriptorReader final : public ISDCFile
{
    int m_fd;
  public:
    constexpr DescriptorReader( int fd ) noexcept
        : m_fd( fd )
    {}

    Int8 IsOpened() const override
    {
      if ( -1 == m_fd )
      {
        return 0;
      }
      return 1;
    }

    UInt32 Read( void* aDestPtr, UInt32 aSize, UInt32 aOffset ) override
    {
      return static_cast<UInt32>(::pread( m_fd, aDestPtr, size_t( aSize ), off_t( aOffset ) ));
    }

    UInt32 GetSize() const override
    {
      struct stat st = {0};
      if ( 0 == ::fstat( m_fd, &st ) )
      {
        return static_cast<UInt32>(st.st_size);
      }
      return 0;
    }
};

class InputStreamReader final : public ISDCFile
{
    typedef JNIEnv* PJNIEnv;
    PJNIEnv& m_env;
    const JavaFunction& m_readerFunction;
    const jobject m_object;
    const jclass m_result;
    const jfieldID m_jData;
    const jfieldID m_jSize;

    template < class T >
    static T getGlobal( JNIEnv* const env, T object )noexcept
    {
      jobject result = env->NewGlobalRef( object );
      env->DeleteLocalRef( object );
      return static_cast<T>(result);
    }

  public:
    ~InputStreamReader() noexcept
    {
      m_env->DeleteGlobalRef( m_object );
    }

    InputStreamReader(
        PJNIEnv& env,
        const UInt32 size,
        const JavaFunction& readerFunction,
        jobject object
    ) noexcept
        : m_env( env ), m_readerFunction( readerFunction ), m_object( getGlobal( env, object ) ),
          m_result( getGlobal( env, env->FindClass( "com/paragon_software/engine/NativeFunctions$ReadResult" ) ) ),
          m_jData( env->GetFieldID( m_result, "data", "[B" ) ),
          m_jSize( env->GetFieldID( m_result, "size", "I" ) )
    {
    }

    Int8 IsOpened() const override
    {
      return 1;
    }

    UInt32 Read( void* aDestPtr, UInt32 aSize, UInt32 aOffset ) override
    {
      const jobject result = m_readerFunction.functionO( m_env, m_object, (jint) aSize, (jint) aOffset );
      if ( nullptr != result )
      {
        const jint size = m_env->GetIntField( result, m_jSize );
        const jobject array = m_env->GetObjectField( result, m_jData );
        m_env->GetByteArrayRegion( (jbyteArray) array, 0, size, (jbyte*) aDestPtr );
        m_env->DeleteLocalRef( result );
        return UInt32( size );
      }
      return 0;
    }

    UInt32 GetSize() const override
    {
      return 0;
    }
};

#endif //SEARCH_ALL_DICTIONARY_DESCRIPTOR_READER_H
