package com.paragon_software.article_manager.controllers;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.google.android.material.tabs.TabLayout;
import android.view.View;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.BaseShareActivity;
import com.paragon_software.article_manager.ShareControllerAPI;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.text.SubstringInfo;

public class TabsViewController implements CollectionView.OnSelectionChange, CollectionView.OnItemRangeChanged
{
  @NonNull
  private final CollectionView< CollectionView< ArticleItem, SubstringInfo >, Void >
                                                                      mWordsCollectionView;
  @NonNull
  private final TabLayout                                             mTabLayout;
  private final BaseShareActivity.ArticleControllerAPIWrapper         mArticleControllerWrapper;
  @Nullable
  private       CollectionView< ArticleItem, SubstringInfo > mTabsCollectionView;
  @NonNull
  private final ShareControllerAPI                                    mShareController;

  private CollectionView.OnSelectionChange tabsSelectionChangeListener = new CollectionView.OnSelectionChange()
  {
    @Override
    public void onSelectionChanged()
    {
      mArticleControllerWrapper.toggleSearchUI(false);
    }
  };

  public TabsViewController( @NonNull TabLayout tabLayout, @NonNull ShareControllerAPI shareController,
                             BaseShareActivity.ArticleControllerAPIWrapper articleController )
  {

    this.mTabLayout = tabLayout;
    this.mShareController = shareController;
    this.mArticleControllerWrapper = articleController;
    this.mWordsCollectionView = shareController.getWords();
  }

  public void populate()
  {
    int selection = mWordsCollectionView.getSelection();
    if ( (0 <= selection) && (selection < mWordsCollectionView.getCount()) )
    {
      CollectionView< ArticleItem, SubstringInfo > tabsCollectionView =
          mWordsCollectionView.getItem(selection);
      if ( null != tabsCollectionView && tabsCollectionView.getCount() > 1 )
      {
        mTabLayout.removeOnTabSelectedListener(mTabSelectedListener);
        mTabLayout.setVisibility(View.VISIBLE);
        mTabLayout.removeAllTabs();
        int selectionOfTab = tabsCollectionView.getSelection();
        for ( int i = 0 ; i < tabsCollectionView.getCount() ; i++ )
        {
          ArticleItem articleItem = tabsCollectionView.getItem(i);
          if ( null != articleItem )
          {
            TabLayout.Tab tab = mTabLayout.newTab();
            tab.setText(articleItem.getShowVariantText());
            mTabLayout.addTab(tab);
            if ( i == selectionOfTab )
            {
              tab.select();
            }
          }
        }
        mTabLayout.addOnTabSelectedListener(mTabSelectedListener);
      }
      else
      {
        mTabLayout.setVisibility(View.GONE);
      }
      listenTabsSelection(tabsCollectionView);
      mArticleControllerWrapper.toggleSearchUI(false);
    }
    else
    {
      mTabLayout.setVisibility(View.GONE);
    }
  }

  private void listenTabsSelection( @Nullable CollectionView< ArticleItem, SubstringInfo > tabsCollectionView )
  {
    if ( null != tabsCollectionView )
    {
      if ( null != mTabsCollectionView )
      {
        mTabsCollectionView.unregisterListener(tabsSelectionChangeListener);
      }
      mTabsCollectionView = tabsCollectionView;
      mTabsCollectionView.registerListener(tabsSelectionChangeListener);
    }
  }

  public void unregisterFromCollectionView()
  {
    if ( null != mTabsCollectionView )
    {
      mTabsCollectionView.unregisterListener(tabsSelectionChangeListener);
    }
    mWordsCollectionView.unregisterListener(this);
  }

  public void registerToCollectionView()
  {
    mWordsCollectionView.registerListener(this);
  }

  private TabLayout.OnTabSelectedListener mTabSelectedListener = new TabLayout.OnTabSelectedListener()
  {
    @Override
    public void onTabSelected( TabLayout.Tab tab )
    {
      mShareController.selectTab(tab.getPosition());
    }

    @Override
    public void onTabUnselected( TabLayout.Tab tab )
    {
    }

    @Override
    public void onTabReselected( TabLayout.Tab tab )
    {
      mShareController.selectTab(tab.getPosition());
    }
  };

  @Override
  public void onSelectionChanged()
  {
    populate();
  }

  @Override
  public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int startPosition, int itemCount )
  {
    if ( type.equals(CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED) )
    {
      populate();
    }
  }
}
