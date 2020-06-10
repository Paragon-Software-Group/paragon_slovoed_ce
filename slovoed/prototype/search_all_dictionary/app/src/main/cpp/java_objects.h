//
// Created by Ivan M. Zhdanov on 17.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_JAVAOBJECTS_H
#define SEARCH_ALL_DICTIONARY_JAVAOBJECTS_H

#include <jni.h>
#include <functional>

class JavaObject
{
  protected:
    JNIEnv* m_env = nullptr;
    jobject m_object = nullptr;

  public:
    ~JavaObject() noexcept
    {
      if ( nullptr != m_env )
      {
        clean( m_env );
        m_env->DeleteLocalRef( m_object );
      }
    }

    constexpr JavaObject()noexcept = default;

    constexpr JavaObject( JNIEnv* env, jobject object ) noexcept
        : m_env( env ), m_object( object )
    {}

    JavaObject( JavaObject&& other )noexcept
        : m_env( other.m_env ), m_object( other.m_object )
    {
      other.m_env = nullptr;
    }

    virtual JavaObject& operator=( JavaObject&& other )noexcept
    {
      if ( this != &other )
      {
        m_env = other.m_env;
        m_object = other.m_object;
        other.m_env = nullptr;
      }
      return *this;
    }

    operator jobject() const noexcept
    {
      return m_object;
    }

    jobject release() noexcept
    {
      m_env = nullptr;
      return m_object;
    }

    operator bool() const noexcept
    {
      return nullptr != m_env && nullptr != m_object;
    }

  protected:
    virtual void clean( JNIEnv* env ) noexcept
    {}
};

class JavaString : public JavaObject
{
    const char* m_data = nullptr;
    jsize m_dataSize = 0;
    jsize m_length = 0;
  private:
    friend class JavaObjectFactory;

    JavaString()noexcept = default;

    constexpr JavaString( JNIEnv* env, jobject object, const char* data, jsize dataSize, jsize length ) noexcept
        : JavaObject( nullptr, nullptr ), m_data( data ), m_dataSize( dataSize ), m_length( length )
    {}

  public:
    const char* data() const noexcept
    {
      return m_data;
    }

    size_t data_size() const noexcept
    {
      if ( m_dataSize < 0 )
      {
        return 0;
      }
      return static_cast<size_t>(m_dataSize);
    }

    size_t size() const noexcept
    {
      if ( m_length < 0 )
      {
        return 0;
      }
      return static_cast<size_t>(m_length);
    }
};

namespace std
{
template <>
struct hash< JavaString > : public unary_function< JavaString, size_t >
{
    size_t operator()( const JavaString& __val ) const noexcept
    {
      return __murmur2_or_cityhash< size_t >()( __val.data(), __val.data_size() );
    }
};
}

class JavaObjectFactory
{
    jclass m_jBooleanClass = nullptr;
    jmethodID m_jBooleanClassCtor = nullptr;
    jmethodID m_jBooleanClassGet = nullptr;

    jclass m_jIntegerClass = nullptr;
    jmethodID m_jIntegerClassCtor = nullptr;
    jmethodID m_jIntegerClassGet = nullptr;

    jclass m_jLongClass = nullptr;
    jmethodID m_jLongClassCtor = nullptr;
    jmethodID m_jLongClassGet = nullptr;

    jclass m_jStringClass = nullptr;
  public:

    constexpr JavaObjectFactory() noexcept
    {}

    void init( JNIEnv* env ) noexcept
    {
      {
        jclass jClass = env->FindClass( "java/lang/Boolean" );
        m_jBooleanClass = (jclass) env->NewGlobalRef( jClass );
        env->DeleteLocalRef( jClass );
        m_jBooleanClassCtor = env->GetStaticMethodID( m_jBooleanClass, "valueOf", "(Z)Ljava/lang/Boolean;" );
        m_jBooleanClassGet = env->GetMethodID( m_jBooleanClass, "booleanValue", "()Z" );
      }

      {
        jclass jClass = env->FindClass( "java/lang/Integer" );
        m_jIntegerClass = (jclass) env->NewGlobalRef( jClass );
        env->DeleteLocalRef( jClass );
        m_jIntegerClassCtor = env->GetStaticMethodID( m_jIntegerClass, "valueOf", "(I)Ljava/lang/Integer;" );
        m_jIntegerClassGet = env->GetMethodID( m_jIntegerClass, "intValue", "()I" );
      }

      {
        jclass jClass = env->FindClass( "java/lang/Long" );
        m_jLongClass = (jclass) env->NewGlobalRef( jClass );
        env->DeleteLocalRef( jClass );
        m_jLongClassCtor = env->GetStaticMethodID( m_jLongClass, "valueOf", "(J)Ljava/lang/Long;" );
        m_jLongClassGet = env->GetMethodID( m_jIntegerClass, "longValue", "()J" );
      }

      {
        jclass jClass = env->FindClass( "java/lang/String" );
        m_jStringClass = (jclass) env->NewGlobalRef( jClass );
        env->DeleteLocalRef( jClass );
      }
    }

    void clean( JNIEnv* env )noexcept
    {
      env->DeleteGlobalRef( m_jLongClass );
      env->DeleteGlobalRef( m_jIntegerClass );
      env->DeleteGlobalRef( m_jBooleanClass );
      env->DeleteGlobalRef( m_jStringClass );
    }

    bool IsInstanceOfBoolean( JNIEnv* env, jobject object ) const noexcept
    {
      return JNI_TRUE == env->IsInstanceOf( object, m_jBooleanClass );
    }

    bool IsInstanceOfInteger( JNIEnv* env, jobject object ) const noexcept
    {
      return JNI_TRUE == env->IsInstanceOf( object, m_jIntegerClass );
    }

    bool IsInstanceOfLong( JNIEnv* env, jobject object ) const noexcept
    {
      return JNI_TRUE == env->IsInstanceOf( object, m_jLongClass );
    }

    JavaObject&& getBoolean( JNIEnv* env, jboolean value ) const noexcept
    {
      return std::move( JavaObject( env, env->CallStaticObjectMethod( m_jBooleanClass, m_jBooleanClassCtor, value ) ) );
    }

    jboolean getBoolean( JNIEnv* env, jobject object ) const noexcept
    {
      if ( IsInstanceOfBoolean( env, object ) )
      {
        return env->CallBooleanMethod( object, m_jBooleanClassGet );
      }
      return JNI_FALSE;
    }

    JavaObject&& getInteger( JNIEnv* env, jint value ) const noexcept
    {
      return std::move( JavaObject( env, env->CallStaticObjectMethod( m_jIntegerClass, m_jIntegerClassCtor, value ) ) );
    }

    jint getInteger( JNIEnv* env, jobject object ) const noexcept
    {
      if ( IsInstanceOfInteger( env, object ) )
      {
        return env->CallIntMethod( object, m_jIntegerClassGet );
      }
      return -1;
    }

    JavaObject&& getLong( JNIEnv* env, jlong value ) const noexcept
    {
      return std::move( JavaObject( env, env->CallStaticObjectMethod( m_jLongClass, m_jLongClassCtor, value ) ) );
    }

    jlong getLong( JNIEnv* env, jobject object ) const noexcept
    {
      if ( IsInstanceOfLong( env, object ) )
      {
        return env->CallLongMethod( object, m_jLongClassGet );
      }
      return -1;
    }

    JavaString getString( JNIEnv* env, jobject object ) const noexcept
    {
      if ( JNI_TRUE == env->IsInstanceOf( object, m_jStringClass ) )
      {
        jboolean copy = JNI_FALSE;
        return std::move(
            JavaString{
                env,
                (jstring) object,
                env->GetStringUTFChars( (jstring) object, &copy ),
                env->GetStringLength( (jstring) object ),
                env->GetStringUTFLength( (jstring) object )
            }
        );
      }
      return std::move( JavaString{} );
    }

    JavaObject getString( JNIEnv* env, const UInt16* string ) const noexcept
    {
      UInt8 result[4096] = {0};
      CSldCompare::StrUTF16_2_UTF8( result, string );
      return std::move(JavaObject( env, env->NewStringUTF( reinterpret_cast<char*>(result) ) ));
    }
};

namespace
{
template < class TResult >
class FunctionWrapper
{

};

}
class JavaFunction final
{
    jclass m_functionClass = nullptr;
    jmethodID m_functionMethod = nullptr;

  public:
    JavaFunction() noexcept = default;

    void
    init( JNIEnv* env, const char* className, const char* methodName, const char* methodSig, bool isStatic ) noexcept
    {
      {
        jclass jClass = env->FindClass( className );
        if ( isStatic )
        {
          m_functionClass = (jclass) env->NewGlobalRef( jClass );
          m_functionMethod = env->GetStaticMethodID( m_functionClass, methodName, methodSig );
        }
        else
        {
          m_functionMethod = env->GetMethodID( m_functionClass, methodName, methodSig );
        }
        env->DeleteLocalRef( jClass );
      }
    }

    void clean( JNIEnv* env )noexcept
    {
      if ( nullptr != m_functionClass )
      {
        env->DeleteGlobalRef( m_functionClass );
      }
    }

    template < class ...Args >
    void function( JNIEnv* env, Args ... args ) const noexcept
    {
      env->CallStaticVoidMethod( m_functionClass, m_functionMethod, args... );
    }

    template < class ...Args >
    bool functionBl( JNIEnv* env, Args ... args ) const noexcept
    {
      return JNI_TRUE == env->CallStaticBooleanMethod( m_functionClass, m_functionMethod, args... );
    }

    template < class ...Args >
    jobject functionO( JNIEnv* env, Args ... args ) const noexcept
    {
      return env->CallStaticObjectMethod( m_functionClass, m_functionMethod, args... );
    }
};

#endif //SEARCH_ALL_DICTIONARY_JAVAOBJECTS_H
