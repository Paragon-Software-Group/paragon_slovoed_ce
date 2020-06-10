package com.paragon_software.engine.rx.translatearticle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import android.util.SparseArray;
import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.*;
import com.paragon_software.engine.rx.DictionaryCache;
import com.paragon_software.engine.rx.EngineTaskRunner;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.native_engine.EngineArticleAPI;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import com.paragon_software.native_engine.HtmlBuilderParams;
import io.reactivex.functions.Consumer;
import io.reactivex.functions.Function;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

import static com.paragon_software.engine.nativewrapper.ArticleItemFactory.NORMAL_VARIANTS;

public final class TranslateArticle extends EngineTaskRunner< Params >
{
  @NonNull
  private final Subject< Task< Params > > source = PublishSubject.create();

  @NonNull
  private final Map< EngineArticleAPI.OnTranslationReadyCallback, Task< Params > > taskMap = new HashMap<>();

  private TranslateObserver observer = null;

  private DictionaryCache dictionaryCache;

  public void translate( @NonNull EngineArticleAPI.OnTranslationReadyCallback callback,
                         @Nullable ArticleItem articleItem, @NonNull HtmlBuilderParams htmlParams )
  {
    assertInitCalled();
    if ( dictionaryCache == null )
    {
      dictionaryCache = new DictionaryCache(applicationContext, 1);
    }
    if ( observer == null )
    {
      subscribe();
    }
    Dictionary paramsDictionary = null;
    if ( articleItem != null )
    {
      for ( Dictionary dictionary : dictionaryManager.getDictionaries() )
      {
        if ( articleItem.getDictId().equals(dictionary.getId()) )
        {
          paramsDictionary = dictionary;
          break;
        }
      }
    }
    emitNewTask(new Params(callback, articleItem, paramsDictionary, htmlParams));
  }

  @Override
  protected void emitTask( Task< Params > task )
  {
    source.onNext(task);
  }

  @Override
  protected Collection< Task< Params > > getCurrentTasks()
  {
    return taskMap.values();
  }

  private void subscribe()
  {
    RunOn< Task< Params > > switch1 = new RunOn<>(RunOn.What.Observe, RunOn.Where.Computation);
    RunOn< Task< Result > > switch2 = new RunOn<>(RunOn.What.Observe, RunOn.Where.Caller);
    observer = source.doOnNext(new SaveCurrentTask()).compose(switch1).map(new Translator()).compose(switch2)
                     .filter(new OnlyUncanceled()).doOnNext(new DiscardCurrentTask())
                     .subscribeWith(new TranslateObserver());
  }

  private final class SaveCurrentTask implements Consumer< Task< Params > >
  {
    @Override
    public void accept( Task< Params > newTask )
    {
      EngineArticleAPI.OnTranslationReadyCallback callback = newTask.get().callback;
      Task< Params > oldTask = taskMap.get(callback);
      if ( oldTask != null )
      {
        oldTask.cancel();
      }
      taskMap.put(callback, newTask);
    }
  }

  private final class Translator implements Function< Task< Params >, Task< Result > >
  {
    @Override
    public Task< Result > apply( Task< Params > paramsWithLocationTask )
    {
      Params params = paramsWithLocationTask.get();
      Result result = new Result(params.callback, params.articleItem, "", false, null);
      if ( !( paramsWithLocationTask.isCanceled() || ( params.articleItem == null ) || ( params.dictionary == null ) ) )
      {
        NativeDictionary dictionary = dictionaryCache.get(params.dictionary, true, true);
        if ( !( paramsWithLocationTask.isCanceled() || ( dictionary == null ) ) )
        {
          synchronized (dictionary) {
            //TODO: set correct hierarchy path
            ArticleSearcher articleSearcher = new ArticleSearcher(dictionary);
            ArticleItem updatedItem = updateArticleItem(params.articleItem, articleSearcher, dictionary);
            if (updatedItem != null) {
              String translation = dictionary.translate(updatedItem.getListId(), dictionary
                  .resetList(updatedItem.getListId(), updatedItem.getGlobalIndex()), params.htmlParams);
              result = new Result(params.callback, updatedItem, translation
                  , false, articleSearcher);
            }
          }
        }
      }
      return paramsWithLocationTask.spawn(result);
    }
  }

  private ArticleItem updateArticleItem(ArticleItem articleItem, ArticleSearcher articleSearcher, NativeDictionary dictionary) {

    if (articleItem.isSpecialAdditionalInteractive() && articleItem.getDirection() != null && (articleItem.getSortKey() != null)) {
      return articleSearcher.findPractisePronunciationArticleItem(
          articleItem.getDictId(),
          articleItem.getSortKey(),
          articleItem.getDirection().getLanguageFrom());
    } else {
      SparseArray<ListInfo> listInfo;
      if (articleItem.isAdditional()) {
        listInfo = dictionary.getLists(ListType.AdditionalInfo);
      } else if(articleItem.isMorphologyArticle()){
        listInfo = dictionary.getLists(ListType.MorphologyArticles);
      }else {
        listInfo = dictionary.getLists(ListType.Group.Main);
      }

      ListInfo info = listInfo.get(articleItem.getListId());
      int[] variants = info.getVariants(NORMAL_VARIANTS);
      String[] tmpBuffer = new String[NORMAL_VARIANTS.length];

      return ArticleItemFactory
          .createFromTranslate(
              articleItem.getDictId(),
              dictionary,
              articleItem.getListId(),
              articleItem.getGlobalIndex(),
              variants,
              tmpBuffer,
              articleItem.getLabel(),
              articleItem.getFtsHeadword(),
              articleItem.getFtsAnchor(),
              articleItem.getSearchQuery(),
              articleItem.getWordReferences());
    }
  }

  private final class DiscardCurrentTask implements Consumer< Task< Result > >
  {
    @Override
    public void accept( Task< Result > task )
    {
      taskMap.remove(task.get().callback);
    }
  }
}
