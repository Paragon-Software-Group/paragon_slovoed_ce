//
// Created by borbat on 20.09.2018.
//

#include "ResetList.h"
#include "java/java_objects.h"

jobject ResetList::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _listIndex,
    jobject _hierarchyPath,
    jobject _pathLength
) const noexcept
{
  Int32 listIndex = JavaObjects::GetInteger( env, _listIndex );
  jintArray hierarchyPath = (jintArray) _hierarchyPath;
  jint realArrayLength = env->GetArrayLength( hierarchyPath );
  jint neededPathLength = JavaObjects::GetInteger( env, _pathLength );
  jboolean res = JNI_FALSE;

  if ( ( neededPathLength >= 0 ) && ( neededPathLength <= realArrayLength ) )
  {
    ESldError error = dictionary->SetBase( listIndex, MAX_UINT_VALUE );
    if ( error == eOK )
    {
      if ( neededPathLength > 0 )
      {
        jint* buffer = (jint*) env->GetPrimitiveArrayCritical( hierarchyPath, nullptr );
        for ( jint i = 0 ; i < neededPathLength ; i++ )
        {
          error = dictionary->SetBase( listIndex, buffer[i] );
          if ( error != eOK )
          {
            break;
          }
        }
        env->ReleasePrimitiveArrayCritical( hierarchyPath, buffer, JNI_ABORT );
      }
    }
    res = (unsigned char) (char) ( error == eOK );
  }

  return JavaObjects::GetBoolean( env, res );
}
