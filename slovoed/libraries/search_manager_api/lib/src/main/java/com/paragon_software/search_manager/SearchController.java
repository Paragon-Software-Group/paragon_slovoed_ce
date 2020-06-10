package com.paragon_software.search_manager;

import android.app.Activity;
import android.content.Context;
import android.text.SpannableString;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.ScrollResult;
import com.paragon_software.native_engine.SearchAllResult;
import com.paragon_software.native_engine.SearchType;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.List;

import io.reactivex.Observable;

public interface SearchController
{
  String  CONTROLLER_TYPE_DEFAULT   = "CONTROLLER_TYPE_DEFAULT";
  String  CONTROLLER_TYPE_OALD   = "CONTROLLER_TYPE_OALD";
  String CONTROLLER_TYPE_DUDEN = "CONTROLLER_TYPE_DUDEN";
  String CONTROLLER_TYPE_BILINGUAL = "CONTROLLER_TYPE_BILINGUAL";

  void registerNotifier(Notifier notifier);

  void unRegisterNotifier(Notifier notifier);

  void setSelectedSearch(@Nullable SearchType searchType);

  @Nullable
  SearchType getSelectedSearch();

  ScrollResult scroll(String text , boolean autoChangeDirection, boolean exactly);

  SearchAllResult searchAll(String word);

  CollectionView<CollectionView<ArticleItem, GroupHeader>,Dictionary.Direction>
  search(@NonNull String word, boolean autoChangeDirection, @NonNull SearchType searchType, @NonNull SortType sortType);

  void setNeedUpdateSearchResults( boolean needUpdateSearchResults );

  boolean needUpdateSearchResults();

  void setSearchText(String text);

  String getSearchText();

  float getEntryListFontSize();

  boolean isShowKeyboardEnabled();

  boolean isShowHighlightingEnabled();

  CollectionView<ArticleItem, Dictionary.Direction> getArticles();

  CollectionView<ArticleItem, Void> getMorphoArticles();

  SearchAllResult getSearchAllArticles();

  CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> getSpecialSearchArticles();

  void setNormalSearchScrollState(@NonNull ScrollState scrollState);

  @NonNull
  ScrollState getNormalSearchScrollState();

  void setDictionaryFilterScrollState(@NonNull ScrollState scrollState);

  @NonNull
  ScrollState getDictionaryFilterScrollState();

  void setSpecialSearchScrollState(@NonNull ScrollState scrollState);

  @NonNull
  ScrollState getSpecialSearchScrollState();

  void setExpandedSpecialSearchGroupScrollState(@NonNull ScrollState scrollState);

  @NonNull
  ScrollState getExpandedSpecialSearchGroupScrollState();

  boolean isScrollResultStartsWith( @Nullable String text );

  Dictionary.DictionaryId getSearchDictionaryId();

  ArticleItem getActiveArticle();
  void setActiveArticle( @Nullable ArticleItem articleItem );

  Dictionary.DictionaryId getSelectedDictionary();

  int getSelectedDirection();

  List< Dictionary > getDictionaries();

  void setContext( Context context);

  void openArticle(ArticleItem item);

  void playSound(ArticleItem item);

  boolean itemHasSound(ArticleItem item);

  void setSpecialSearchGroupExpanded(boolean value);

  boolean isSpecialSearchGroupExpanded();

  void setExpandedSpecialSearchGroup(CollectionView<ArticleItem, GroupHeader> view);

  CollectionView<ArticleItem, GroupHeader> getExpandedSpecialSearchGroup();

  void setExpandedGroupHeader(@Nullable String header );

  @Nullable
  String getExpandedGroupHeader();

  void setFilterDictionaryId( Dictionary.DictionaryId dictionaryId);

  Dictionary.DictionaryId getFilterDictionaryId();

  CollectionView<Dictionary.DictionaryId, Void> getFilterDictionaryIdList();

  void setSelectedDirection(@Nullable Dictionary.Direction selectedDirection);

  void setEntryListFontSize( float entryListFontSize );

  void setShowKeyboardEnabled( boolean enabled );

  void setShowHighlightingEnabled( boolean enabled );

  void restoreSearchState();

  SpannableString getHighLightLabel(ArticleItem item);

  boolean IsScrollSelected();

  SpannableString getHeadWord(ArticleItem item);

  boolean launchTestMode( @NonNull Activity activity, @NonNull String text );

  boolean isFullDictionary();

  boolean isFtsNotAvailableInDictionary();

  boolean showHintDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager );

  void buySelectedDictionary(Activity activity);

  void openScreen(ScreenType screenType);

  interface Notifier { }

  @NonNull
  Observable< Boolean > getTopScreenOverlayStateObservable();
}
