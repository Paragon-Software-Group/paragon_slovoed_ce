package com.paragon_software.engine.rx.searchalldictionaries;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.destructionqueue.ManagedResource;
import com.paragon_software.engine.destructionqueue.ResourceHolder;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.native_engine.SearchAllResult;
import com.paragon_software.utils_slovoed.collections.CachedCollectionView;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.lang.ref.WeakReference;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

class ResultContainer implements SearchAllResult
{
  @Nullable
  private ResourceHolder< ObserverImpl > observerHolder = null;

  @NonNull
  private final CachedCollectionView<ArticleItem, Void> articleCollection = new CachedCollectionView<>();

  @NonNull
  private final DictionaryCollection dictionaryCollection = new DictionaryCollection();

  public void reset()
  {
    articleCollection.resetCache();
    dictionaryCollection.resetCache();
  }

  Observer< Task< Result > > getObserver()
  {
    if ( observerHolder == null )
    {
      observerHolder = ResourceHolder.create(this, new ObserverImpl(this));
    }
    return observerHolder.getResource();
  }

  void toggleSearch( boolean searchInProgress )
  {
    articleCollection.toggleProgress(searchInProgress);
    dictionaryCollection.toggleProgress(searchInProgress);
  }

  @Override
  public CollectionView< ArticleItem, Void > getArticleItemList()
  {
    return articleCollection;
  }

  @Override
  public CollectionView< Dictionary.DictionaryId, Void > getDictionaryIdList()
  {
    return dictionaryCollection;
  }

  private class DictionaryCollection extends CachedCollectionView< Dictionary.DictionaryId, Void >
  {
    @NonNull
    private final Set< Dictionary.DictionaryId > set = new HashSet<>();

    @Override
    public void resetCache()
    {
      super.resetCache();
      set.clear();
    }

    void addDictionary( @NonNull Dictionary.DictionaryId dictionaryId )
    {
      if ( !set.contains(dictionaryId) )
      {
        cacheItems(Collections.singletonList(dictionaryId));
        set.add(dictionaryId);
      }
    }
  }

  private static class ObserverImpl implements Observer< Task< Result > >, ManagedResource
  {
    @NonNull
    private final WeakReference< ResultContainer > weakParent;

    private Disposable disposable;

    @Nullable
    private Task< Result > lastTask = null;

    ObserverImpl( @NonNull ResultContainer parent )
    {
      weakParent = new WeakReference<>(parent);
    }

    @Override
    public void onSubscribe( Disposable d )
    {
      disposable = d;
    }

    @Override
    public void onNext( Task< Result > resultTask )
    {
      ResultContainer parent = weakParent.get();
      if ( parent != null )
      {
        //        Log.d("SLOVOED", "on next: " + Thread.currentThread().getName());
        if ( !resultTask.isRelatedTo(lastTask) )
        {
          parent.dictionaryCollection.resetCache();
          parent.articleCollection.resetCache();
          lastTask = resultTask;
        }
        Result result = resultTask.get();
        Dictionary.DictionaryId dictionaryId = result.dictionaryId;
        List< ArticleItem > items = result.items;
        if ( ( dictionaryId != null ) && ( !items.isEmpty() ) )
        {
          parent.dictionaryCollection.addDictionary(dictionaryId);
        }
        parent.articleCollection.cacheItems(items);
        if ( result.isTerminator() )
        {
          parent.toggleSearch(false);
        }
      }
    }

    @Override
    public void onError( Throwable e )
    {

    }

    @Override
    public void onComplete()
    {

    }

    @Override
    public void free()
    {
      if ( disposable != null )
      {
        if ( !disposable.isDisposed() )
        {
          disposable.dispose();
        }
      }
    }
  }
}
