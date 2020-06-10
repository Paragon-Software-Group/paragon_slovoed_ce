package com.paragon_software.search_manager;

import android.graphics.Color;
import android.graphics.Typeface;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.ForegroundColorSpan;
import android.text.style.RelativeSizeSpan;
import android.text.style.StyleSpan;
import android.text.style.SuperscriptSpan;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.SearchType;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_DID_YOU_MEAN;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_FTS;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_WILD_CARD;

public class DudenSearchController extends ParagonSearchController {

	private final static String partOfSpeechColor = "#333333";
	private final static String highLightColor = "#FDDD7F";

	DudenSearchController(SearchManagerAPI manager, ScreenOpenerAPI screenOpener, HintManagerAPI hintManager,
                          ToolbarManager toolbarManager ) {
		super(manager, screenOpener, hintManager, toolbarManager);
		mIsScrollSelected = true;
	}

	@Override
	protected void setToolbarFtsMode() {
	}

	@Override
	public void setSearchText(String text) {
		mSearchText = text;
		if(mSelectedSearch == SEARCH_TYPE_DID_YOU_MEAN) {
			mSelectedSearch = SEARCH_TYPE_FTS;
		}
	}

	@Override
	public CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction>
	search(@NonNull String word, boolean autoChangeDirection, @NonNull SearchType searchType, @NonNull SortType sortType)
	{
		boolean needSearch = !word.isEmpty() || (!SearchType.SEARCH_TYPE_WILD_CARD.equals(searchType) && !SearchType.SEARCH_TYPE_FUZZY.equals(searchType));
		return search(word, autoChangeDirection, searchType, sortType, needSearch);
	}

	@Override
	public SpannableString getHeadWord(ArticleItem item) {
		StringBuilder sb = new StringBuilder();
		String text = item.getFtsHeadword() == null ? item.getLabel() : item.getFtsHeadword();
		String num = item.getNumeration() != null ? item.getNumeration() : "";
		String partOfSpeech = item.getPartOfSpeech() != null ? item.getPartOfSpeech() : "";

		sb.append(text).append(num).append(" ").append(partOfSpeech).append(" ");
		SpannableString headWord = new SpannableString(sb);

		int position = text != null ? text.length() : 0;
		headWord.setSpan(new SuperscriptSpan(), position, position + num.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
		headWord.setSpan(new RelativeSizeSpan(0.7f), position, position += num.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

		headWord.setSpan(new StyleSpan(Typeface.ITALIC), position += 1, position + partOfSpeech.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
		headWord.setSpan(new ForegroundColorSpan(Color.parseColor(partOfSpeechColor)), position, position + partOfSpeech.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

		return headWord;
	}

	@Override
	int getHighLightColor() {
		return Color.parseColor(highLightColor);
	}

	@Override
	public void restoreSearchState() {
	}
}
