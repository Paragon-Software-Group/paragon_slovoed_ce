//
// Created by borbat on 10.09.2018.
//

#include "GetDictionaryLocalizedStrings.h"
#include "java/java_objects.h"
#include "java/java_callback.h"

typedef ESldError (CSldLocalizedString::*fpointer)( UInt32, const UInt16** ) const;

static fpointer Functions[] = {&CSldLocalizedString::GetDictionaryNameShort,
                               &CSldLocalizedString::GetDictionaryName,
                               &CSldLocalizedString::GetProductName,
                               &CSldLocalizedString::GetAuthorWeb};

jobject
GetDictionaryLocalizedStrings::native_a(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobjectArray callbacks
) const noexcept
{
  const CSldLocalizedString* pStrings;
  ESldError error = dictionary->GetLocalizedStrings( &pStrings );
  if ( error == eOK )
  {
    UInt32 i, n = pStrings->GetNumberOfLanguages();
    for ( i = 0u ; i < n ; i++ )
    {
      UInt32 code;
      error = pStrings->GetLanguagesCode( i, &code );
      if ( error == eOK )
      {
        jobject javaInt = JavaObjects::GetInteger( env, (Int32) code );
        size_t j, m = sizeof( Functions ) / sizeof( Functions[0] );
        for ( j = 0u ; j < m ; j++ )
        {
          const UInt16* string;
          error = ( pStrings->*Functions[j] )( code, &string );
          jobject callback = env->GetObjectArrayElement( callbacks, (jsize) j );
          if ( ( error == eOK ) && ( callback != nullptr ) )
          {
            jstring javaString = JavaObjects::GetString( env, string );
            JavaCallback::Call( env, callback, {javaInt, javaString} );
            env->DeleteLocalRef( javaString );
            env->DeleteLocalRef( callback );
          }
        }
        env->DeleteLocalRef( javaInt );
      }
    }
  }
  return JavaObjects::GetInteger( env, 0 );
}
