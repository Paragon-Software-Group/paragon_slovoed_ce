package com.paragon_software.article_manager;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.flash_cards_manager.FlashcardManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.EngineArticleAPI;
import com.paragon_software.search_manager.SearchManagerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.splash_screen_manager.ISplashScreenManager;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.text.SubstringInfo;

import java.util.List;

public abstract class ArticleManagerAPI
{
  @NonNull
  public static final String EXTRA_CONTROLLER_ID = "CONTROLLER_ID";

  public abstract void registerNotifier( @NonNull Notifier notifier );

  public abstract void unregisterNotifier( @NonNull Notifier notifier );

  public abstract void registerArticleUI( @NonNull String articleUI, @NonNull Class activity );

  public abstract void registerShareUI( @NonNull String s, @NonNull Class activity );

  public abstract void registerPractisePronunciationUI(@NonNull String s, @NonNull Class activity);

  @Nullable
  public abstract Class<Activity> getShareActivity();

  public abstract void registerDictionaryManager( @NonNull DictionaryManagerAPI manager );

  public abstract void registerSoundManager( @NonNull SoundManagerAPI manager );

  public abstract void registerFavoritesManager( @NonNull FavoritesManagerAPI manager );

  public abstract void registerHistoryManager( @NonNull HistoryManagerAPI manager );

  public abstract void registerSettingsManager( @NonNull SettingsManagerAPI manager );

  public abstract void registerSearchManager ( @NonNull SearchManagerAPI manager );

  public abstract void registerSplashScreenManager( @NonNull ISplashScreenManager manager );

  public abstract void registerEngine( @NonNull EngineArticleAPI engine );

  public abstract void registerFlashcardManager( @NonNull FlashcardManagerAPI manager );

  public abstract void  registerHintManager( @Nullable HintManagerAPI hintManager );

  public abstract boolean showArticleActivity(@Nullable ArticleItem article, @NonNull String articleUI, Context context );

  public abstract boolean showArticleActivity(@Nullable ArticleItem article, @Nullable ShowArticleOptions showArticleOptions, @NonNull String articleUI, Context context );

  public abstract boolean showArticle(@Nullable ArticleItem articleItem,
                                      @Nullable ShowArticleOptions showArticleOptions,
                                      @NonNull String articleUI,
                                      Context context);

  public abstract boolean showArticleActivityFromSeparateList(@NonNull List<ArticleItem> separateArticles,
                                                              int currentArticleIndex,
                                                              @NonNull String articleUI,
                                                              Context context);

  public abstract boolean showArticleFromSeparateList(@NonNull List<ArticleItem> separateArticles,
                                                      int currentArticleIndex,
                                                      @NonNull String articleUI,
                                                      Context context);

  public abstract float getArticleScale();

  abstract void setArticleScale( float newScale );

  abstract void saveState();

  @NonNull
  abstract ArticleControllerAPI getArticleController(@NonNull String articleUI );

  abstract void freeArticleController(@NonNull String articleUI );

  abstract void setActiveArticleUI( @Nullable String articleUI );

  @NonNull
  abstract ShareControllerAPI getShareController( @NonNull String articleUI );

  @NonNull
  abstract PractisePronunciationControllerAPI getPractisePronunciationController( @NonNull Context context, @NonNull String articleUI );

  @NonNull
  abstract CollectionView< CollectionView<ArticleItem, SubstringInfo>, Void> getWords( @Nullable String query, @Nullable Dictionary dictionary, @Nullable Dictionary.Direction selectedDirection );

  abstract void showSplashScreen( @NonNull Activity activity, @Nullable PendingIntent onCompletePendingIntent );

  abstract boolean haveNotShownSplashScreens();

  @Nullable
  public abstract byte[] getExternalImage(Dictionary.DictionaryId dictId, String imageBaseId, int listId, String imageKey);

  interface Notifier
  {
  }

  @Nullable
  public abstract ArticleItem findForSwipeSeparateArticles(boolean isSwipeForward);

  @Nullable
  abstract ArticleItem makeStepForSwipeSeparateArticles(boolean isSwipeForward);
}
