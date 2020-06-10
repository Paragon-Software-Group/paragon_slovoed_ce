package com.paragon_software.engine.rx.deserializearticle;

import android.content.Context;
import androidx.annotation.NonNull;

import java.io.Serializable;
import java.util.*;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.native_engine.PersistentArticle;
import io.reactivex.Observable;
import io.reactivex.ObservableSource;
import io.reactivex.functions.Function;

public class SplitByDictionaries implements Function< Task< Params >, ObservableSource< Task< Result > > >
{
  private final Context              applicationContext;
  private final DictionaryManagerAPI dictionaryManager;

  @NonNull
  private final RunOn< Task< Result > > dictionaryRunOn;

  SplitByDictionaries( Context context, DictionaryManagerAPI _dictionaryManager,
                       @NonNull RunOn< Task< Result > > _dictionaryRunOn )
  {
    applicationContext = context;
    dictionaryManager = _dictionaryManager;
    dictionaryRunOn = _dictionaryRunOn;
  }

  @Override
  public ObservableSource< Task< Result > > apply( Task< Params > task )
  {
    ObservableSource< Task< Result > > res;
    if ( !task.isCanceled() )
    {
      Params params = task.get();
      Map< Dictionary.DictionaryId, Map< Integer, Serializable > > serializableSplitMap = new HashMap<>();
      if ( params.serialized instanceof PersistentArticle[] )
      {
        serializableSplitMap = splitPersistentArticlesByDictionaries( (PersistentArticle[]) params.serialized );
      }
      List< Task< SingleDictionaryParams > > tasksList = new ArrayList<>(serializableSplitMap.size());
      for ( Map.Entry<Dictionary.DictionaryId, Map< Integer, Serializable > > entry : serializableSplitMap.entrySet() )
      {
        Map< Integer, Serializable > persistentArticlesMap = entry.getValue();
        tasksList.add(task.spawn(new SingleDictionaryParams(persistentArticlesMap, entry.getKey(), params.callback, params.openSoundBases)));
      }
      res = Observable.fromIterable(tasksList).flatMap(new DeserializeInDictionary(applicationContext, dictionaryManager, dictionaryRunOn))
                      .concatWith(Observable.just(task.spawn(Result.createTermantor(task.get().callback))));
    }
    else
    {
      res = Observable.empty();
    }
    return res;
  }

  private Map<Dictionary.DictionaryId, Map< Integer, Serializable > > splitPersistentArticlesByDictionaries( PersistentArticle[] persistentArticles )
  {
    Map< Dictionary.DictionaryId, Map< Integer, Serializable > > persistentArticlesMap = new HashMap<>();
    int index = 0;
    for ( PersistentArticle persistentArticle : persistentArticles )
    {
      Dictionary.DictionaryId dictId = new Dictionary.DictionaryId(persistentArticle.getDictId());
      if ( persistentArticlesMap.get(dictId) == null )
      {
        persistentArticlesMap.put(dictId, new TreeMap< Integer, Serializable >());
      }
      persistentArticlesMap.get(dictId).put(index, persistentArticle);
      index++;
    }
    return persistentArticlesMap;
  }
}
