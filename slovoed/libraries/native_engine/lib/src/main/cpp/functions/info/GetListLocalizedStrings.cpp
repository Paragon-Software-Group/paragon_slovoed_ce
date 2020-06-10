//
// Created by borbat on 11.09.2018.
//

#include "GetListLocalizedStrings.h"
#include "java/java_objects.h"
#include "java/java_callback.h"

typedef ESldError (CSldListLocalizedString::*fpointer)( UInt32, const UInt16** ) const;

static fpointer Functions[] = {&CSldListLocalizedString::GetListNameShort,
                               &CSldListLocalizedString::GetListName};

jobject GetListLocalizedStrings::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject listNumber,
    jobject callbacks
) const noexcept
{
  const CSldListLocalizedString* pStrings;
  ESldError error = dictionary->GetLocalizedStringsByListIndex( JavaObjects::GetInteger( env, listNumber ), &pStrings );
  if ( ( error == eOK ) && ( pStrings != nullptr ) )
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
          jobject callback = env->GetObjectArrayElement( (jobjectArray) callbacks, (jsize) j );
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
