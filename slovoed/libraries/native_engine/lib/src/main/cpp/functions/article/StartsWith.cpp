//
// Created by mamedov on 18.10.2019.
//

#include <vector>
#include "StartsWith.h"
#include "java/java_objects.h"

jobject StartsWith::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _listIndex,
    jobject _globalIndex,
    jobject _text
) const noexcept
{
  jboolean res = JNI_FALSE;
  Int32 listIndex = JavaObjects::GetInteger( env, _listIndex );
  Int32 globalIndex = JavaObjects::GetInteger( env, _globalIndex );
  JavaObjects::string text = JavaObjects::GetString(env, _text);

  const CSldCompare *compare = nullptr;
  dictionary->GetCompare(&compare);

  const CSldListInfo *listInfo;
  dictionary->GetWordListInfo(listIndex, &listInfo);

  auto variantIndex = listInfo->GetVariantIndexByType(eVariantShow);

  if (variantIndex != -1)
  {
    UInt16* word = nullptr;
    dictionary->GetWordByGlobalIndex( listIndex, globalIndex, variantIndex, &word );

    SldU16String aEffectiveStringText;
    SldU16String aEffectiveStringWord;


    compare->GetEffectiveString( text.c_str(), aEffectiveStringText);
    compare->GetEffectiveString( word, aEffectiveStringWord);

    if ((CSldCompare::StrLen(aEffectiveStringText.c_str()) <= CSldCompare::StrLen(aEffectiveStringWord.c_str())))
    {
        auto textBufferLength = compare->StrLen( aEffectiveStringText.c_str() );
        auto wordEnd = aEffectiveStringWord.c_str() + textBufferLength + 1;
        std::vector< UInt16 > wordBuffer{aEffectiveStringWord.c_str(), wordEnd};
        wordBuffer.back() = 0;

        res = (unsigned char) (compare->StrICmp( aEffectiveStringText, wordBuffer.data() ) == 0);
    }
  }
  return JavaObjects::GetBoolean( env, res );
}