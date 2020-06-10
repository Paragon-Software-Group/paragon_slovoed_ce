package com.paragon_software.engine.rx.deserializearticle;

import android.content.Context;
import androidx.annotation.Nullable;

import java.io.Serializable;
import java.util.*;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.native_engine.PersistentArticle;
import io.reactivex.ObservableEmitter;
import io.reactivex.ObservableOnSubscribe;


public class DeserializerSource implements ObservableOnSubscribe< Task< Result > >
{
  private final Context              applicationContext;
  private final DictionaryManagerAPI dictionaryManager;

  private final Task< SingleDictionaryParams > task;

  DeserializerSource( @Nullable Context _context, @Nullable DictionaryManagerAPI _dictionaryManager,
                      Task< SingleDictionaryParams > _task )
  {
    applicationContext = _context;
    dictionaryManager = _dictionaryManager;
    task = _task;
  }

  @Override
  public void subscribe( ObservableEmitter< Task< Result > > emitter )
  {
    if ( !task.isCanceled() )
    {
      SingleDictionaryParams params = task.get();
      if ( params.sortedSerializedMap.values().size() > 0 && dictionaryManager != null && applicationContext != null )
      {
        for ( Dictionary dictionary : dictionaryManager.getDictionaries() )
        {
          if ( dictionary.getId().equals(params.dictId) )
          {
            Map< Integer, ArticleItem > sortedDeserializedArticlesMap = deserializeForDictionary(dictionary, params.sortedSerializedMap, params.openSoundBases);
            Result result = new Result(params.callback, sortedDeserializedArticlesMap);
            emitter.onNext(task.spawn(result));
            break;
          }
        }
      }
    }
    emitter.onComplete();
  }

  private Map< Integer, ArticleItem > deserializeForDictionary( Dictionary dictionary, Map< Integer, Serializable > sortedPersistentArticlesMap, boolean openSoundBases )
  {
    Map< Integer, ArticleItem > sortedDeserializedArticlesMap = new TreeMap<>();
    NativeDictionary nativeDictionary = NativeDictionary.open(applicationContext, dictionary, true, openSoundBases);
    if ( nativeDictionary != null )
    {
      for ( Map.Entry< Integer, Serializable > entry : sortedPersistentArticlesMap.entrySet() )
      {
        if ( entry.getValue() instanceof PersistentArticle && dictionaryManager != null )
        {
          ArticleItem res = DeserializerUtils
              .deserializeFromPersistentArticleForDictionary((PersistentArticle) entry.getValue(), dictionary,
                                                             nativeDictionary);
          if ( res != null )
          {
            sortedDeserializedArticlesMap.put(entry.getKey(), res);
          }
        }
      }
      nativeDictionary.close();
    }
    return sortedDeserializedArticlesMap;
  }
}