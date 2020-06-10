//
// Created by mamedov on 04.09.2019.
//

#include "GetPreloadedFavorites.h"
#include "java/java_objects.h"

jobject GetPreloadedFavorites::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _listIndex,
    jobject _callback
) const noexcept
{
  Int32 listIndex = JavaObjects::GetInteger( env, _listIndex );
  dictionary->SetCurrentWordlist(listIndex);
  dictionary->SetBase(-1);
  ReadWordsFromHierarchyList(env, *dictionary, _callback);

  return JavaObjects::GetInteger( env, -1 );
}

ESldError GetPreloadedFavorites::ReadWordsFromHierarchyList(JNIEnv* env, CSldDictionary &dict, jobject callback) const
{
  Int32 numberOfWords = -1;
  dict.GetNumberOfWords(&numberOfWords);

  for (Int32 wordIndex = 0; wordIndex < numberOfWords; ++wordIndex)
  {
    dict.GetWordByIndex(wordIndex);

    UInt32 hasHierarchy = 0;
    dict.isWordHasHierarchy(wordIndex, &hasHierarchy);
    UInt16 *article = nullptr;
    if (hasHierarchy)
    {
      dict.GetCurrentWord(eVariantShow, &article);
      JavaPreloadedWordsCallback::AddNewChildDirectory( env, callback, JavaObjects::GetString(env, article) );
      dict.SetBase(wordIndex);
      ReadWordsFromHierarchyList(env, dict, callback);
      dict.GoToLevelUp();
      JavaPreloadedWordsCallback::NavigateToUpDirectory( env, callback );
    }
    else
    {
      Int32 translationCount = 0;
      dict.GetRealIndexesCount(wordIndex, &translationCount);

      if (translationCount)
      {
        Int32 globalListIndex = SLD_DEFAULT_LIST_INDEX;
        Int32 wordGlobalIndex = SLD_DEFAULT_WORD_INDEX;
        for (Int32 translationIndex = 0; translationIndex < translationCount; translationIndex++)
        {
          dict.GetRealIndexes(wordIndex, translationIndex, &globalListIndex, &wordGlobalIndex);
        }

        Int32 globalIndex = -1;
        dict.GetWordByGlobalIndex(globalListIndex, wordGlobalIndex, 1, &article);
        dict.GetCurrentGlobalIndex(globalListIndex, &globalIndex);
        JavaPreloadedWordsCallback::AddArticleItem( env, callback, globalListIndex, globalIndex );
      }
    }
  }
  return eOK;
}