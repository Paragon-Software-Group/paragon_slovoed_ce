//
// Created by popov on 31.05.19.
//

#include <java/java_objects.h>
#include <SldDictionaryHelper.h>
#include "GetWordReference.h"

jobject GetWordReference::native(
        JNIEnv *env,
        CDictionaryContext* context,
        jobject _listIndex,
        jobject _phrase
) const noexcept {
    CSldDictionary* dictionary = context->GetDictionary();
    CSldDictionaryHelper helper = CSldDictionaryHelper(*dictionary);
    jint listIndex = JavaObjects::GetInteger( env, _listIndex );
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

    error = helper.GetWordReference(phrase.data(), morphos, wordRefs, isInternalMorphoAvailable);
    if (error != eOK)
        return nullptr;

    jintArray jData = env->NewIntArray(wordRefs.size()*2 );
        jint* data = (jint*) env->GetPrimitiveArrayCritical( jData, nullptr );
    for (auto wordRef = wordRefs.begin(); wordRef != wordRefs.end(); ++wordRef) {
                data[size++] = wordRef->StartPos;
                data[size++] = wordRef->EndPos;
   }
    env->ReleasePrimitiveArrayCritical( jData, data, 0 );

    return jData;
}
