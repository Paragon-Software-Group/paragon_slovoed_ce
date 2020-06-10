//
// Created by kovtunov on 26.11.18.
//

#include "GetBaseForms.h"
#include <java/java_objects.h>

#define SUPER_BUF_SIZE 8*1024

jobject GetBaseForms::native(
        JNIEnv* env,
        CDictionaryContext* dictionaryContext,
        jobject _direction,
        jobject _searchQuery
) const noexcept
{

    bool  foundForm = false;

    UInt16 buf[SUPER_BUF_SIZE];
    UInt16 word[MAX_WORD_LEN];
    UInt16 buf2[SUPER_BUF_SIZE];
    UInt16 variant[SUPER_BUF_SIZE];

    const MorphoInflectionRulesSetHandle* ruleset;
    WordFormsIterator formsIter;

    UInt32 direction = (UInt32)JavaObjects::GetInteger( env, _direction );

    MorphoData * morpho = dictionaryContext->GetExternalMorpho(direction,CDictionaryContext::eFts);
    if (!morpho) {
        return NULL;
    }

    JavaObjects::string searchQuery = JavaObjects::GetString(env,_searchQuery);
    if (searchQuery.length() == 0u)
    {
        return NULL;
    }

    const UInt16* wtmp= searchQuery.c_str();
    UInt32 length = (UInt32)searchQuery.length();

//    __android_log_print(ANDROID_LOG_DEBUG,"Slovoed", "Native. text length: %d",length);

    if (!wtmp) {
        return NULL;
    }

    //copy chars
    length = length > MAX_WORD_LEN - 1 ? MAX_WORD_LEN - 1 : length;
    memmove(word, wtmp,  length * sizeof(UInt16));
    word[length] = 0;

    UInt8 size = 0;

//    __android_log_print(ANDROID_LOG_DEBUG,"Slovoed", "Native. size calculated");

    jstring tmp;
    jclass alClass = env->FindClass("java/util/ArrayList");
    jmethodID alContructor = env->GetMethodID(alClass, "<init>", "()V");
    jmethodID setter = env->GetMethodID(alClass, "add", "(Ljava/lang/Object;)Z");
    jmethodID toArray = env->GetMethodID(alClass, "toArray", "([Ljava/lang/Object;)[Ljava/lang/Object;");

    jobject arrayList = env->NewObject(alClass, alContructor);
    env->DeleteLocalRef(alClass);

//    __android_log_print(ANDROID_LOG_DEBUG,"Slovoed", "Native. array created");

    WritingVersionIterator vIter;
    morpho->WritingVersionIteratorInitW(vIter,word,length);

    while( morpho->GetNextWritingVersionW(vIter,variant) )
    {
//		__android_log_print(ANDROID_LOG_DEBUG,"Slovoed", "Native. GetNextWritingVersion");
        // test reversal conversion
        BaseFormsIterator bIter;
        morpho->BaseFormsIteratorInitW(bIter, variant, M_IGNORE_BASEFORMS_OF_PARTS | M_BASEFORMS_SHOULD_MATCH_PRECONDITION | M_BASEFORMS_USE_DICTIONARY | M_BASEFORMS_ONLY_DICTIONARY_WORDS  /** / | M_DONT_DO_LANGSPEC_IF_BASEFORMS_FOUND /**/ );
        //
        while( morpho->GetNextBaseFormW(bIter, buf, &ruleset) )
        {
//			__android_log_print(ANDROID_LOG_DEBUG,"Slovoed", "Native. GetNextBaseFormW");
            size++;
            //base form
            tmp = env->NewString(buf, CSldCompare::StrLen(buf));
            env->CallBooleanMethod(arrayList, setter, tmp);

            //class name (noun, verb etc.)
//            morpho->GetFullClassNameByRulesetPtrW(ruleset, buf2);
//            tmp = env->NewString(buf2, CSldCompare::StrLen(buf2));
//            env->CallBooleanMethod(arrayList, setter, tmp);

            ///////////////////
            foundForm = true;
        }
    }
//	__android_log_print(ANDROID_LOG_DEBUG,"Slovoed", "Native. Count of morpho form %d",size);

    if ( !foundForm )
    {
        return NULL;
    }
    jclass classString = env->FindClass("java/lang/String");

    //Create double sized array if class names needed
//    jobjectArray res = env->NewObjectArray(size * 2, classString, NULL);
    jobjectArray res = env->NewObjectArray(size,  classString, NULL);


    env->DeleteLocalRef(classString);
    return (jobjectArray) env->CallObjectMethod(arrayList, toArray, res);
}
