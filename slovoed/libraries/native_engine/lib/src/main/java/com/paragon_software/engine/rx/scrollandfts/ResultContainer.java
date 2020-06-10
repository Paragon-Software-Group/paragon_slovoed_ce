package com.paragon_software.engine.rx.scrollandfts;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.destructionqueue.ManagedResource;
import com.paragon_software.engine.destructionqueue.ResourceHolder;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.engine.rx.scrollandfts.params.AbstractParams;
import com.paragon_software.engine.rx.scrollandfts.params.SpecialSearchParam;
import com.paragon_software.engine.rx.scrollandfts.result.AbstractResult;
import com.paragon_software.engine.rx.scrollandfts.result.ScrollSearchResult;
import com.paragon_software.engine.rx.scrollandfts.result.SpecialSearchResult;
import com.paragon_software.engine.views.MorphoCollectionView;
import com.paragon_software.engine.views.RegularNativeCollectionView;
import com.paragon_software.engine.views.SpecialSearchNativeCollectionView;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.ScrollResult;
import com.paragon_software.utils_slovoed.collections.CachedCollectionView;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

class ResultContainer
{
  private ResourceHolder< ObserverImpl > holder = null;

  @NonNull
  private final RegularNativeCollectionView regularView = new RegularNativeCollectionView();

  @NonNull
  private final MorphoCollectionView morphoView = new MorphoCollectionView();

  @NonNull
  private final ScrollResult scrollResult = new ParagonSearchResult(regularView, morphoView);

  @NonNull
  private final CachedCollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction>
          specialSearchOuterView = new CachedCollectionView<>();

  @Nullable
  private NativeDictionary dictionary = null;

  public void reset()
  {
    regularView.close();
    morphoView.clear();
    specialSearchOuterView.resetCache();
  }

  Observer< Task< AbstractResult > > getObserver()
  {
    if ( holder == null )
    {
      holder = ResourceHolder.create(this, new ObserverImpl(this));
    }
    return holder.getResource();
  }

  void beforeSearch( @Nullable DictionaryLocation paramsLocation, @NonNull AbstractParams params )
  {
    morphoView.clear();
    toggleProgress(true);
    if ( ( paramsLocation != null ) && ( params.dictionaryId != null ) && ( dictionary != null ) )
    {
      clearSpecialSearchResults();
      if ( dictionary.getLocation().equals(paramsLocation) )
      {
        if ( params instanceof SpecialSearchParam)
        {
          regularView.close();
        }
      }
    }
  }

  @NonNull
  CollectionView< ArticleItem, Dictionary.Direction> getScrollCollectionView()
  {
    return regularView;
  }

  @NonNull
  ScrollResult getScrollResult()
  {
    return scrollResult;
  }

  @NonNull
  CollectionView< CollectionView< ArticleItem, GroupHeader >, Dictionary.Direction> getSpecialSearchCollectionView()
  {
    return specialSearchOuterView;
  }

  private void toggleProgress( boolean isInProgress )
  {
    regularView.toggleProgress(isInProgress);
    specialSearchOuterView.toggleProgress(isInProgress);
  }

  private void clearSpecialSearchResults()
  {
    int i, n = specialSearchOuterView.getCount();
    SpecialSearchNativeCollectionView[] inner = new SpecialSearchNativeCollectionView[n];
    for ( i = 0; i < n ; i++ )
    {
      CollectionView< ArticleItem, GroupHeader > view = specialSearchOuterView.getItem(i);
      if ( view instanceof SpecialSearchNativeCollectionView )
      {
        inner[i] = (SpecialSearchNativeCollectionView) view;
      }
    }
    specialSearchOuterView.resetCache();
    for ( SpecialSearchNativeCollectionView view : inner )
    {
      view.close();
    }
  }

  private void onGetResult( @NonNull AbstractResult result )
  {
    if ( result instanceof ScrollSearchResult )
    {
      setScroll((ScrollSearchResult) result);
    }
    else if ( result instanceof SpecialSearchResult )
    {
      setSpecialSearch((SpecialSearchResult) result);
    }
    dictionary = result.dictionary;
    toggleProgress(false);
  }

  private void setScroll( @NonNull ScrollSearchResult result )
  {
    if ( ( result.dictionaryId != null ) && ( result.dictionary != null ) )
    {
      regularView.updateMetadata(result.resultDirection);
      if ( ( !result.dictionary.getLocation().equals(getCurrentDictionaryLocation()) ) || ( result.listIndex
          != regularView.getListIndex() ) || ( !regularView.isOpen() ) )
      {
        regularView.open(result.dictionaryId, result.dictionary, result.listIndex);
      }
      if ( result.wordIndex >= 0 )
      {
        regularView.updatePosition(result.wordIndex);
      }
      if (null != result.baseFormIndexList)
      {
        morphoView.setData(result.dictionaryId,result.dictionary,result.listIndex,result.baseFormIndexList);
      }
    }
    else
    {
      regularView.close();
    }
  }

  private void setSpecialSearch( @NonNull SpecialSearchResult result )
  {
    if ( ( result.dictionaryId != null ) && ( result.dictionary != null ) && result.listIndices.length > 0 )
    {
      specialSearchOuterView.updateMetadata(result.resultDirection);
      List< CollectionView< ArticleItem, GroupHeader > > inner = new ArrayList<>(result.listIndices.length);
      for ( int listIndex : result.listIndices )
      {
        SpecialSearchNativeCollectionView view = new SpecialSearchNativeCollectionView(result.word);
        view.open(result.dictionaryId, result.dictionary, listIndex);
        inner.add(view);
      }
      specialSearchOuterView.cacheItems(inner);
    }
  }

  @Nullable
  private DictionaryLocation getCurrentDictionaryLocation()
  {
    DictionaryLocation res = null;
    if ( dictionary != null )
    {
      res = dictionary.getLocation();
    }
    return res;
  }

  private static class ObserverImpl implements Observer< Task< AbstractResult > >, ManagedResource
  {
    @NonNull
    private final WeakReference< ResultContainer > weakParent;

    private Disposable disposable;

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
    public void onNext( Task< AbstractResult > task )
    {
      ResultContainer parent = weakParent.get();
      if ( ( parent != null ) && ( !task.isCanceled() ) )
      {
        parent.onGetResult(task.get());
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
  class ParagonSearchResult implements ScrollResult
  {

    private final RegularNativeCollectionView mArticleItemList;

    private final CollectionView<ArticleItem,Void> mMorphoArticleItemList;

    public ParagonSearchResult(RegularNativeCollectionView articleItemList,
                               CollectionView<ArticleItem, Void> morphoArticleItemList)
    {
      mArticleItemList = articleItemList;
      mMorphoArticleItemList = morphoArticleItemList;
    }

    @Override
    public CollectionView<ArticleItem, Dictionary.Direction> getArticleItemList() {
      return mArticleItemList;
    }

    @Override
    public CollectionView<ArticleItem, Void> getMorphoArticleItemList() {
      return mMorphoArticleItemList;
    }

    @Override
    public boolean startsWith( @NonNull String text )
    {
      return mArticleItemList.startsWith(text);
    }
  }
}
