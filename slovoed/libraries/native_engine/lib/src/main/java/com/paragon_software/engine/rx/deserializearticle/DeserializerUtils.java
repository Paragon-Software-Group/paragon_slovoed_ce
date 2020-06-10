package com.paragon_software.engine.rx.deserializearticle;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.engine.nativewrapper.ArticleItemFactory;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.native_engine.PersistentArticle;

public final class DeserializerUtils
{

  private DeserializerUtils() { }

  public static @Nullable ArticleItem deserializeFromPersistentArticle( @NonNull PersistentArticle persistentArticle,
                                                              @NonNull DictionaryManagerAPI dictionaryManager,
                                                              @NonNull Context applicationContext )
  {
    ArticleItem res = null;
    Dictionary.DictionaryId dictId = new Dictionary.DictionaryId(persistentArticle.getDictId());
    for ( Dictionary dictionary : dictionaryManager.getDictionaries() )
    {
      if ( dictionary.getId().equals(dictId) )
      {
        NativeDictionary nativeDictionary = NativeDictionary.open(applicationContext, dictionary.getDictionaryLocation(),dictionary.getMorphoInfoList(),true);
        if ( nativeDictionary != null )
        {
          res = DeserializerUtils
              .deserializeFromPersistentArticleForDictionary(persistentArticle, dictionary, nativeDictionary);
          nativeDictionary.close();
        }
      }
    }
    return res;
  }

  public static @Nullable ArticleItem deserializeFromPersistentArticleForDictionary( @NonNull PersistentArticle persistentArticle,
                                                                           @NonNull Dictionary dictionary,
                                                                           @NonNull NativeDictionary nativeDictionary )
  {
    ArticleItem res = null;
    if ( persistentArticle.getHistoryElement() != null )
    {
      res = ArticleItemFactory
          .createNormalFromHistoryElement(dictionary.getId(), nativeDictionary, persistentArticle.getHistoryElement());
    }
    return res;
  }
}
