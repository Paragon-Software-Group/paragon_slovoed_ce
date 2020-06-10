//
// Created by popov on 05.09.2019.
//

#include "java/java_objects.h"
#include "GetCurrentWordStylizedVariant.h"

jobject GetCurrentWordStylizedVariant::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _variantIndex
) const noexcept
{

  jint aVariantIndex  = JavaObjects::GetInteger( env, _variantIndex );
  if (!dictionary)
    return NULL;

  if(!dictionary->HasStylizedVariantForCurrentWord(aVariantIndex))
    return NULL;

  SplittedArticle split;
  if(dictionary->GetCurrentWordStylizedVariant(aVariantIndex, split) != eOK)
    return NULL;

  jobjectArray res = env->NewObjectArray(split.size() * 2, env->FindClass("java/lang/Object"), NULL);

  UInt32 i;
  for(i = 0; i < split.size(); i++)
  {
    if(!split[i].Text.empty())
    {
      jobject obj1 = env->NewString(split[i].Text.c_str(), split[i].Text.size());
      jobject obj2 = JavaObjects::GetInteger( env, split[i].StyleIndex );

      env->SetObjectArrayElement(res, i * 2, obj1);
      env->SetObjectArrayElement(res, i * 2 + 1, obj2);

      env->DeleteLocalRef(obj1);
      env->DeleteLocalRef(obj2);
    }
  }

  return res;
}
