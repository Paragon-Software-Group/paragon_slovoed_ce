package com.paragon_software.native_engine;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.Dictionary.DictionaryId;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.directory.Directory;
import io.reactivex.Single;

import java.util.Collection;

public interface SearchEngineAPI
{
  void registerDictionaryManager(@NonNull DictionaryManagerAPI manager);

  SearchAllResult searchAll(String word, int maxWords);

  ScrollResult scroll(DictionaryId id,
                      Dictionary.Direction direction,
                      String word,
                      @Nullable Collection<Dictionary.Direction> availableDirections,
                      boolean exactly);

  CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction>
  search(DictionaryId id,
         Dictionary.Direction direction,
         String word,
         @Nullable Collection<Dictionary.Direction> availableDirections,
         @NonNull SearchType searchType,
         @NonNull SortType sortType,
         @NonNull Boolean needRunSearch);

  CollectionView<ArticleItem, Void> getAdditionalArticles(DictionaryId id);

  Single< Directory< ArticleItem > > getPreloadedFavorites( @NonNull Dictionary.DictionaryId dictionaryId );
}
