package com.paragon_software.article_manager.controllers;

import android.content.res.Resources;
import android.graphics.drawable.NinePatchDrawable;
import androidx.annotation.NonNull;
import androidx.core.content.res.ResourcesCompat;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.style.CharacterStyle;
import android.text.style.ClickableSpan;
import android.text.style.RelativeSizeSpan;
import android.view.View;
import android.widget.TextView;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ShareControllerAPI;
import com.paragon_software.article_manager.spans.SpanRounded;
import com.paragon_software.common_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.text.SubstringInfo;

public class WordsViewController implements CollectionView.OnItemRangeChanged, CollectionView.OnSelectionChange {
	// Workaround from: https://svn-shdd.paragon-software.com/svn/Projects/SlovoEd for Android/Branches/5.5/app/src/main/java/com/paragon/dictionary/ShareWordsFragment.java
	private static final String EXTRA_LINE = "\n ";//it's used with android:lineSpacingExtra="10dp" android:lineSpacingMultiplier="1" for last line's height adjustment.
	@NonNull
	private final TextView mTextView;
	@NonNull
	private final String mInitiallyQuery;
	@NonNull
	final Resources mResources;
	@NonNull
	private final ShareControllerAPI mShareController;
	@NonNull
	private final CollectionView<CollectionView<ArticleItem, SubstringInfo>, Void>
			mWordsCollectionView;
	protected int mColorTextSelected;
	protected int mColorTextRegular;
	protected float mSpanPaddingStartEnd;
	protected NinePatchDrawable mNinePatchDrawableRegular;
	protected NinePatchDrawable mNinePatchDrawableSelected;

	public WordsViewController(@NonNull Resources.Theme theme, @NonNull TextView textView,
							   @NonNull String initiallyQuery, @NonNull ShareControllerAPI shareController) {

		this.mTextView = textView;
		this.mResources = textView.getResources();
		this.mInitiallyQuery = initiallyQuery;
		this.mShareController = shareController;
		this.mWordsCollectionView = mShareController.getWords();
		mColorTextSelected = ResourcesCompat.getColor(mResources, R.color.share_query_text_selected_color, theme);
		mColorTextRegular = ResourcesCompat.getColor(mResources, R.color.share_query_text_regular_color, theme);
		mSpanPaddingStartEnd = mResources.getDimension(R.dimen.share_query_span_padding);
		mNinePatchDrawableRegular = (NinePatchDrawable) ResourcesCompat.getDrawable(mResources, R.drawable.share_query_span_clickable_background_regular,
				theme);
		mNinePatchDrawableSelected = (NinePatchDrawable) ResourcesCompat.getDrawable(mResources, R.drawable.share_query_span_clickable_background_selected,
				theme);
	}

	public void unregisterFromCollectionView() {
		mWordsCollectionView.unregisterListener(this);
	}

	@Override
	public void onItemRangeChanged(CollectionView.OPERATION_TYPE type, int startPosition, int itemCount) {
		populate();
	}

	public void registerToCollectionView() {
		mWordsCollectionView.registerListener(this);
	}

	public void populate() {
		String query = mInitiallyQuery + EXTRA_LINE;
		SpannableStringBuilder ssb = new SpannableStringBuilder(query);
		for (int i = 0; i < mWordsCollectionView.getCount(); i++) {
			CollectionView<ArticleItem, SubstringInfo> item = mWordsCollectionView.getItem(i);
			if (null != item) {
				int selection = mWordsCollectionView.getSelection();
				boolean spanSelected = (selection == i);
				SubstringInfo substring = item.getMetadata();
				int end = substring.getOffset() + substring.getLength();

				ssb.setSpan(createSpan(spanSelected), substring.getOffset(), end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
				ssb.setSpan(creteClickableSpan(i), substring.getOffset(), end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
			}
		}
		// Prevents last spans to be cut
		ssb.setSpan(new RelativeSizeSpan(.01f), query.length() - 1, query.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
		mTextView.setText(ssb);
	}

	/**
	 * Create invisible clickable span
	 *
	 * @param index of clickable span
	 */
	private CharacterStyle creteClickableSpan(final int index) {
		return new ClickableSpan() {
			@Override
			public void onClick(View view) {
				mShareController.selectWord(index);
			}

			@Override
			public void updateDrawState(TextPaint ds) {
			}
		};
	}

	private CharacterStyle createSpan(boolean selected) {
		if (selected) {
			return new SpanRounded(mSpanPaddingStartEnd, mColorTextSelected, mResources, mNinePatchDrawableSelected);
		} else {
			return new SpanRounded(mSpanPaddingStartEnd, mColorTextRegular, mResources, mNinePatchDrawableRegular);
		}
	}

	@Override
	public void onSelectionChanged() {
		populate();
	}
}
