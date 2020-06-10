//
// Created by mamedov on 14.02.2019.
//

#include "GetSoundIndexByWordGlobalIndex.h"
#include "java/java_objects.h"

jobject GetSoundIndexByWordGlobalIndex::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _listIndex,
    jobject _globalIndex
) const noexcept
{
  jint listIndex = JavaObjects::GetInteger( env, _listIndex );
  jint globalIndex = JavaObjects::GetInteger( env, _globalIndex );
  if ( dictionary->SetCurrentWordlist(listIndex) == eOK )
  {
    if ( dictionary->GetWordByGlobalIndex(globalIndex) == eOK )
    {
      CSldVector<Int32> indexes;
      if ( dictionary->GetCurrentWordSoundIndex(indexes) == eOK && indexes.size() )
      {
        return JavaObjects::GetInteger( env, indexes.front() );
      }
    }
  }
  return JavaObjects::GetInteger( env, SLD_INDEX_SOUND_NO );
}