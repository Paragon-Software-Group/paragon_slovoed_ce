package com.paragon_software.search_manager;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.search_manager_ui.R;

public class MorphoAdapter extends BaseSearchResultAdapter<MorphoAdapter.ViewHolder>
{
    private OnArticleClickListener    mOnArticleClickListener  = null;
    private CollectionView<ArticleItem, Void> mMorphoResultList;
    private SearchController mSearchController;

    public void registerOnArticleClickListener(OnArticleClickListener listener){
        mOnArticleClickListener = listener;
    }

    public void setSearchController(SearchController searchController)
    {
        mSearchController = searchController;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.morpho_article, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {

        if (null != mMorphoResultList)
        {
            holder.setData(mMorphoResultList.getItem(position),mEntryListFontSize);
        }
    }

    @Override
    public int getItemCount() {
        if (null == mMorphoResultList)
        {
            return 0;
        }
        else {
            return mMorphoResultList.getCount();
        }
    }

    public void setData(CollectionView<ArticleItem, Void> result, float entryListFontSize)
    {
        mEntryListFontSize = entryListFontSize;
        if (null != mMorphoResultList && mMorphoResultList.equals(result))
        {
            return;
        }

        mMorphoResultList = result;
        notifyDataSetChanged();
    }

    public CollectionView<ArticleItem, Void> getData()
    {
        return mMorphoResultList;
    }

    class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener
    {
        private final TextView  mTextView;
        private ArticleItem     mItem;

        ViewHolder(View view)
        {
            super(view);
            mTextView = view.findViewById(R.id.text);
            view.setOnClickListener(this);
        }

        public void setData(ArticleItem item, float entryListFontSize)
        {
            mTextView.setTextSize(TypedValue.COMPLEX_UNIT_SP, entryListFontSize);
            mTextView.setText(item.getLabel());
            mItem = item;
        }

        @Override
        public void onClick(View v) {
            if (null != mOnArticleClickListener )
            {
                mOnArticleClickListener.onArticleClick(mItem);
            }
        }
    }
}
