//
// Created by popov on 18.10.19.
//

#include <java/java_objects.h>
#include <SldDictionaryHelper.h>
#include "GetHighlightWordReferenceIncludingWhitespace.h"

jobject GetHighlightWordReferenceIncludingWhitespace::native(
        JNIEnv *env,
        CDictionaryContext* context,
        jobject _listIndex,
        jobject _phrase,
        jobject _searchQuery
) const noexcept {
    CSldDictionary* dictionary = context->GetDictionary();
    CSldDictionaryHelper helper = CSldDictionaryHelper(*dictionary);
    jint listIndex = JavaObjects::GetInteger( env, _listIndex );
    JavaObjects::string searchQuery = JavaObjects::GetString(env, _searchQuery);
    JavaObjects::string phrase = JavaObjects::GetString(env, _phrase);
    ESldLanguage languageFrom = dictionary->GetListLanguageFrom( listIndex );
    bool isInternalMorphoAvailable = context->IsInternalMorphoAvailable( languageFrom, CDictionaryContext::eFts );
    ESldError error;
    jint size = 0;

    CSldVector<TWordRefInfo> wordRefs;
    CSldVector<UInt32> aWords;
    CSldVector< MorphoDataType* > morphos;
    MorphoData* morphoData = nullptr;

    if ( !isInternalMorphoAvailable )
    {
        morphoData = context->GetExternalMorpho( languageFrom, CDictionaryContext::eFts );
    }


    if ( morphoData )
    {
        morphos.push_back( morphoData );
    }

    error = helper.GetWordReferenceInQuery( SldU16StringRef( phrase.c_str() ), morphos, aWords, SldU16StringRef( searchQuery.c_str() ), isInternalMorphoAvailable );

    if (error != eOK)
        return nullptr;

    error = helper.GetWordsPositionsInPhraseIncludingWhitespaces(SldU16StringRef(phrase.c_str()), aWords, wordRefs);

    if (error != eOK)
        return nullptr;

    jintArray jData = env->NewIntArray(wordRefs.size()*2 );
        jint* data = (jint*) env->GetPrimitiveArrayCritical( jData, nullptr );
    for (auto &wordRef : wordRefs) {
                data[size++] = wordRef.StartPos;
                data[size++] = wordRef.EndPos;
   }
    env->ReleasePrimitiveArrayCritical( jData, data, 0 );

    return jData;
}
