//
// Created by kovtunov on 14.02.19.
//

#include "SpellingSearch.h"
#include "java/java_objects.h"

jobject SpellingSearch::native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _word,
        jobject _maximumWords
) const noexcept
{
    Int32 listIndex = JavaObjects::GetInteger( env, _listIndex );
    JavaObjects::string word = JavaObjects::GetString( env, _word );
    Int32 maximumWords = JavaObjects::GetInteger( env, _maximumWords );
    jobject errorValue = JavaObjects::GetInteger( env, -1 );

    ESldError error;

    error = dictionary->DoSpellingSearch(listIndex,word.c_str(), maximumWords);
    if (eOK != error)
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
