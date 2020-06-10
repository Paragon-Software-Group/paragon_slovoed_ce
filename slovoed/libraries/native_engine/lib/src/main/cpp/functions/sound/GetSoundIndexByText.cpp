//
// Created by mamedov on 25.01.2019.
//

#include "GetSoundIndexByText.h"
#include "java/java_objects.h"

jobject GetSoundIndexByText::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _text
) const noexcept
{
  JavaObjects::string text = JavaObjects::GetString( env, _text );
  if ( text.length() > 0u )
  {
    UInt32 aResultFlag = 0;
    ESldError error = dictionary->GetMostSimilarWordByText( text.c_str(), &aResultFlag );
    CSldVector< Int32 > indexes;
    if ( error == eOK && aResultFlag == 1 && dictionary->GetCurrentWordSoundIndex( indexes ) == eOK && indexes.size() )
    {
      return JavaObjects::GetInteger( env, indexes.front() );
    }
  }
  return JavaObjects::GetInteger( env, SLD_INDEX_SOUND_NO );
}