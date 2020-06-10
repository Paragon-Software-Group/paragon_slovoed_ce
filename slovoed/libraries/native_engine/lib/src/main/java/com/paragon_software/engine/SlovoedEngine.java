package com.paragon_software.engine;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.extbase.PictureInfo;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.nativewrapper.ArticleSearcher;
import com.paragon_software.engine.nativewrapper.EngineInformation;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.rx.deserializearticle.DeserializeArticlesTaskRunner;
import com.paragon_software.engine.rx.deserializearticle.DeserializerUtils;
import com.paragon_software.engine.rx.getadditional.GetAdditional;
import com.paragon_software.engine.rx.getwordreferenceinlist.GetWordReferenceInList;
import com.paragon_software.engine.rx.getwordreferenceinlist.Params;
import com.paragon_software.engine.rx.preloadedwords.PreloadedFavoritesTaskRunner;
import com.paragon_software.engine.rx.scrollandfts.ScrollAndSpecialSearch;
import com.paragon_software.engine.rx.searchalldictionaries.SearchAllDictionaries;
import com.paragon_software.engine.rx.translatearticle.TranslateArticle;
import com.paragon_software.native_engine.*;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.directory.Directory;
import com.paragon_software.utils_slovoed.text.SubstringInfo;
import io.reactivex.Single;

import java.io.Serializable;
import java.util.Collection;

public class SlovoedEngine implements SearchEngineAPI, EngineArticleAPI, EngineSerializerAPI, EngineInformationAPI
{
  private static final boolean DEFAULT_HAS_HIDE_OR_SWITCH_BLOCKS = false;

  private final Context mApplicationContext;

  @NonNull
  private final ScrollAndSpecialSearch mScrollAndSpecialSearch = new ScrollAndSpecialSearch();

  @NonNull
  private final SearchAllDictionaries mSearchAllDictionaries = new SearchAllDictionaries();

  @NonNull
  private final GetWordReferenceInList mGetWordReferenceInList = new GetWordReferenceInList();

  @NonNull
  private final TranslateArticle mTranslateArticle = new TranslateArticle();

  @NonNull
  private final DeserializeArticlesTaskRunner mDeserializeArticles = new DeserializeArticlesTaskRunner();

  @NonNull final GetAdditional mGetAdditional = new GetAdditional();

  @NonNull
  private final PreloadedFavoritesTaskRunner mPreloadedFavoritesTaskRunner = new PreloadedFavoritesTaskRunner();

  @Nullable
  private DictionaryManagerAPI mDictionaryManager = null;

  public SlovoedEngine( final Context context )
  {
    mApplicationContext = context.getApplicationContext();
  }

  @Override
  public void registerDictionaryManager( @NonNull DictionaryManagerAPI manager )
  {
    mScrollAndSpecialSearch.init(mApplicationContext, manager);
    mSearchAllDictionaries.init(mApplicationContext, manager);
    mTranslateArticle.init(mApplicationContext, manager);
    mDeserializeArticles.init(mApplicationContext, manager);
    mGetWordReferenceInList.init(mApplicationContext, manager);
    mGetAdditional.init(mApplicationContext, manager);
    mPreloadedFavoritesTaskRunner.init(mApplicationContext, manager);
    mDictionaryManager = manager;
  }

  @Override
  public boolean translateNext( @Nullable ArticleItem current, @Nullable ArticleItem next, @NonNull HtmlBuilderParams htmlParams,
                                @NonNull OnTranslationReadyCallback callback )
  {
    mTranslateArticle.translate(callback, next, htmlParams);
    return true;
  }


  @Nullable
  @Override
  public ArticleItem find( @NonNull Dictionary.DictionaryId dictionaryId, @NonNull String headword, int language )
  {
    //TODO: need to implement in future
    return null;
  }

  @Nullable
  @Override
  public ArticleItem findItemByEntryId(@NonNull String entryId, @NonNull Dictionary.DictionaryId dictId, @NonNull Dictionary.Direction language) {
    ArticleItem res = null;
    if (mDictionaryManager != null) {
      for (Dictionary dictionary : mDictionaryManager.getDictionaries()) {
        if (dictionary.getId().equals(dictId)) {
          NativeDictionary nativeDictionary = NativeDictionary.open(mApplicationContext, dictionary.getDictionaryLocation(), dictionary.getMorphoInfoList(), false);
          if (nativeDictionary != null) {
            ArticleSearcher articleSearcher = new ArticleSearcher(nativeDictionary);
            res = articleSearcher.findLinkedWotDArticleItem(dictId, entryId, language.getLanguageFrom());
            nativeDictionary.close();
            break;
          }
        }
      }
    }
    return res;
  }

  @Nullable
  @Override
  public Object[] getSoundInfoFromExternalKey(@NonNull String entryId, @NonNull Dictionary.DictionaryId dictId, @NonNull Dictionary.Direction language) {
    Object[] stylizedVariant = new Object[]{};
    if (mDictionaryManager != null) {
      for (Dictionary dictionary : mDictionaryManager.getDictionaries()) {
        if (dictionary.getId().equals(dictId)) {
          NativeDictionary nativeDictionary = NativeDictionary.open(mApplicationContext, dictionary.getDictionaryLocation(), dictionary.getMorphoInfoList(), false);
          if (nativeDictionary != null) {
            ArticleSearcher articleSearcher = new ArticleSearcher(nativeDictionary);
            stylizedVariant = articleSearcher.getSoundInfoFromExternalKey(entryId, language.getLanguageFrom());
            nativeDictionary.close();
            break;
          }
        }
      }
    }
    return stylizedVariant;
  }

  @Nullable
  @Override
  public ArticleItem find( @NonNull Dictionary.DictionaryId dictionaryId, int listId, int globalIdx,
                           @Nullable String anchor )
  {
    //TODO: now result from this methods uses in another classes,
    // therefore, we do not remove this line, but in the future we need to implement this method
    return new ArticleItem.Builder(dictionaryId, listId, globalIdx).setLabel(anchor).build();
  }

  @Override
  public boolean hasHideOrSwitchBlocks( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    //TODO: need to implement in future, now return default value
    return DEFAULT_HAS_HIDE_OR_SWITCH_BLOCKS;
  }

  @Nullable
  @Override
  public Serializable serializeArticleItem( @NonNull ArticleItem articleItem )
  {
    return new PersistentArticle(articleItem);
  }

  @Nullable
  @Override
  public ArticleItem deserializeArticleItem( @NonNull Serializable serializable )
  {
    ArticleItem res = null;
    if ( serializable instanceof PersistentArticle )
    {
      if ( mDictionaryManager != null && mApplicationContext != null )
      {
        res = DeserializerUtils.deserializeFromPersistentArticle(
            (PersistentArticle) serializable, mDictionaryManager, mApplicationContext);
      }
    }
    return res;
  }

  @Override
  public void deserializeArticleItems( @NonNull Serializable[] serialized,
                                       @NonNull OnDeserializedArticlesReadyCallback callback,
                                       boolean openSoundBases )
  {
    mDeserializeArticles.deserializeArticleItems(serialized, callback, openSoundBases);
  }

  @Override
  public Single< Directory< ArticleItem > > getPreloadedFavorites( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    return mPreloadedFavoritesTaskRunner.getPreloadedFavorites(dictionaryId);
  }

  @Override
  public ScrollResult scroll(Dictionary.DictionaryId id, Dictionary.Direction direction,
                             String word, @Nullable Collection<Dictionary.Direction> availableDirections, boolean exactly)
  {
    return mScrollAndSpecialSearch.scroll(id, direction, word, availableDirections, exactly);
  }

  @Override
  public CollectionView< CollectionView< ArticleItem, GroupHeader >, Dictionary.Direction>
    search(Dictionary.DictionaryId id,
           Dictionary.Direction direction,
           String word,
           @Nullable Collection<Dictionary.Direction> availableDirections,
           @NonNull SearchType searchType,
           @NonNull SortType sortType,
           @NonNull Boolean needRunSearch)
  {
    return mScrollAndSpecialSearch.search(id, direction, word, availableDirections, searchType, sortType, needRunSearch);
  }

  @Override
  public CollectionView<ArticleItem, Void> getAdditionalArticles(Dictionary.DictionaryId id) {
    return mGetAdditional.getAdditionalArticles(id);
  }

  @Override
  public SearchAllResult searchAll( String word, int maxWords )
  {
    return mSearchAllDictionaries.search(( word != null ) ? word : "", maxWords);
  }

  @NonNull
  @Override
  public CollectionView< CollectionView< ArticleItem, SubstringInfo >, Void > getWordReferenceInList(
      Dictionary.DictionaryId id, Dictionary.Direction direction, String query )
  {
    return mGetWordReferenceInList.search(new Params(id, direction, query)).get();
  }

  @Nullable
  @Override
  public byte[] getExternalImage(Dictionary.DictionaryId dictId, String imageBaseId, int listId, String imageKey) {
    if (mDictionaryManager != null) {
      for(Dictionary dict : mDictionaryManager.getDictionaries())
      {
       if (dict.getId().equals(dictId)) {
        for(PictureInfo pictureInfo: dict.getPictureInfoList())
        {
          if(pictureInfo.getSdcId().equalsIgnoreCase(imageBaseId))
          {
            NativeDictionary nativeDictionary = NativeDictionary.open(mApplicationContext, dict.getPictureInfoList().get(0).getLocation(), null,  false);
            if (nativeDictionary != null) {
              byte[] imgRes = nativeDictionary.getExternalImage(listId, imageKey);
              nativeDictionary.close();
              return imgRes;
            }
          }
        }
      }
      }
    }
    return new byte[] {};
  }

  @NonNull
  @Override
  public EngineVersion getEngineVersion()
  {
    return EngineInformation.INSTANCE;
  }

  @Nullable
  @Override
  public DictionaryInfoAPI getDictionaryInfo( DictionaryLocation location )
  {
    NativeDictionary nativeDictionary = ExternalBasesHolder.get(location);
    return null != nativeDictionary ? nativeDictionary.getDictionaryInfo() : null;
  }
}
