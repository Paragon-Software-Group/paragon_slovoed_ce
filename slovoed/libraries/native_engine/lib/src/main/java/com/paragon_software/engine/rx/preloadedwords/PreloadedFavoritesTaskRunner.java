package com.paragon_software.engine.rx.preloadedwords;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.SparseArray;

import java.util.Collection;
import java.util.Collections;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.ListInfo;
import com.paragon_software.engine.nativewrapper.ListType;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.rx.EngineTaskRunner;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.utils_slovoed.directory.Directory;
import io.reactivex.Single;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;
import io.reactivex.functions.Function;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.SingleSubject;
import io.reactivex.subjects.Subject;


public class PreloadedFavoritesTaskRunner extends EngineTaskRunner< Params >
{
  @NonNull
  private final Subject< Task< Params > > source = PublishSubject.create();
  @Nullable
  private Disposable sourceDisposable = null;
  @Nullable
  private Task< Params > currentTask;

  public Single< Directory< ArticleItem > > getPreloadedFavorites( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    assertInitCalled();
    if ( sourceDisposable == null )
    {
      subscribe();
    }
    SingleSubject< Directory< ArticleItem > > resultCallback = SingleSubject.create();
    emitNewTask(new Params(dictionaryId, resultCallback));
    return resultCallback;
  }

  private void subscribe()
  {
    RunOn< Task< Params > > switch1 = new RunOn<>(RunOn.What.Observe, RunOn.Where.Computation);
    RunOn< Task< Result > > switch2 = new RunOn<>(RunOn.What.Observe, RunOn.Where.Caller);
    sourceDisposable = source.doOnNext(new SaveCurrentTask()).compose(switch1).map(new GetPreloadedFavorites()).compose(switch2)
                             .filter(new OnlyUncanceled()).doOnNext(new DiscardCurrentTask())
                             .subscribe( resultTask -> {
                                           Directory< ArticleItem > res = resultTask.get().preloadedWords;
                                           if ( res != null )
                                           {
                                             resultTask.get().callbackSubject.onSuccess(res);
                                           }
                                           else
                                           {
                                             resultTask.get().callbackSubject.onError(new NullPointerException());
                                           }
                                         },
                                         throwable -> {
                                           if (currentTask != null)
                                             currentTask.get().callbackSubject.onError(throwable);
                                         });
  }

  @Override
  protected void emitTask( Task< Params > task )
  {
    source.onNext(task);
  }

  @Override
  protected Collection< Task< Params > > getCurrentTasks()
  {
    Collection< Task< Params > > res = Collections.emptySet();
    if ( currentTask != null )
    {
      res = Collections.singleton(currentTask);
    }
    return res;
  }

  private final class GetPreloadedFavorites implements Function< Task< Params >, Task< Result > >
  {
    @Override
    public Task< Result > apply( Task< Params > paramsTask )
    {
      Params params = paramsTask.get();
      Result result = new Result(null, params.callbackSubject);
      for ( Dictionary dictionary : dictionaryManager.getDictionaries() )
      {
        if ( dictionary.getId().equals(params.dictionaryId) )
        {
          NativeDictionary nativeDictionaryForPreloadedWords = NativeDictionary.open(applicationContext, dictionary.getDictionaryLocation(),
                                                                    dictionary.getMorphoInfoList(), true);
          NativeDictionary nativeDictionaryForArticleFactory = NativeDictionary.open(applicationContext, dictionary.getDictionaryLocation(),
                                                                                     dictionary.getMorphoInfoList(),true);
          if ( nativeDictionaryForPreloadedWords != null && nativeDictionaryForArticleFactory != null )
          {
            PreloadedWordsNativeCallback callback = new PreloadedWordsNativeCallback(dictionary, nativeDictionaryForArticleFactory);
            SparseArray< ListInfo > lists = nativeDictionaryForPreloadedWords.getLists(ListType.PreloadedFavourites);
            int listIndex = lists.size() > 0 ? lists.keyAt(0) : -1;
            if ( listIndex != -1 )
            {
              nativeDictionaryForPreloadedWords.getPreloadedFavorites(listIndex, callback);
            }
            Directory< ArticleItem > preloadedFavoritesDirectory = callback.getRootDirectory();
            nativeDictionaryForPreloadedWords.close();
            nativeDictionaryForArticleFactory.close();
            result = new Result(preloadedFavoritesDirectory, params.callbackSubject);
          }
        }
      }
      return paramsTask.spawn(result);
    }
  }

  private class SaveCurrentTask implements Consumer< Task< Params > >
  {
    @Override
    public void accept( Task< Params > paramsTask )
    {
      if ( currentTask != null )
      {
        currentTask.cancel();
      }
      currentTask = paramsTask;
    }
  }

  private final class DiscardCurrentTask implements Consumer< Task< Result > >
  {
    @Override
    public void accept( Task< Result > task )
    {
      currentTask = null;
    }
  }
}
