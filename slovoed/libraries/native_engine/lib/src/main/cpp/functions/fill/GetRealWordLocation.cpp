//
// Created by borbat on 19.09.2018.
//

#include "GetRealWordLocation.h"
#include "java/java_objects.h"

jobject GetRealWordLocation::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _listIndex,
    jobject _localIndex,
    jobject _outputArray
) const noexcept
{
  Int32 listIndex = JavaObjects::GetInteger( env, _listIndex );
  Int32 localIndex = JavaObjects::GetInteger( env, _localIndex );
  jintArray outputArray = (jintArray) _outputArray;
  jboolean res;

  Int32 realListIdx = -1, realGlobalIdx = -1;
  ESldError error1 = dictionary->GetRealListIndex( listIndex, localIndex, &realListIdx );
  ESldError error2 = dictionary->GetRealGlobalIndex( listIndex, localIndex, &realGlobalIdx );

  if ( ( error1 == eOK ) && ( error2 == eOK ) && ( realListIdx >= 0 ) && ( realGlobalIdx >= 0 ) )
  {
    jint* buffer = (jint*) env->GetPrimitiveArrayCritical( outputArray, nullptr );
    buffer[0] = realListIdx;
    buffer[1] = realGlobalIdx;
    env->ReleasePrimitiveArrayCritical( outputArray, buffer, 0 );
    res = JNI_TRUE;
  }
  else
  {
    res = JNI_FALSE;
  }

  return JavaObjects::GetBoolean( env, res );
}
