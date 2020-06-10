//
// Created by mamedov on 01.11.2018.
//

#include "GetHistoryElementByGlobalIndex.h"
#include "java/java_objects.h"

jobject GetHistoryElementByGlobalIndex::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _listIndex,
    jobject _globalIndex
) const noexcept
{
  Int32 listIndex = JavaObjects::GetInteger( env, _listIndex );
  Int32 globalIndex = JavaObjects::GetInteger( env, _globalIndex );
  ESldError error1 = dictionary->SetCurrentWordlist( listIndex );
  if ( error1 == eOK )
  {
    CSldVector<CSldHistoryElement> elements;
    ESldError error2 = dictionary->GetHistoryElement( globalIndex, elements );
    if ( error2 == eOK & elements.size() )
    {
      CSldHistoryElement element = elements.front();
      Int8* data = NULL; UInt32 size = 0;
      element.GetBinaryData(&data, &size);
      jbyteArray raw = env->NewByteArray(size);
      env->SetByteArrayRegion(raw, 0, size, data);
      sldMemFree(data);
      return raw;
    }
  }
  return NULL;
}