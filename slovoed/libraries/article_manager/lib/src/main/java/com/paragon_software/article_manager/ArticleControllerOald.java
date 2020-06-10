package com.paragon_software.article_manager;

import android.app.Activity;
import android.content.Context;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.util.Pair;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.analytics_manager.events.ProductDescriptionAndPricesScreenEvent;
import com.paragon_software.analytics_manager.events.ScreenName;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.flash_cards_manager.FlashcardManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.EngineArticleAPI;
import com.paragon_software.native_engine.HtmlBuilderParams;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

import static com.paragon_software.utils_slovoed.convertValues.ConverterBaseId.convertOald10SoundBase;

public class ArticleControllerOald extends ArticleController {

	private Pair< ArticleItem, ShowArticleOptions > mBackItem;

	private ArticleControllerOald(@NonNull ArticleManagerAPI _articleManager,
								  @NonNull String _controllerId,
								  @Nullable DictionaryManagerAPI _dictionaryManager,
								  @Nullable SoundManagerAPI _soundManager,
								  @Nullable FavoritesManagerAPI _favoritesManager,
								  @Nullable HistoryManagerAPI _historyManager,
								  @Nullable SettingsManagerAPI _settingsManager,
								  @Nullable EngineArticleAPI _engine,
								  @Nullable FlashcardManagerAPI _flashcardManager, @Nullable HintManagerAPI _hintManager) {
		super(_articleManager, _controllerId, _dictionaryManager, _soundManager, _favoritesManager, _historyManager, _settingsManager, _engine, _flashcardManager, _hintManager);
	}

	static ArticleControllerAPI create( @NonNull ArticleManagerAPI _articleManager, @NonNull String _controllerId,
										@Nullable DictionaryManagerAPI _dictionaryManager,
										@Nullable SoundManagerAPI _soundManager,
										@Nullable FavoritesManagerAPI _favoritesManager,
										@Nullable HistoryManagerAPI _historyManager,
										@Nullable SettingsManagerAPI _settingsManager, @Nullable EngineArticleAPI _engine,
										@Nullable FlashcardManagerAPI _flashcardManager, @Nullable HintManagerAPI _hintManager ) {
		ArticleControllerOald articleController =
				new ArticleControllerOald(_articleManager, _controllerId, _dictionaryManager, _soundManager, _favoritesManager,
						_historyManager, _settingsManager, _engine, _flashcardManager, _hintManager);
		initController(articleController);
		articleController.mArticle.scale = _articleManager.getArticleScale();
		return articleController;
	}

	@NonNull
	@Override
	public VisibilityState getDemoBannerVisibility() {
		VisibilityState res;
		if (mArticle.isLocked) {
			res = VisibilityState.enabled;
		} else {
			res = VisibilityState.gone;
		}
		return res;
	}

	@Override
	public void buy( @NonNull Activity activity ) {
		ArticleItem articleItem = mArticle.stack.getCurrentItem();
		if ((!mTransitionState.translation.mustDisableArticleButtons())
				&& (null != mDictionaryInfo.edition)
				&& (mDictionaryManager != null)
				&& (articleItem != null)) {

			if (mDictionaryInfo.edition.canBePurchased()) {
				ScreenName from = ScreenName.ARTICLE_DEMO;
				AnalyticsManagerAPI.get().logEvent(new ProductDescriptionAndPricesScreenEvent(from));
				mDictionaryManager.buy(activity, articleItem.getDictId());
			} else {
				mDictionaryManager.openMyDictionariesUI(activity, articleItem.getDictId());
			}
		}
	}

	@Override
	protected void addArticleInHistory( ArticleItem articleItem )
	{
		if ( mHistoryManager != null && articleItem != null && !articleItem.isLocked())
		{
			mHistoryManager.addWord(articleItem);
		}
	}

	@Override
	public void swipe(boolean leftToRight) {
		ArticleItem item = mArticle.stack.getCurrentItem();
		ShowArticleOptions options = mArticle.stack.getCurrentShowArticleOptions();
		super.swipe(leftToRight);
		checkBack(item, options);
	}

	protected HtmlBuilderParams.Builder createHtmlBuilderParamsBuilder()
	{
		mArticle.scale = mArticleManager.getArticleScale();
		return super.createHtmlBuilderParamsBuilder().setHorizontalPadding(8f);
	}

	@Override
	public void nextTranslation(Context context, int listId, int globalIdx, @Nullable String label) {
		mArticle.scale = mArticleManager.getArticleScale();
		ArticleItem currentItem = mArticle.stack.getCurrentItem();
		if ((mArticleSearcher != null) && (currentItem != null)) {
			ArticleItem next =
					mArticleSearcher.find(currentItem.getDictId(), listId, globalIdx, label);
			if (next != null) {
				if (isAdditional(next)) {
					mArticleManager.showArticleActivity(next, ArticleControllerType.OALD_CONTROLLER_ID_ADDITIONAL_INFO, context);
				} else {
					nextTranslation(next, mArticle.stack.getCurrentShowArticleOptions());
				}
			}
			checkBack(currentItem, mArticle.stack.getCurrentShowArticleOptions());
		}
	}

	@Override
	public void nextTranslation(@Nullable ArticleItem articleItem, @Nullable ShowArticleOptions showArticleOptions) {
		checkBack(articleItem, showArticleOptions);
		super.nextTranslation(articleItem, showArticleOptions);
	}

	@Override
	public void openPractisePronunciationScreenActivity(@NonNull Context context )
	{
		ArticleItem currentItem = mArticle.stack.getCurrentItem();
		if (mArticleSearcher != null && currentItem != null && currentItem.getSortKey() != null && currentItem.getDirection() !=null) {
			ArticleItem next = mArticleSearcher.findPractisePronunciationArticleItem(currentItem.getDictId(), currentItem.getSortKey(), currentItem.getDirection().getLanguageFrom());
			if (next != null) {
				mArticleManager.showArticleActivity(next, ArticleControllerType.OALD_CONTROLLER_ID_PRACTICE_PRONUNCIATION, context);
			}
		}
	}

	@Override
	public void openPractisePronunciationScreen(@NonNull Context context )
	{
		ArticleItem currentItem = mArticle.stack.getCurrentItem();
		if (mArticleSearcher != null && currentItem != null && currentItem.getSortKey() != null && currentItem.getDirection() !=null) {
			ArticleItem next = mArticleSearcher.findPractisePronunciationArticleItem(currentItem.getDictId(), currentItem.getSortKey(), currentItem.getDirection().getLanguageFrom());
			if (next != null) {
				mArticleManager.showArticle(next, null, ArticleControllerType.OALD_CONTROLLER_ID_PRACTICE_PRONUNCIATION, context);
			}
		}
	}

	@NonNull
	@Override
	public ButtonState getRunningHeadsButtonState(boolean leftToRight)
	{
		VisibilityState vis = VisibilityState.gone;
		if (mArticle.stack.getCurrentShowArticleOptions() != null && !mArticle.isLocked
				&& !ArticleControllerType.OALD_CONTROLLER_ID_SHARE.equals(mControllerId))
		{
			if ( getRunningHeadsHeader(leftToRight) != null )
			{
				vis = VisibilityState.enabled;
			}
		}
		return new ButtonState(vis, CheckState.uncheckable);
	}

	@Override
	public void setArticleScale(final float newScale) {
		final float newArticleScale = Math.min(ApplicationSettings.getMaxArticleScale(),
											   Math.max(ApplicationSettings.getMinArticleScale(), newScale));
		changeState(new Runnable() {
			@Override
			public void run() {
				mArticle.scale = newArticleScale;
			}
		});
	}

	@Override
	public boolean isNeedCrossRef() {
		return super.isNeedCrossRef()
				&& !ArticleControllerType.OALD_CONTROLLER_ID_SHARE.equals(mControllerId)
				&& !ArticleControllerType.OALD_CONTROLLER_ID_PRACTICE_PRONUNCIATION.equals(mControllerId);
	}

	@Override
	protected boolean isRemoveBodyMargin() {
		return ArticleControllerType.OALD_CONTROLLER_ID_PRACTICE_PRONUNCIATION.equals(mControllerId);
	}

	@Override
	public void addToFavorites(FragmentManager fragmentManager) {
		if (mFavoritesManager != null && mArticle.stack.getCurrentItem() != null) {
			mFavoritesManager.showAddArticleInDirectoryScreen(fragmentManager, mArticle.stack.getCurrentItem());
		}
	}

	@NonNull
	@Override
	public ButtonState getSearchUIButtonState() {
		if (getDemoBannerVisibility() != VisibilityState.gone)
			return new ButtonState(VisibilityState.gone, CheckState.unchecked);
		return super.getSearchUIButtonState();
	}

	@NonNull
	@Override
	public ButtonState getFavoritesButtonState() {
		if (getDemoBannerVisibility() != VisibilityState.gone)
			return new ButtonState(VisibilityState.gone, CheckState.unchecked);
		return super.getFavoritesButtonState();
	}

	@NonNull
	@Override
	public ButtonState getGoToHistoryButtonState() {
		if (getDemoBannerVisibility() != VisibilityState.gone)
			return new ButtonState(VisibilityState.gone, CheckState.unchecked);
		return super.getGoToHistoryButtonState();
	}

	@Override
	public void playSoundOnline( final String soundBaseIdx, final String soundKey )
	{
		if ( mArticle.stack.getCurrentItem() != null )
		{
			final Dictionary.DictionaryId dictionaryId = mArticle.stack.getCurrentItem().getDictId();
			Runnable hintOkRunnable = new Runnable()
			{
				@Override
				public void run()
				{
					if ( mSoundManager != null )
					{
						mSoundManager.playSoundOnline(dictionaryId, convertOald10SoundBase(soundBaseIdx), soundKey);
					}
				}
			};
			if ( mHintManager != null && mHintManager.isNeedToShowHint(HintType.AudioOnlineStreaming) )
			{
				sendRequestToShowHint(HintType.AudioOnlineStreaming, new HintParams.Builder().setOnFirstActionRunnable(hintOkRunnable).build());
			}
			else
			{
				hintOkRunnable.run();
			}
		}
	}

	@Override
	public void playSound(String soundBaseIdx, String soundKey)
	{
		super.playSound(convertOald10SoundBase(soundBaseIdx), soundKey);
	}

	@Override
	public void back() {
		if (mBackItem != null) {
			nextTranslation(mBackItem.first, mBackItem.second);
			mBackItem = null;
		}
	}

	@Override
	public boolean isNeedReturnPreview() {
		return mBackItem != null && !getFullBaseStatus(mBackItem.first.getDictId()) && mArticle.stack.getCurrentItem() != null && !mArticle.isLocked;
	}

	private void checkBack(@Nullable ArticleItem articleItem, @Nullable ShowArticleOptions articleOptions) {
		if (articleItem != null && articleItem.isLocked()) {
			mBackItem = new Pair<>(articleItem, articleOptions);
		} else {
			mBackItem = null;
		}

	}

	@Override
	public boolean isExternalBaseDownloaded(@Nullable String baseId) {
		if (baseId != null) {
			return super.isExternalBaseDownloaded(convertOald10SoundBase(baseId));
		} else {
			return false;
		}
	}

	@Override
	public void openMyDictionariesUI( Activity activity )
	{
		ArticleItem articleItem = mArticle.stack.getCurrentItem();
		if ( ( !mTransitionState.translation.mustDisableArticleButtons() ) && ( mDictionaryManager != null ) && ( articleItem != null ) )
		{
			mDictionaryManager.openMyDictionariesUI(activity, articleItem.getDictId());
		}
	}

	@Override
	public boolean isNeedOpenPictures()
	{
		return mArticle.stack.getCurrentItem() != null ? !mArticle.stack.getCurrentItem().isAdditional() : super.isNeedOpenPictures();
	}
}
