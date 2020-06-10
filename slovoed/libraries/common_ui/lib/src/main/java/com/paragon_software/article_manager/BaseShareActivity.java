package com.paragon_software.article_manager;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.SearchManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.google.android.material.tabs.TabLayout;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;
import android.text.TextUtils;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.view.animation.AnimationUtils;
import android.widget.ImageSwitcher;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.analytics_manager.events.ShareLaunch;
import com.paragon_software.article_manager.controllers.TabsViewController;
import com.paragon_software.article_manager.controllers.WordsViewController;
import com.paragon_software.article_manager.dialog.DictionariesFetchDialogFragment;
import com.paragon_software.article_manager.dialog.DictionariesMissingFullBaseDialogFragment;
import com.paragon_software.article_manager.dialog.DictionariesMissingPurchasesDialogFragment;
import com.paragon_software.common_ui.R;
import com.paragon_software.dictionary_manager.Language;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.sothree.slidinguppanel.SlidingUpPanelLayout;

import java.lang.ref.WeakReference;

public abstract class BaseShareActivity extends BaseArticleActivity
		implements ShareControllerAPI.OnDictionariesFetchUIVisibilityChangedListener,
		ShareControllerAPI.OnDictionariesMissingPurchasesUIVisibilityListener,
		ShareControllerAPI.OnDictionariesMissingFullBaseUIVisibilityListener,
		DictionariesFetchDialogFragment.Actions,
		DictionariesMissingFullBaseDialogFragment.Actions,
		DictionariesMissingPurchasesDialogFragment.Actions {

	static final String TAG_DICTIONARIES_FETCH_UI = BaseShareActivity.class.getCanonicalName() + ".DICTIONARIES_FETCH_UI";
	static final String TAG_DICTIONARIES_MISSING_PURCHASES_UI = BaseShareActivity.class.getCanonicalName() + ".DICTIONARIES_MISSING_PURCHASES_UI";
	static final String TAG_DICTIONARIES_MISSING_FULL_BASE_UI = BaseShareActivity.class.getCanonicalName() + ".DICTIONARIES_MISSING_FULL_BASE_UI";
	static final int REQUEST_CODE_SHARE_PENDING_INTENT = BaseShareActivity.class.getCanonicalName().hashCode();

	private ShareControllerAPI mShareController;
	@Nullable
	private ImageSwitcher mSliderChevronImageSwitcher;
	@Nullable
	protected SlidingUpPanelLayout mSlidingPanelLayout;
	@Nullable
	protected WordsViewController mWordsViewController;
	@Nullable
	protected String mInitiallyQuery;
	@Nullable
	private TabsViewController mTabsViewController;
	@Nullable
	private CollectionView.OnItemRangeChanged mOnWordsCollectionViewChanged = new WordsCollectionViewListener();

	@Override
	protected void onCreate(@Nullable Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if (null == savedInstanceState) {
			AnalyticsManagerAPI.get().logEvent(new ShareLaunch());
		}
		String query = getQuery(getIntent());
		if (TextUtils.isEmpty(query)) {
			showEmptyQueryToast();
			finish();
			return;
		}

		mInitiallyQuery = query;
		Bundle b = getIntent().getExtras();
		String controllerId = getControllerType();
		if (b != null) {
			String extraControllerId = b.getString(ArticleManagerAPI.EXTRA_CONTROLLER_ID);
			controllerId = null == extraControllerId ? controllerId : extraControllerId;
		}
		if (savedInstanceState == null) {
			Fragment articleFragment = getArticleFragment();
			if (b == null) {
				b = new Bundle(1);
			}
			b.putString(ArticleManagerAPI.EXTRA_CONTROLLER_ID, controllerId);
			articleFragment.setArguments(b);
			getSupportFragmentManager().beginTransaction().add(R.id.article_fragment, articleFragment).commit();
		}

		ArticleManagerAPI articleManager = ArticleManagerHolder.getManager();
		if (articleManager != null) {
			if (articleManager.haveNotShownSplashScreens()) {
				articleManager.showSplashScreen(this, createPendingIntent(mInitiallyQuery, controllerId));
				finish();
			}
			setArticleController(articleManager.getArticleController(controllerId));
			getArticleController().setActive();
			setShareController(articleManager.getShareController(controllerId));
		} else {
			throw new IllegalStateException("Cant initialize Share Activity without ArticleManagerAPI");
		}

	}

    @Override
    protected void onPostCreate(@Nullable Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        if(savedInstanceState == null) {
            ShareControllerAPI shareController = getShareController();
            shareController.hideMissingFullBaseUI();
            shareController.hideMissingPurchasesUI();
            shareController.showDictionariesFetchUI();
            shareController.setInitiallyQuery(mInitiallyQuery);
		}
    }

    protected abstract Fragment getArticleFragment();

	protected abstract String getControllerType();

	/**
	 * Abstract methods for getting ids of views
	 */
	protected abstract int getSliderChevronImageSwitcherId();

	protected abstract int getSlidingPanelId();

	protected abstract int getSlidingPanelInnerId();

	protected abstract int getQueryTextViewId();

	protected abstract int getTabLayoutId();

	protected abstract int getNoResultTextViewId();

	@NonNull
	private PendingIntent createPendingIntent(String query, String controllerId) {
		Intent intent = new Intent(this, ShareActivity.class);
		intent.putExtra(ArticleManagerAPI.EXTRA_CONTROLLER_ID, controllerId);
		intent.putExtra(Intent.EXTRA_TEXT, query);
		return PendingIntent
				.getActivity(this, REQUEST_CODE_SHARE_PENDING_INTENT, intent, PendingIntent.FLAG_UPDATE_CURRENT);
	}

	protected void initSlidingPanel() {
		mSliderChevronImageSwitcher = findViewById(getSliderChevronImageSwitcherId());
		if (null != mSliderChevronImageSwitcher) {
			mSliderChevronImageSwitcher.setFactory(() -> new ImageView(mSliderChevronImageSwitcher.getContext()));
			Context context = getApplicationContext();
			mSliderChevronImageSwitcher.setInAnimation(AnimationUtils.loadAnimation(context, R.anim.fade_in));
			mSliderChevronImageSwitcher.setOutAnimation(AnimationUtils.loadAnimation(context, R.anim.fade_out));

			mSlidingPanelLayout = findViewById(getSlidingPanelId());
			if (null != mSlidingPanelLayout) {
				View slidingPanelInner = findViewById(getSlidingPanelInnerId());
				Utils.setEmptyClickListener(slidingPanelInner);
				boolean expanded = mSlidingPanelLayout.getPanelState().equals(SlidingUpPanelLayout.PanelState.EXPANDED);
				mSliderChevronImageSwitcher.setImageResource(expanded ? R.drawable.ic_collapse : R.drawable.ic_expand);
				mSlidingPanelLayout.addPanelSlideListener(new SlidingUpPanelLayout.PanelSlideListener() {
					@Override
					public void onPanelSlide(View panel, float slideOffset) {
					}

					@Override
					public void onPanelStateChanged(View panel, SlidingUpPanelLayout.PanelState previousState,
													SlidingUpPanelLayout.PanelState newState) {
						if (newState.equals(SlidingUpPanelLayout.PanelState.EXPANDED) || newState
								.equals(SlidingUpPanelLayout.PanelState.COLLAPSED)) {
							boolean up = newState.equals(SlidingUpPanelLayout.PanelState.EXPANDED);
							mSliderChevronImageSwitcher.setImageResource(up ? R.drawable.ic_collapse : R.drawable.ic_expand);
						}
					}
				});
			}
		}
	}

	protected void initWordsView() {
		TextView queryTextView = findViewById(getQueryTextViewId());
		if (null != queryTextView) {
			queryTextView.setMovementMethod(LinkMovementMethod.getInstance());
			if (!TextUtils.isEmpty(mInitiallyQuery)) {
				unregisterWordsView();
				mShareController.setInitiallyQuery(mInitiallyQuery);
				mWordsViewController = getWordsViewController(queryTextView);
				mWordsViewController.registerToCollectionView();
				mWordsViewController.populate();
			}
		}
	}

	private void unregisterWordsView() {
		if(mWordsViewController != null)
			mWordsViewController.unregisterFromCollectionView();
	}

	protected WordsViewController getWordsViewController( @NonNull TextView textView)
	{
		return new WordsViewController(getTheme(), textView, mInitiallyQuery, getShareController());
	}

	protected void initTabsView() {
		TabLayout tabLayout = findViewById(getTabLayoutId());
		if (null != tabLayout) {
			mTabsViewController = new TabsViewController(tabLayout, getShareController(), new ArticleControllerAPIWrapper(getArticleController(), this));
			mTabsViewController.registerToCollectionView();
			mTabsViewController.populate();
		}
	}


	private void updateNoResultViewVisibility() {
		final View noResult = findViewById(getNoResultTextViewId());
		if (null != noResult) {
			noResult.setVisibility(isNoResultVisible() ? View.VISIBLE : View.GONE);
		}
	}

	protected boolean isNoResultVisible() {
		boolean inProgress = getShareController().getWords().isInProgress();
		boolean empty = getShareController().getWords().getCount() <= 0;
		return empty && !inProgress;
	}

	private void updateArticleFragmentViewVisibility() {
		final View articleFragmentView = findViewById(R.id.article_fragment);
		if (null != articleFragmentView) {
			boolean empty = getShareController().getWords().getCount() <= 0;
			articleFragmentView.setVisibility(empty ? View.GONE : View.VISIBLE);
		}
	}

	@Override
	protected void onNewIntent(Intent intent) {
		super.onNewIntent(intent);
		setIntent(intent);
		String query = getQuery(intent);
		if (TextUtils.isEmpty(query)) {
			showEmptyQueryToast();
			return;
		}

		mInitiallyQuery = query;

		ShareControllerAPI shareController = getShareController();
		shareController.hideMissingFullBaseUI();
		shareController.hideMissingPurchasesUI();

		initWordsView();
	}


	protected void listenWordsCollectionView() {
		ShareControllerAPI shareController = getShareController();
		shareController.getWords().registerListener(mOnWordsCollectionViewChanged);
		updateArticleFragmentViewVisibility();
		updateNoResultViewVisibility();
	}

	void showEmptyQueryToast() {
		Toast.makeText(this, R.string.utils_slovoed_ui_common_no_result_found, Toast.LENGTH_LONG).show();
	}

	String getQuery(Intent intent) {
		String query = intent.getStringExtra(Intent.EXTRA_TEXT);
		if (TextUtils.isEmpty(query)) {
			query = intent.getStringExtra(SearchManager.QUERY);
		}
		if (null != query) {
			query = query.trim();
		}
		return query;
	}


	@Override
	protected void onResume() {
		super.onResume();
		getShareController().registerNotifier(this);
		onDictionariesFetchUIVisibilityChanged();
		onDictionariesMissingFullBaseUIVisibility();
		onDictionariesMissingPurchasesUIVisibility();
	}

	@Override
	protected void onPause() {
		super.onPause();
		getShareController().unregisterNotifier(this);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		if (null != mShareController) {
			mShareController.getWords().unregisterListener(mOnWordsCollectionViewChanged);
			mShareController.setInitiallyQuery(null);
		}
		if (null != mWordsViewController) {
			mWordsViewController.unregisterFromCollectionView();
		}
		if (null != mTabsViewController) {
			mTabsViewController.unregisterFromCollectionView();
		}
	}

	@Override
	protected void onRestoreInstanceState(Bundle savedInstanceState) {
		super.onRestoreInstanceState(savedInstanceState);
		if (null != mSlidingPanelLayout && null != mSliderChevronImageSwitcher) {
			boolean expanded = mSlidingPanelLayout.getPanelState().equals(SlidingUpPanelLayout.PanelState.EXPANDED);
			mSliderChevronImageSwitcher.setImageResource(expanded ? R.drawable.ic_collapse : R.drawable.ic_expand);
		}
	}

	@NonNull
	public ShareControllerAPI getShareController() {
		return mShareController;
	}

	public void setShareController(ShareControllerAPI mShareController) {
		this.mShareController = mShareController;
	}


	@Nullable
	protected DialogFragment findDialogFragment(String tag) {
		Fragment fragment = getSupportFragmentManager().findFragmentByTag(tag);
		DialogFragment dialogFragment = null;
		if (null != fragment && fragment instanceof DialogFragment) {
			dialogFragment = (DialogFragment) fragment;
		}
		return dialogFragment;
	}

	protected void showDialogFragment(DialogFragment dialogFragment, String tag) {
		if (!isDialogFragmentVisible(dialogFragment)) {
			dialogFragment.show(getSupportFragmentManager(), tag);
		}
	}

	protected void dismissDialogFragment(@Nullable DialogFragment dialogFragment) {
		if (isDialogFragmentVisible(dialogFragment)) {
			dialogFragment.dismiss();
		}
	}

	@Override
	public abstract void onDictionariesMissingPurchasesUIVisibility();

	@Override
	public abstract void onDictionariesMissingFullBaseUIVisibility();

	@Override
	public abstract void onDictionariesFetchUIVisibilityChanged();

	// Issue with method DialogFragment.isVisible()
	// Solution from: https://stackoverflow.com/a/24412988
	private static boolean isDialogFragmentVisible(DialogFragment dialogFragment) {
		return dialogFragment != null && dialogFragment.getDialog() != null && dialogFragment.getDialog().isShowing()
				&& !dialogFragment.isRemoving();
	}

	@Override
	public void onCancelUIDictionariesFetch() {
		getShareController().hideDictionariesFetchUI();
		finish();
	}

	@Override
	public void onOkUIMissingFullBase() {
		getShareController().hideMissingFullBaseUI();
		finish();
	}

	@Override
	public void onOkUIMissingPurchases() {
		getShareController().hideMissingPurchasesUI();
		finish();
	}

	protected CharSequence languageToString(Language from, Language to) {
		//TODO: USE_LOCALIZED_LANGUAGE_STRINGS_FROM_PDAHPC
		if (from.equals(to)) {
			return from.getFullForm();
		} else {
			return getString(R.string.article_manager_ui_share_switch_direction_two_lang_template, from.getFullForm(), to.getFullForm());
		}
	}

	private static class Utils {
		static void setEmptyClickListener(@Nullable View view) {
			if (null != view) {
				view.setOnClickListener(view1 -> {
				});
			}
		}
	}

	public static class ArticleControllerAPIWrapper {
		private final ArticleControllerAPI mArticleController;
		@NonNull
		private final WeakReference<Activity> mActivity;

		ArticleControllerAPIWrapper(@NonNull ArticleControllerAPI articleController, @NonNull Activity activity) {

			this.mArticleController = articleController;
			this.mActivity = new WeakReference<>(activity);
		}

		public void toggleSearchUI(boolean show) {
			mArticleController.toggleSearchUI(show);
			Activity activity = mActivity.get();
			if (null != activity) {
				KeyboardHelper.hideKeyboard(activity.getCurrentFocus());
			}
		}
	}

	class WordsCollectionViewListener implements CollectionView.OnItemRangeChanged, CollectionView.OnProgressChanged {
		@Override
		public void onItemRangeChanged(CollectionView.OPERATION_TYPE type, int startPosition, int itemCount) {
			updateArticleFragmentViewVisibility();
		}

		@Override
		public void onProgressChanged() {
			updateNoResultViewVisibility();
			supportInvalidateOptionsMenu();
		}
	}
}
