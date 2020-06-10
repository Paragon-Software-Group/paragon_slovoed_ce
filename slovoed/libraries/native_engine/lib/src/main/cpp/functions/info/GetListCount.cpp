//
// Created by borbat on 10.09.2018.
//

#include "GetListCount.h"
#include "java/java_objects.h"

jobject GetListCount::native( JNIEnv* env, CSldDictionary* dictionary ) const noexcept
{
  Int32 res;
  ESldError error = dictionary->GetNumberOfLists( &res );
  return JavaObjects::GetInteger( env, ( error == eOK ) ? res : (Int32) 0 );
}
