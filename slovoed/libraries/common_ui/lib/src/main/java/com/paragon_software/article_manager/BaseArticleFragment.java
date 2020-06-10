package com.paragon_software.article_manager;

import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import android.text.TextUtils;
import android.util.Pair;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.Checkable;

import com.paragon_software.common_ui.R;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.utils_slovoed.gestures.GestureSplitter;
import com.paragon_software.utils_slovoed.gestures.SwipeGestureDetector;
import com.paragon_software.utils_slovoed.gestures.SwipeListener;
import com.paragon_software.utils_slovoed.network.NetworkUtils;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;
import com.paragon_software.utils_slovoed_ui_common.fragments.SimpleDialog;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.Arrays;
import java.util.Map;
import java.util.TreeMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.CompositeDisposable;

public abstract class BaseArticleFragment extends Fragment
		implements View.OnTouchListener,
        OnArticleDataChangedListener,
        OnArticleVisibilityChangedListener,
        OnArticleScaleChangedListener,
        OnMyViewSettingsChangedListener,
		SwipeListener,
		SimpleDialog.Target {

	private static final String NEED_FULL_IMAGE_BASE_DIALOG_TAG = "NEED_FULL_IMAGE_BASE_DIALOG_TAG";

	private static final Pattern SLD_REF_PATTERN = Pattern.compile("([a-z]{4}):(\\d+):(.+)", Pattern.DOTALL);
	private static final Pattern SLD_LINK_PATTERN = Pattern.compile("(\\d+):(\\d+):(\\d+):(\\w*)", Pattern.DOTALL);
	private static final Pattern SLD_POPUP_PATTERN = Pattern.compile("(\\d+):(\\d+):", Pattern.DOTALL);
	private static final Pattern SLD_SOUND_PATTERN = Pattern.compile("(.+):([\\w+]{4}):(-?\\d+):(.+)", Pattern.DOTALL);
	protected static final Pattern SLD_EXTERNAL_BASE_IMAGE_PATTERN = Pattern.compile("([\\w+]{4}):(\\d+):(.+)", Pattern.DOTALL);

	protected static final Pattern SLD_LINK_IDIOMS_ANCHOR_PATTERN = Pattern.compile("(.+)_idm(.*)_(.+)", Pattern.DOTALL);
	protected static final Pattern SLD_LINK_PHRASAL_VERBS_ANCHOR_PATTERN = Pattern.compile("(.+)_pv(.*)_(.+)", Pattern.DOTALL);

	@NonNull
	private static final Map<String, WebViewWrapper> mWebViewWrappers = new TreeMap<>();

	private CompositeDisposable mCompositeDisposable = new CompositeDisposable();

	private String mControllerId;
	private String mCurrentArticleData;
	protected ArticleControllerAPI mController;
	protected boolean mWebViewLoading = false;
	protected WebViewWrapper mWebViewWrapper = null;
	protected View mProgressBarBackground, mProgressBar;
	protected ViewGroup mArticleContentFrame;

	protected GestureSplitter mGestureSplitter = new GestureSplitter();
	private boolean isWebScroll;
	protected GestureDetector mSwipeGestureDetector;
	protected ScaleGestureDetector mScaleGestureDetector;
	protected final ScaleGestureDetector.OnScaleGestureListener mScaleGestureListener
			= new ScaleGestureDetector.OnScaleGestureListener() {
		private float mTotalScaleFactor;
		private float initialArticleScale;

		@Override
		public boolean onScaleBegin(ScaleGestureDetector detector) {
			mTotalScaleFactor = 0;
			initialArticleScale = mController.getArticleScale();
			return true;
		}

		@Override
		public boolean onScale(ScaleGestureDetector detector) {
			mTotalScaleFactor += detector.getScaleFactor() - 1;
			mController.setArticleScale(initialArticleScale + mTotalScaleFactor);
			return true;
		}

		@Override
		public void onScaleEnd(ScaleGestureDetector detector) {
			mController.saveState();
		}
	};

	@Override
	public void onCreate(@Nullable Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mScaleGestureDetector = new ScaleGestureDetector(getContext(), mScaleGestureListener);
		mSwipeGestureDetector = new SwipeGestureDetector(getContext(), this);
		Bundle b = getArguments();
		if (b != null) {
			ArticleManagerAPI articleManager = ArticleManagerHolder.getManager();
			String controllerId = b.getString(ArticleManagerAPI.EXTRA_CONTROLLER_ID);
			if ((articleManager != null) && (controllerId != null)) {
				mControllerId = controllerId;
				mController = articleManager.getArticleController(controllerId);
				DictionaryManagerAPI dictionaryManager = mController.getDictionaryManager();
				if (dictionaryManager != null)
					dictionaryManager.loadOnlineDictionaryStatusInformation(this);
			}
		}
	}

	@Override
	@Nullable
	public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
							 @Nullable Bundle savedInstanceState) {
		if (mControllerId != null) {
			mWebViewWrapper = mWebViewWrappers.get(mControllerId);
			if (mWebViewWrapper == null) {
				mWebViewWrappers.put(mControllerId, mWebViewWrapper = new WebViewWrapper(inflater.getContext().getApplicationContext()));
			} else {
				ViewGroup parent = (ViewGroup) mWebViewWrapper.getView().getParent();
				if (parent != null) {
					parent.removeView(mWebViewWrapper.getView());
				}
			}
		} else
			mWebViewWrapper = new WebViewWrapper(inflater.getContext().getApplicationContext());
		mWebViewWrapper.getView().setVisibility(View.GONE);
		mArticleContentFrame.addView(mWebViewWrapper.getView());
		mWebViewWrapper.updateSearchPosition();
		mWebViewWrapper.getView().setWebViewClient(new ArticleWebViewClient());
		mWebViewWrapper.getView().setWebChromeClient(new ArticleWebChromeClient());
		mWebViewWrapper.getView().getSettings().setCacheMode(WebSettings.LOAD_NO_CACHE);
		mWebViewWrapper.getView().setLayerType(View.LAYER_TYPE_HARDWARE, null);
		mWebViewWrapper.getView().setOnTouchListener(this);
		mWebViewWrapper.getView().getViewTreeObserver().addOnScrollChangedListener(() -> isWebScroll = true);

		if(savedInstanceState != null) {
			mCurrentArticleData = mController.getArticleData();
		}

		return null;
	}

	@Override
	public void onResume() {
		super.onResume();
		mController.registerNotifier(this);
		mController.registerSearchTarget(mWebViewWrapper);
		mController.setPlaySoundChannel(BaseArticleFragment.class.getSimpleName());
		if((mCurrentArticleData == null || !mCurrentArticleData.equals(mController.getArticleData()))
				&& !TextUtils.isEmpty(mController.getArticleData()))
		{
            onArticleDataChanged();
        }
		onArticleVisibilityChanged();
		mCompositeDisposable.add(mController.getNeedToShowHintObservable()
                                            .observeOn(AndroidSchedulers.mainThread())
                                            .subscribe(this::onNeedToShowHint));
	}

	@Override
	public void onPause() {
		super.onPause();
		clearPlayingSoundIconsState();
		mController.unregisterSearchTarget(mWebViewWrapper);
		mController.unregisterNotifier(this);
        mCompositeDisposable.clear();
	}

	@Override
	public void onDestroyView() {
		mWebViewWrapper.getView().setWebViewClient(null);
		mWebViewWrapper.getView().setWebChromeClient(null);
		mWebViewWrapper.getView().setOnTouchListener(null);
		mWebViewWrapper.rememberSearchPosition();
		mArticleContentFrame.removeView(mWebViewWrapper.getView());
		mWebViewWrapper = null;
		mCurrentArticleData = null;
		super.onDestroyView();
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		if ((v == mWebViewWrapper.getView()) && (event != null)) {
			int action = event.getAction();
			if (action == MotionEvent.ACTION_DOWN) {
				KeyboardHelper.hideKeyboard(v);
				isWebScroll = false;
			}
		}
		if (mController.isPinchToZoomEnabled()) {
			mScaleGestureDetector.onTouchEvent(event);
		}
		boolean eventConsumed = mScaleGestureDetector.isInProgress();
		if (mGestureSplitter.isNextGestureHandlingPossible(eventConsumed) && !isWebScroll) {
			eventConsumed = mSwipeGestureDetector.onTouchEvent(event);
		}
		return eventConsumed;
	}

	@Override
	public void onArticleDataChanged() {
		mCurrentArticleData = mController.getArticleData();
		loadData(mCurrentArticleData);
	}

	public void loadData(String data) {
		mWebViewLoading = true;
		onArticleVisibilityChanged();
		mWebViewWrapper.getView().loadDataWithBaseURL("fake://", TextUtils.isEmpty(data) ? "" : data, "text/html", "UTF-8", null);
	}

	@Override
	public void onArticleScaleChanged() {
		mWebViewWrapper.getView().loadUrl("javascript:updateViewport(" + mController.getArticleScale() + ")");
	}

	private static void applyButtonState(Button btn, ButtonState state) {
		applyVisibilityState(btn, state.getVisibility());
		CheckState checkState = state.getCheckState();
		if ((btn instanceof Checkable) && (checkState != CheckState.uncheckable)) {
			((Checkable) btn).setChecked(checkState == CheckState.checked);
		}
	}

	protected static void applyVisibilityState(View view, VisibilityState v) {
		view.setEnabled(v == VisibilityState.enabled);
		view.setVisibility((v != VisibilityState.gone) ? View.VISIBLE : View.GONE);
	}

	@Override
	public void onArticleVisibilityChanged() {
		VisibilityState v = mController.getArticleVisibility();
		if ((v == VisibilityState.enabled) && mWebViewLoading) {
            v = VisibilityState.disabled;
		}
		applyVisibilityState(mWebViewWrapper.getView(), v);
		int progressBarVisible = ((v == VisibilityState.enabled) ? View.GONE : View.VISIBLE);
		mProgressBarBackground.setVisibility(progressBarVisible);
		mProgressBar.setVisibility(progressBarVisible);
	}

	private static String cutHeadAndTail(String textValue, int offset) {
		String res = textValue;
		int len = textValue.length();
		if ((offset >= 0) && (offset < len)) {
			while ((!Character.isLetterOrDigit(textValue.charAt(offset))) && ((offset + 1) < len)) {
				offset++;
			}
			while ((!Character.isLetterOrDigit(textValue.charAt(offset))) && ((offset - 1) >= 0)) {
				offset--;
			}
			if (Character.isLetterOrDigit(textValue.charAt(offset))) {
				int begin = offset, end = offset;
				while ((begin - 1) >= 0) {
					if (Character.isLetterOrDigit(textValue.charAt(begin - 1))) {
						begin--;
					} else {
						break;
					}
				}
				while ((end + 1) < len) {
					if (Character.isLetterOrDigit(textValue.charAt(end + 1))) {
						end++;
					} else {
						break;
					}
				}
				res = textValue.substring(begin, end + 1);
			}
		}
		return res;
	}

	private void closeSearch() {
		mController.toggleSearchUI(false);
		Activity activity = getActivity();
		if (null != activity) {
			KeyboardHelper.hideKeyboard(activity.getCurrentFocus());
		}
	}

	@Override
	public void onSwipe(boolean leftToRight) {
		mController.swipe(leftToRight);
	}

	abstract void openFullImage( String imgSrcPath );

	private void showNeedDownloadPicturesBaseDialog()
	{
		SimpleDialog.show(this,
				NEED_FULL_IMAGE_BASE_DIALOG_TAG,
				new SimpleDialog.Builder()
					.setMessage(getString(R.string.article_manager_ui_download_pictures_base_description))
					.setPositiveText(getString(R.string.utils_slovoed_ui_common_yes))
					.setNegativeText(getString(R.string.utils_slovoed_ui_common_no)));
	}

	protected void onArticleDisplayed() {
	}

	protected void clearPlayingSoundIconsState() {
		mWebViewWrapper.getView().loadUrl("javascript:clearSoundImageStates()");
	}

	protected void showConnectionUnavailableDialog() {
	}

	protected @Nullable String getPhraseHighlightColor()
	{
		return null;
	}

	protected @Nullable String getQueryHighlightColor()
	{
		return null;
	}

	protected @NonNull String getJSHighlightColorParameters()
	{
		StringBuilder colorParameters = new StringBuilder();
		String phraseColor = getPhraseHighlightColor();
		String queryColor = getQueryHighlightColor();
		if ( phraseColor != null || queryColor != null)
		{
			colorParameters.append(", '");
			colorParameters.append(phraseColor == null ? "khaki" : phraseColor);
			colorParameters.append("', '");
			colorParameters.append(queryColor == null ? "tomato" : queryColor);
			colorParameters.append("'");
		}
		return colorParameters.toString();
	}

	@Override
	public void onMyViewSettingsChanged()
	{
		String anchor = mController.getArticleFtsAnchor();
		updatePageAccordingMyViewSettings(mWebViewWrapper.getView(), !TextUtils.isEmpty(anchor));
	}

	private void updatePageAccordingMyViewSettings( @NonNull WebView view, boolean withScroll )
	{
		// if article with scroll then disable 'My View' settings
		boolean myViewEnabled = mController.isMyViewEnabled() && !withScroll;
		runJSActivateAppSettingsSwitch(view, "phonetics", myViewEnabled && mController.isHidePronunciations());
		runJSActivateAppSettingsSwitch(view, "examples", myViewEnabled && mController.isHideExamples());
		runJSActivateAppSettingsSwitch(view, "images", myViewEnabled && mController.isHidePictures());
		runJSActivateAppSettingsSwitch(view, "idioms", myViewEnabled && mController.isHideIdioms());
		runJSActivateAppSettingsSwitch(view, "phrase", myViewEnabled && mController.isHidePhrasalVerbs());
	}

	private void runJSActivateAppSettingsSwitch( @NonNull WebView view, @NonNull String thematic, boolean isHide )
	{
		view.loadUrl("javascript:sld2_activateAppSettingsSwitch(\"" + thematic + "\", "
							 + (isHide ? "1" : "0") + " )");
	}

	protected boolean showSldSoundHintDialog(String soundKey, String soundBaseIdx, int soundIdx, String soundLang,
											 @Nullable HintParams hintParams )
	{
		switch ( soundLang )
		{
			case "enUK":
				return showHint(HintType.AudioBritish, hintParams);
			case "enUS":
				return showHint(HintType.AudioAmerican, hintParams);
			case "enWR":
				return showHint(HintType.AudioWorldEnglish, hintParams);
			default:
				return false;
		}
	}

	protected boolean showSldLinkHintDialog(String value, int listId, int globalIndex, String anchor)
	{
		if ( anchor != null )
		{
			if ( SLD_LINK_IDIOMS_ANCHOR_PATTERN.matcher(anchor).matches() )
			{
				return showHint(HintType.Idioms, null);
			}
			else if ( SLD_LINK_PHRASAL_VERBS_ANCHOR_PATTERN.matcher(anchor).matches() )
			{
				return showHint(HintType.PhrasalVerbs, null);
			}
		}
		return false;
	}

	private boolean showHint( @NonNull HintType hintType, @Nullable HintParams hintParams )
    {
      return mController.showHintManagerDialog(hintType, getFragmentManager(), hintParams);
    }

    protected void onNeedToShowHint( @Nullable Pair<HintType, HintParams> hintTypeHintParamsPair )
    {
      if ( hintTypeHintParamsPair!= null )
      {
        showHint(hintTypeHintParamsPair.first, hintTypeHintParamsPair.second);
      }
    }

	private class ArticleWebViewClient extends WebViewClient {
		@Override
		public boolean shouldOverrideUrlLoading(WebView view, String url) {
			closeSearch();
			Uri uri = Uri.parse(url);
			String scheme = uri.getScheme();
			String body = uri.toString().replaceAll(scheme + ":", "");
			if ("sld-ref".equals(scheme) && (!TextUtils.isEmpty(body))) {
				if(!mController.isNeedCrossRef())return true;
				Matcher matcher = SLD_REF_PATTERN.matcher(body);
				if (matcher.matches()) {
					String langCode = matcher.group(1);
					String offset = matcher.group(2);
					String textValue = matcher.group(3);
					try {
						String newWord = URLDecoder.decode(textValue, "UTF-8");
						newWord = cutHeadAndTail(newWord, Integer.valueOf(offset));
						showHint(HintType.JumpToEntry, null);
						String finalNewWord = newWord;
						new Handler().postDelayed(() -> mController.nextTranslation(finalNewWord, localeFromString(langCode)),100);
					}
					catch (UnsupportedEncodingException ignore) { }
				}
			} else if (("sld-link".equals(scheme)) && (!TextUtils.isEmpty(body))) {
				Matcher matcher = SLD_LINK_PATTERN.matcher(body);
				if (matcher.matches()) {
					String value = matcher.group(1);
					int listId = Integer.valueOf(matcher.group(2));
					int globalIndex = Integer.valueOf(matcher.group(3));
					String anchor = matcher.group(4);

					showSldLinkHintDialog(value, listId, globalIndex, anchor);
                    if (!anchor.isEmpty() && mController.isCurrentArticle(listId, globalIndex)) {
                        view.loadUrl("javascript:scrollToName(\"" + anchor + "\")");
                    } else {
                        mController.nextTranslation(getContext(), listId, globalIndex, anchor.isEmpty() ? null : anchor);
                    }
				}
			} else if (("sld-popup-article".equals(scheme)) && (!TextUtils.isEmpty(body))) {
				Matcher matcher = SLD_POPUP_PATTERN.matcher(body);
				if (matcher.matches()) {
					String listId = matcher.group(1);
					String globalIndex = matcher.group(2);
                    mController.translatePopup(Integer.valueOf(listId), Integer.valueOf(globalIndex));
				}
			}
			else if("sld-sound".equals(scheme) && (!TextUtils.isEmpty(body))) {
				Matcher matcher = SLD_SOUND_PATTERN.matcher(body);
				if (matcher.matches()) {
					String soundKey = matcher.group(1);
					String soundBaseIdx = matcher.group(2);
					int soundIdx = Integer.valueOf(matcher.group(3));
					String soundLang = matcher.group(4);

					setPlayingSoundIcon(soundLang);

					showSldSoundHintDialog(soundKey, soundBaseIdx, soundIdx, soundLang, null);

					if (!soundBaseIdx.equals("0000") && !soundKey.trim().isEmpty()) {
						if (mController.isExternalBaseDownloaded(soundBaseIdx)) {
							mController.playSound(soundBaseIdx, soundKey);
						} else {
							if (!NetworkUtils.isNetworkAvailable(getContext())) {
								showConnectionUnavailableDialog();
								return true;
							}
							mController.playSoundOnline(soundBaseIdx, soundKey);
						}
					} else if (soundIdx != -1) {
						mController.playSound(soundIdx);
					}
				}
			}
			else if("sld-full-image".equals(scheme) && (!TextUtils.isEmpty(body)))
			{
				if(mController.isNeedOpenPictures()) {
					Matcher matcher = SLD_EXTERNAL_BASE_IMAGE_PATTERN.matcher(body);
					if (matcher.matches()) {
						String baseId = matcher.group(1);
						if (!mController.isExternalBaseDownloaded(baseId)) {
							showNeedDownloadPicturesBaseDialog();
							return true;
						}
					}
					openFullImage(body);
				}
			} else {
				Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
				startActivity(browserIntent);
			}

			return true;
		}

		private int localeFromString(@NonNull String locale) {
			int b1 = locale.codePointAt(3);
			int b2 = locale.codePointAt(2);
			int b3 = locale.codePointAt(1);
			int b4 = locale.codePointAt(0);
			return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
		}

		@Override
		public void onPageStarted(WebView view, String url, Bitmap favicon) {
			super.onPageStarted(view, url, favicon);
		}

		@Override
		public void onPageFinished(final WebView view, String url) {
			super.onPageFinished(view, url);

			final String anchor = mController.getArticleFtsAnchor();
			updatePageAccordingMyViewSettings(view, !TextUtils.isEmpty(anchor));
			if (null != anchor) {
				scrollByAnchor(view, anchor);
			} else {
				view.loadUrl("javascript:window.scrollTo(0,0)");
			}
			if(mWebViewLoading) {
				endLoadingAnimation();
				mWebViewLoading = false;
			}
			onArticleDisplayed();
			onArticleScaleChanged();
			onArticleVisibilityChanged();
		}
	}

	protected void scrollByAnchor(WebView view, String anchor)
	{
		view.evaluateJavascript("javascript:sld2_getTextForLabel(\"" + anchor + "\")", rawArticleText -> {
			if (rawArticleText != null && mController.isShowHighlightingEnabled()) {
				String articleText = rawArticleText.replaceAll("^\"|\"$", "");
				int[] highlightWords = mController.getQueryHighlightData(articleText);
				view.loadUrl("javascript:sld2_highlightTextForLabel(\"" + anchor + "\","
						+ Arrays.toString(highlightWords) + getJSHighlightColorParameters() + ")");
			}
			//This call should be here so that scroll errors do not appear
			view.loadUrl("javascript:scrollToName(\"" + anchor + "\")");
				}
		);
	}

	@Override
	public void onCreateSimpleDialog( @Nullable String tag, @NonNull Dialog dialog, @Nullable Bundle extra )
	{

	}

	@Override
	public void onSimpleDialogButtonPressed(@Nullable String tag, int n, @Nullable Bundle extra)
	{
		if( n == Dialog.BUTTON_POSITIVE )
		{
			if( NEED_FULL_IMAGE_BASE_DIALOG_TAG.equals(tag) )
			{
				mController.openDownloadManagerUI(getActivity());
			}
		}
	}

	protected void setPlayingSoundIcon(String soundLang) {
	}

	private class ArticleWebChromeClient extends WebChromeClient {
		@Override
		public void onProgressChanged(WebView view, int newProgress) {
			super.onProgressChanged(view, newProgress);

			if (mWebViewLoading && newProgress > 70) {
				mWebViewLoading = false;
				endLoadingAnimation();
				onArticleVisibilityChanged();
			}
		}
	}

	protected void endLoadingAnimation()
	{
	}
}
