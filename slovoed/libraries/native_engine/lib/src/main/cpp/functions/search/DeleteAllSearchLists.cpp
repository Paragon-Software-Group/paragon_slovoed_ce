//
// Created by borbat on 20.09.2018.
//

#include "DeleteAllSearchLists.h"
#include "java/java_objects.h"

jobject DeleteAllSearchLists::native( JNIEnv* env, CSldDictionary* dictionary ) const noexcept
{
  ESldError error = dictionary->ClearSearch();
  jboolean res = (unsigned char) (char) ( error == eOK );
  return JavaObjects::GetBoolean( env, res );
}
