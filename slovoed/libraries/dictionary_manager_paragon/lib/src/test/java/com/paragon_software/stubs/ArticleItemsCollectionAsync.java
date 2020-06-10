package com.paragon_software.stubs;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

public abstract class ArticleItemsCollectionAsync implements CollectionView<ArticleItem, Void>
{
  private String[] ids;
  List< ArticleItem > articleItems = new LinkedList<>();

  public ArticleItemsCollectionAsync( String text )
  {
    this.ids = text.split(";");
    new Thread(new Runnable()
    {
      @Override
      public void run()
      {
        for ( String id : ids )
        {
          sleep(300);
          articleItems.add(0, new ArticleItem.Builder(new Dictionary.DictionaryId(id), 0, 0).build());
          callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, 0, 1);
        }

      }

      private void sleep( long millis )
      {
        try
        {
          Thread.sleep(millis);
        }
        catch ( InterruptedException e )
        {
          e.printStackTrace();
        }
      }
    }).start();
  }

  @NonNull
  private final Set< Notifier > notifiers = new HashSet<>();

  @Override
  public void registerListener( Notifier notifier )
  {
    if ( notifier != null )
    {
      notifiers.add(notifier);
    }
  }

  @Override
  public void unregisterListener( Notifier notifier )
  {
    if ( notifier != null )
    {
      notifiers.remove(notifier);
    }
  }

  public void callOnItemRangeChanged( @NonNull OPERATION_TYPE operationType, int startPosition, int itemCount )
  {
    for ( Notifier notifier : notifiers )
    {
      if ( notifier instanceof OnItemRangeChanged )
      {
        ( (OnItemRangeChanged) notifier ).onItemRangeChanged(operationType, startPosition, itemCount);
      }
    }
  }

  @Override
  public void updateSelection( int selection )
  {

  }

  @Override
  public int getCount()
  {
    return articleItems.size();
  }

  @Override
  public ArticleItem getItem( int position )
  {
    return articleItems.get(position);
  }

  @Override
  public Void getMetadata()
  {
    return null;
  }

  @Override
  public int getSelection()
  {
    return -1;
  }

  @Override
  public boolean isInProgress()
  {
    return false;
  }

  @Override
  public int getPosition()
  {
    return 0;
  }
}