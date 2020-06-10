package com.paragon_software.search_manager;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.TextView;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.search_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;

public class SpecialSearchGroupAdapter extends BaseSearchResultAdapter<SpecialSearchGroupAdapter.ViewHolder>
{

  private SearchController mSearchController;
  private CollectionView<ArticleItem, ?> mSpecialSearchGroup;
  private int mResId;
  private OnArticleClickListener    mOnArticleClickListener  = null;

  private final static int VIEW_TYPE_NORMAL = 0;
  private final static int VIEW_TYPE_HEADWORD = 1;

  public void registerOnArticleClickListener(OnArticleClickListener listener){
    mOnArticleClickListener = listener;
  }

  public void setSearchController(SearchController searchController)
  {
    mSearchController = searchController;
  }

  public void setData(CollectionView<ArticleItem, ?> specialSearchGroup, float entryListFontSize)
  {
    mEntryListFontSize = entryListFontSize;
    mSpecialSearchGroup = specialSearchGroup;
    notifyDataSetChanged();
  }

  @NonNull
  @Override
  public SpecialSearchGroupAdapter.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType )
  {
    if (VIEW_TYPE_HEADWORD == viewType)
    {
      mResId = R.layout.fts_headword_article;
    }
    else
    {
      mResId = R.layout.base_article;
    }
    return new ViewHolder(LayoutInflater.from(parent.getContext())
                      .inflate(mResId,parent,false));
  }

  @Override
  public int getItemViewType( int position )
  {
    if (SpecialSearchResultAdapter.isFtsHeadwordVisible(mSpecialSearchGroup))
    {
      return VIEW_TYPE_HEADWORD;
    }
    else
    {
      return VIEW_TYPE_NORMAL;
    }
  }

  @Override
  public void onBindViewHolder(@NonNull SpecialSearchGroupAdapter.ViewHolder holder, int position )
  {
      holder.setData(mSpecialSearchGroup.getItem(position));
  }

  @Override
  public int getItemCount()
  {
    if (null == mSpecialSearchGroup)
    {
      return 0;
    }
    else
    {
      return mSpecialSearchGroup.getCount();
    }
  }

  class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener
  {
    ArticleItem mItem;
    TextView mText;
    TextView mFtsHeadword;
    ImageButton mSound;

    ViewHolder(View view )
    {
      super(view);
      view.setOnClickListener(this);
      mText = view.findViewById(R.id.text);
      mFtsHeadword = view.findViewById(R.id.fts_headword);
      mSound = view.findViewById(R.id.sound);
      mSound.setOnClickListener(this);
    }

    public void setData( ArticleItem item)
    {
      mItem = item;
      mText.setText(mSearchController.getHighLightLabel(item));
      mText.setTextSize(TypedValue.COMPLEX_UNIT_SP, mEntryListFontSize);

      if (null != mFtsHeadword)
      {
        mFtsHeadword.setText(item.getFtsHeadword());
        mFtsHeadword.setTextSize(TypedValue.COMPLEX_UNIT_SP, mEntryListFontSize);
      }

      if (mSearchController.itemHasSound(item))
      {
        mSound.setVisibility(View.VISIBLE);
      }
      else
      {
        mSound.setVisibility(View.GONE);
      }

    }

    @Override
    public void onClick(View v) {
      if (v == mSound)
      {
        mSearchController.playSound(mItem);
      }
      else if (null != mOnArticleClickListener )
      {
          mOnArticleClickListener.onArticleClick(mItem);
      }
    }
  }


}
