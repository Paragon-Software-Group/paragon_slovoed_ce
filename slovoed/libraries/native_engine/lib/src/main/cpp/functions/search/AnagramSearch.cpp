#include <SldAuxiliary.h>
#include "AnagramSearch.h"
#include "java/java_objects.h"

jobject AnagramSearch::native(
    JNIEnv* env,
    CDictionaryContext* context,
    jobject _listIndex,
    jobject _word,
    jobject _maximumWords
) const noexcept
{
  Int32 listIndex = JavaObjects::GetInteger( env, _listIndex );
  JavaObjects::string word = JavaObjects::GetString( env, _word );
  Int32 maximumWords = JavaObjects::GetInteger( env, _maximumWords );
  jobject errorValue = JavaObjects::GetInteger( env, -1 );

  CSldDictionary* dictionary = context->GetDictionary();

  ESldError error = dictionary->DoAnagramSearch(listIndex, word.c_str(), maximumWords);
  if ( error != eOK )
  {
    return errorValue;
  }

  Int32 currentList;
  error = dictionary->GetCurrentWordList( &currentList );
  if ( error != eOK )
  {
    return errorValue;
  }

  return JavaObjects::GetInteger( env, currentList );
}
