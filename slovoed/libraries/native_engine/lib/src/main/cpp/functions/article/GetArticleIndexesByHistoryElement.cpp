//
// Created by mamedov on 02.11.2018.
//

#include "GetArticleIndexesByHistoryElement.h"
#include "java/java_objects.h"

jobject GetArticleIndexesByHistoryElement::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _historyElement
) const noexcept
{
  jbyteArray historyElement = static_cast<jbyteArray>(_historyElement);
  UInt32 size = env->GetArrayLength( historyElement );
  Int8* buf = new Int8[size];
  env->GetByteArrayRegion( historyElement, 0, size, buf );
  CSldHistoryElement* element = new CSldHistoryElement();
  element->LoadElement( buf, size );

  delete[] buf;
  ESldHistoryResult result = eHistoryNotCompare;
  Int32 ListIndex, GlobalIndex;
  ESldError e = dictionary->GetWordByHistoryElement( element, &result, &ListIndex, &GlobalIndex );
  delete element;

  UInt32 articleIndexes[2];
  articleIndexes[0] = ListIndex;
  articleIndexes[1] = GlobalIndex;
  jintArray res = env->NewIntArray( 2u );
  if ( e != eOK || ( result != eHistoryFullCompare && result != eHistoryDuplicateCompare ) )
  {
    articleIndexes[0] = articleIndexes[1] = -1;
  }
  env->SetIntArrayRegion( res, 0u, 2u, (jint*) articleIndexes );
  return res;
}