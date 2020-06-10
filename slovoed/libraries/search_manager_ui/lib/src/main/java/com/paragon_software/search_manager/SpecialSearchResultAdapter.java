package com.paragon_software.search_manager;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.search_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_DID_YOU_MEAN;

public class SpecialSearchResultAdapter extends BaseSearchResultAdapter<SpecialSearchResultAdapter.ViewHolder>
{

  private CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> mResultList;
  private OnArticleClickListener mOnArticleClickListener        = null;
  private OnSpecialSearchGroupExpandClickListener mOnSpecialSearchGroupExpandClickListener = null;
  private SearchController mSearchController;


  public interface OnSpecialSearchGroupExpandClickListener
  {
    void onSpecialSearchGroupExpandClick(CollectionView<ArticleItem, GroupHeader> view, String header);
  }

  public void registerOnArticleClickListener(OnArticleClickListener listener )
  {
    mOnArticleClickListener = listener;
  }

  public void registerOnSpecialSearchGroupExpandClickListener(OnSpecialSearchGroupExpandClickListener listener)
  {
    mOnSpecialSearchGroupExpandClickListener = listener;
  }

  public void setSearchController(SearchController searchController)
  {
    mSearchController = searchController;
  }

  public void setData(CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> result, float entryListFontSize)
  {
    mEntryListFontSize = entryListFontSize;
    if (null != mResultList && mResultList.equals(result))
    {
      return;
    }

    mResultList = result;
    notifyDataSetChanged();
  }

  public CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> getData()
  {
    return mResultList;
  }

  @NonNull
  @Override
  public SpecialSearchResultAdapter.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType )
  {
    return new ViewHolder(LayoutInflater.from(parent.getContext())
                                        .inflate(R.layout.fts_view_item,parent,false));
  }

  @Override
  public void onBindViewHolder( @NonNull ViewHolder holder, int position )
  {
      CollectionView<ArticleItem, GroupHeader> resultGroup = mResultList.getItem(position);
      holder.setData(resultGroup, mEntryListFontSize);
  }

  @Override
  public void onViewRecycled(@NonNull ViewHolder holder)
  {
    if (null != holder.mViewHolderCollectionView)
    {
      holder.mViewHolderCollectionView.unregisterListener(holder);
    }
  }

  @Override
  public int getItemCount()
  {
    if (null == mResultList)
    {
      return 0;
    }
    else
    {
      return mResultList.getCount();
    }

  }

  class ViewHolder extends RecyclerView.ViewHolder implements
                                                  View.OnClickListener
                                                 ,CollectionView.OnItemRangeChanged
  {
    View           mItem;
    ViewGroup.LayoutParams mParams;
    TextView       mTitle;
    LinearLayout   mGroupResultList;
    TextView       mOtherResults;
    CollectionView<ArticleItem, GroupHeader> mViewHolderCollectionView;
    String         mGroupHeader;

    int            mResId;


    ViewHolder( View v )
    {
      super(v);
      mItem = v;
      mTitle = v.findViewById(R.id.title);
      mGroupResultList = v.findViewById(R.id.normal_result_container);
      mOtherResults = v.findViewById(R.id.other_results);
      mOtherResults.setOnClickListener(this);
    }

    public void setData(CollectionView<ArticleItem, GroupHeader> data, float entryListFontSize)
    {
      if (!mResultList.isInProgress() && 0 == data.getCount())
      {
          mParams = mItem.getLayoutParams();
          mItem.setVisibility(View.GONE);
          mItem.setLayoutParams(new RecyclerView.LayoutParams(0,0));
          return;
      }

      if (View.GONE == mItem.getVisibility())
      {
        mItem.setVisibility(View.VISIBLE);
        mItem.setLayoutParams(mParams);
      }

      mEntryListFontSize = entryListFontSize;
      if (null != mViewHolderCollectionView)
      {
        mViewHolderCollectionView.unregisterListener(this);
      }
      mViewHolderCollectionView = data;
      if (null != mViewHolderCollectionView)
      {
        mViewHolderCollectionView.registerListener(this);
      }

      if (!SEARCH_TYPE_DID_YOU_MEAN.equals(mSearchController.getSelectedSearch()) )
      {
        GroupHeader label = data.getMetadata();
        mGroupHeader = label.listName.get();
      }
      else
      {
        mGroupHeader = mTitle.getContext().getString(R.string.search_manager_ui_did_you_mean);
      }

      mTitle.setText(mGroupHeader);

      mOtherResults.setText(R.string.search_manager_ui_see_other_results);
      mGroupResultList.removeAllViews();

      if (isFtsHeadwordVisible(data))
      {
        mResId = R.layout.fts_headword_article;
      }
      else
      {
        mResId = R.layout.base_article;
      }

      for (int i = 0; (i < 5) && (i < mViewHolderCollectionView.getCount()); ++i)
      {
        ArticleItem articleItem = mViewHolderCollectionView.getItem(i);
        if (null == articleItem)
        {
          continue;
        }

        View item = LayoutInflater.from(this.itemView.getContext()).inflate(mResId, mGroupResultList, false);
        item.setOnClickListener(this);

        setItemLabel(item, articleItem);
        setFtsHeadword(item, articleItem);
        setItemAudio(item,articleItem);
        mGroupResultList.addView(item);

      }

      if (mViewHolderCollectionView.getCount() > 5)
      {
        mOtherResults.setVisibility(View.VISIBLE);
      }
      else
      {
        mOtherResults.setVisibility(View.GONE);
      }
    }

    private void setItemLabel(View item, ArticleItem articleItem )
    {
      TextView textView = item.findViewById(R.id.text);
      textView.setText(mSearchController.getHighLightLabel(articleItem));
      textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, mEntryListFontSize);
    }

    private void setFtsHeadword( View item, ArticleItem articleItem )
    {
        final TextView ftsHeadword = item.findViewById(R.id.fts_headword);
        if (null == ftsHeadword) return;
        ftsHeadword.setText(articleItem.getFtsHeadword());
        ftsHeadword.setTextSize(TypedValue.COMPLEX_UNIT_SP, mEntryListFontSize);
    }

    private void setItemAudio( View item, ArticleItem articleItem )
    {
      View soundButton = item.findViewById(R.id.sound);
      soundButton.setTag(articleItem);

      if (null != mSearchController && mSearchController.itemHasSound(articleItem))
      {
        soundButton.setVisibility(View.VISIBLE);
      }
      soundButton.setOnClickListener(this);
    }

    @Override
    public void onClick( View v )
    {
      if (v == mOtherResults && null != mOnSpecialSearchGroupExpandClickListener)
      {
        mOnSpecialSearchGroupExpandClickListener.onSpecialSearchGroupExpandClick(mViewHolderCollectionView, mGroupHeader);
      }
      else if (v.getId() == R.id.sound)
      {
        int index = mGroupResultList.indexOfChild((View) v.getParent());
        if (-1 == index) return;
        mSearchController.playSound(mViewHolderCollectionView.getItem(index));
      }
      else
      {
        int index = mGroupResultList.indexOfChild(v);
        if (-1 == index) return;
        if (mOnArticleClickListener != null)
        {
          mOnArticleClickListener.onArticleClick(mViewHolderCollectionView.getItem(index));
        }
      }
    }

    @Override
    public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int position, int count )
    {
      if (position > 5) return;
      notifyItemChanged(getAdapterPosition());
    }
  }

  public static boolean isFtsHeadwordVisible(CollectionView collectionView )
  {
    //noinspection SimplifiableIfStatement
    if ( collectionView.getMetadata() instanceof GroupHeader)
    {
      return ( (GroupHeader) collectionView.getMetadata() ).isHeadwordVisible;
    }
    return false;
  }
}
