//
// Created by borbat on 19.09.2018.
//

#include "GetPathFromGlobalIndex.h"
#include "java/java_objects.h"

jobject GetPathFromGlobalIndex::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _listIndex,
    jobject _globalIndex
) const noexcept
{
  TCatalogPath path;
  ESldError error = dictionary->GetPathByGlobalIndex(
      JavaObjects::GetInteger( env, _listIndex ), JavaObjects::GetInteger( env, _globalIndex ), &path
  );

  jsize neededSize = ( error == eOK ) ? ( (jsize) (UInt32) path.BaseListCount ) : (jsize) 0;
  jintArray array = env->NewIntArray( neededSize );
  jint* buffer = (jint*) env->GetPrimitiveArrayCritical( array, nullptr );
  if ( error == eOK )
  {
    for ( jint i = 0 ; i < neededSize ; i++ )
    {
      buffer[i] = path.BaseList[i];
    }
  }
  env->ReleasePrimitiveArrayCritical( array, buffer, 0 );

  return array;
}
