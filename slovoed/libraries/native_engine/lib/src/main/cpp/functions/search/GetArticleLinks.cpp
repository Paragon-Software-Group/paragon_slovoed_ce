//
// Created by popov on 06.04.2020.
//

#include "GetArticleLinks.h"
#include "java/java_objects.h"

jobjectArray GetArticleLinks::native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject listIndex,
        jobject wordIndex
) const noexcept
{
  jint listIdx = JavaObjects::GetInteger( env, listIndex );
  jint wordIdx = JavaObjects::GetInteger( env, wordIndex );

  SldLinksCollection aLinks;
  ESldError error = dictionary->GetArticleLinks(listIdx, wordIdx, aLinks);

  UInt32 wordReferencesCounter = ( error == eOK ) ? ( aLinks.size() ) : 0;

  jclass alClass = env->FindClass("java/util/ArrayList");
  jmethodID alContructor = env->GetMethodID(alClass, "<init>", "()V");
  jmethodID setter = env->GetMethodID(alClass, "add", "(Ljava/lang/Object;)Z");
  jmethodID toArray = env->GetMethodID(alClass, "toArray", "([Ljava/lang/Object;)[Ljava/lang/Object;");

  jobject arrayList = env->NewObject(alClass, alContructor);
  env->DeleteLocalRef(alClass);

  jclass LinkInfoClass = env->FindClass("com/paragon_software/article_manager/LinkInfo");

  jobjectArray res = env->NewObjectArray(wordReferencesCounter, LinkInfoClass, NULL);

  if (wordReferencesCounter > 0 )
  {
    for ( int i = 0 ; i < wordReferencesCounter ; ++i )
    {
      env->CallBooleanMethod(arrayList, setter, JavaLinkInfo::GetLinkInfo( env,
          aLinks[i].Indexes.ListIndex,
          aLinks[i].Indexes.WordIndex,
          JavaObjects::GetString(env, aLinks[i].DictId.c_str()),
          JavaObjects::GetString(env, aLinks[i].Key.c_str())));
    }
  }

  env->DeleteLocalRef(LinkInfoClass);
  return (jobjectArray) env->CallObjectMethod(arrayList, toArray, res);
}