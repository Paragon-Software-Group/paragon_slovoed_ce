package com.paragon_software.engine.rx.deserializearticle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.lang.ref.WeakReference;
import java.util.*;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.engine.destructionqueue.ManagedResource;
import com.paragon_software.engine.destructionqueue.ResourceHolder;
import com.paragon_software.engine.rx.Task;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;


public class ResultContainer
{
  @Nullable
  private ResourceHolder< ObserverImpl > observerHolder = null;

  private Map< Integer, ArticleItem > sortedArticleItemsMap = new TreeMap<>();

  Observer< Task< Result > > getObserver()
  {
    if ( observerHolder == null )
    {
      observerHolder = ResourceHolder.create(this, new ObserverImpl(this));
    }
    return observerHolder.getResource();
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
    public void onSubscribe( Disposable _disposable )
    {
      disposable = _disposable;
    }

    @Override
    public void onNext( Task< Result > resultTask )
    {
      ResultContainer parent = weakParent.get();
      if ( parent != null )
      {
        if ( !resultTask.isRelatedTo(lastTask) )
        {
          parent.sortedArticleItemsMap.clear();
          lastTask = resultTask;
        }
        Result result = resultTask.get();
        if ( result.isTerminator() )
        {
          lastTask.get().callback.onDeserializationStatusChanged(false);
        }
        else
        {
          lastTask.get().callback.onDeserializationStatusChanged(true);
          parent.sortedArticleItemsMap.putAll(result.sortedItemsMap);
          lastTask.get().callback.onDeserializedArticlesReady(parent.sortedArticleItemsMap.values().toArray(new ArticleItem[0]));
        }
      }
    }

    @Override
    public void onError( Throwable exception )
    {
      if ( lastTask != null )
      {
        lastTask.get().callback.onDeserializationStatusChanged(false);
      }
    }

    @Override
    public void onComplete()
    {
      if ( lastTask != null )
      {
        lastTask.get().callback.onDeserializationStatusChanged(false);
      }
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
