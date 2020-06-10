package com.paragon_software.native_engine;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.text.SubstringInfo;

public interface EngineArticleAPI
{
  /**
   * Registers a dictionary manager to handle dictionary updates.
   *
   * @param manager dictionary manager to register
   */
  void registerDictionaryManager( @NonNull DictionaryManagerAPI manager );

  /**
   * Translates an item. Previous translation with same callback gets cancelled.
   *
   * @param current  current item, can be null
   * @param next     item to translate, can be null (in this case nothing is translated, but the previous translation still gets cancelled)
   * @param scale    translation page scale
   * @param callback callback that will receive the translation
   *
   * @return true if the translation has started and the callback will be called, otherwise false
   */
  boolean translateNext( @Nullable ArticleItem current, @Nullable ArticleItem next, @NonNull HtmlBuilderParams htmlParams,
                         @NonNull OnTranslationReadyCallback callback );
  /**
   * Finds a crossref string.
   *
   * @param dictionaryId current dictionary id
   * @param next         crossref string to translate
   * @param language     language of the crossref string
   *
   * @return found item or null
   */
  @Nullable
  ArticleItem find( @NonNull Dictionary.DictionaryId dictionaryId, @NonNull String next, int language );
  /**
   * Finds a linked ArticleItem for WotD
   *
   * @param dictionaryId current dictionary id
   * @param entryId      Word of the Day Entry Id
   * @param language     language of the crossref string
   *
   * @return found item or null
   */
   @Nullable
   ArticleItem findItemByEntryId(@NonNull String entryId, @NonNull Dictionary.DictionaryId dictionaryId, @NonNull Dictionary.Direction language);

    Object[] getSoundInfoFromExternalKey(@NonNull String entryId, @NonNull Dictionary.DictionaryId dictId, @NonNull Dictionary.Direction language);

    /**
   * Finds a link.
   *
   * @param dictionaryId current dictionary id
   * @param listId       list id og item to translate
   * @param globalIdx    global index of item to translate
   * @param anchor       anchor to scroll to after translation
   *
   * @return true if the translation has started and the callback will be called, otherwise false
   */
  @Nullable
  ArticleItem find( @NonNull Dictionary.DictionaryId dictionaryId, int listId, int globalIdx, @Nullable String anchor );

  /**
   * Determines if the dictionary has hide or switch blocks anywhere inside it.
   *
   * @param dictionaryId id of the dictionary
   *
   * @return true if it has hide or switch blocks, otherwise false
   */
  boolean hasHideOrSwitchBlocks( @NonNull Dictionary.DictionaryId dictionaryId );

  @NonNull
  CollectionView< CollectionView< ArticleItem, SubstringInfo >, Void > getWordReferenceInList(
      Dictionary.DictionaryId id, Dictionary.Direction direction, String query );

  @Nullable
  byte[] getExternalImage(Dictionary.DictionaryId dictId, String imageBaseId, int listId, String imageKey);

  /**
   * Interface of callbacks the receive translations from the engine.
   */
  interface OnTranslationReadyCallback
  {
    /**
     * @param article               the newly translated article item
     * @param translation           translation html
     * @param hasHideOrSwitchBlocks true if article has hide or switch blocks, otherwise false
     */
    void onTranslationReady( @NonNull ArticleItem article, @NonNull String translation, boolean hasHideOrSwitchBlocks,@Nullable ArticleSearcherAPI articleSearcher);
  }
}
