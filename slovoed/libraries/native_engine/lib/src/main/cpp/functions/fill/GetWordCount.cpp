//
// Created by borbat on 11.09.2018.
//

#include "GetWordCount.h"
#include "java/java_objects.h"

jobject GetWordCount::native( JNIEnv* env, CSldDictionary* dictionary, jobject listIndex ) const noexcept
{
  Int32 res;
  ESldError error = dictionary->GetNumberOfWords( JavaObjects::GetInteger( env, listIndex ), &res );
  if ( error != eOK )
  {
    res = 0;
  }
  return JavaObjects::GetInteger( env, res );
}
