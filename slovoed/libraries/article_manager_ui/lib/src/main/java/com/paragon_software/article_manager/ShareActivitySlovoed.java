package com.paragon_software.article_manager;

import android.content.Context;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.widget.AppCompatSpinner;
import androidx.appcompat.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.AnimationUtils;
import android.widget.AdapterView;
import android.widget.ImageSwitcher;
import android.widget.TextSwitcher;

import com.paragon_software.article_manager.adapters.ToolbarDictionariesAdapter;
import com.paragon_software.article_manager.dialog.DictionariesFetchDialogFragment;
import com.paragon_software.article_manager.dialog.DictionariesMissingFullBaseDialogFragment;
import com.paragon_software.article_manager.dialog.DictionariesMissingPurchasesDialogFragment;
import com.paragon_software.article_manager_ui.R;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

public class ShareActivitySlovoed extends BaseShareActivity
		implements ShareControllerAPI.OnDictionariesSpinnerVisibilityChangedListener,
		ShareControllerAPI.OnSwitchDirectionButtonViewStateChangedListener,
		CollectionView.OnSelectionChange,
		AdapterView.OnItemSelectedListener {

	protected static final int[] BUTTON_IDS_IN_OVERFLOW = {
			R.id.article_manager_ui_play_sound, R.id.article_manager_ui_add_to_favorites, R.id.share_go_to_dictionary,
			R.id.article_manager_ui_add_to_flashcards};

	@Nullable
	private AppCompatSpinner mSpinner;
	@Nullable
	private TextSwitcher mDirectionTextSwitcher;
	@Nullable
	private ImageSwitcher mDirectionImageSwitcher;
	@Nullable
	private CollectionView<Dictionary, Void> mDictionariesCollectionView;
	@Nullable
	private View mDirectionClickableLayout;


	private View.OnClickListener mDirectionClickListener = view -> getShareController().switchDirection();

	@Override
	protected void onCreate(@Nullable Bundle savedInstanceState) {
		setContentView(R.layout.share_activity_slovoed);
		super.onCreate(savedInstanceState);

		initToolbar();
		initSlidingPanel();
		initDirectionViews();
		initDictionariesSpinner();
		initWordsView();
		initTabsView();
		listenWordsCollectionView();
	}

	@Override
	protected Fragment getArticleFragment() {
		return new ArticleFragment();
	}

	@Override
	protected String getControllerType() {
		return ArticleControllerType.CONTROLLER_ID_SHARE;
	}

	@Override
	protected int getSliderChevronImageSwitcherId() { return R.id.handle; }

	@Override
	protected int getSlidingPanelId() { return R.id.sliding_panel; }

	@Override
	protected int getSlidingPanelInnerId() { return R.id.sliding_panel_inner; }

	@Override
	protected int getQueryTextViewId() { return R.id.share_query_text_view; }

	@Override
	protected int getTabLayoutId() { return R.id.tab_layout; }

	@Override
	protected int getNoResultTextViewId() { return R.id.share_no_result_text_view; }

	private void initDictionariesSpinner() {
		mSpinner = getDictionariesSpinner();
		ShareControllerAPI shareController = getShareController();
		mDictionariesCollectionView = shareController.getDictionaries();
		if (null != mSpinner) {
			mSpinner.setAdapter(new ToolbarDictionariesAdapter(mDictionariesCollectionView, getLayoutInflater()));
			mSpinner.setOnItemSelectedListener(this);
			int selection = mDictionariesCollectionView.getSelection();
			if ((mDictionariesCollectionView.getCount() > 0) && (0 <= selection)) {
				mSpinner.setSelection(selection);
			}

			mDictionariesCollectionView.registerListener(this);
		}
	}

	private void initDirectionViews() {
		mDirectionClickableLayout = findViewById(R.id.share_direction_clickable_layout);
		mDirectionImageSwitcher = findViewById(R.id.share_direction_image_switcher);
		mDirectionTextSwitcher = findViewById(R.id.share_direction_text_switcher);
		if (null != mDirectionImageSwitcher && null != mDirectionTextSwitcher && null != mDirectionClickableLayout) {
			mDirectionClickableLayout.setOnClickListener(mDirectionClickListener);
			mDirectionImageSwitcher.setFactory(() -> getLayoutInflater().inflate(R.layout.direction_switch_image_view, mDirectionImageSwitcher, false));
			mDirectionTextSwitcher.setFactory(() -> getLayoutInflater().inflate(R.layout.direction_switch_text_view, mDirectionTextSwitcher, false));
			Context context = getApplicationContext();
			mDirectionImageSwitcher.setInAnimation(AnimationUtils.loadAnimation(context, R.anim.fade_in));
			mDirectionImageSwitcher.setOutAnimation(AnimationUtils.loadAnimation(context, R.anim.fade_out));
			mDirectionTextSwitcher.setInAnimation(AnimationUtils.loadAnimation(context, R.anim.fade_in));
			mDirectionTextSwitcher.setOutAnimation(AnimationUtils.loadAnimation(context, R.anim.fade_out));
		}
	}

	private void initToolbar() {
		Toolbar toolbar = findViewById(R.id.share_toolbar);
		if (null != toolbar) {
			setSupportActionBar(toolbar);
		}
		ActionBar actionBar = getSupportActionBar();
		if (actionBar != null) {
			actionBar.setDisplayShowTitleEnabled(false);
			actionBar.setDisplayHomeAsUpEnabled(false);
		}
	}

	@Override
	public void onSelectionChanged() {
		int selection;
		if (null != mDictionariesCollectionView && null != mSpinner) {
			selection = mDictionariesCollectionView.getSelection();
			if (selection != mSpinner.getSelectedItemPosition()) {
				mSpinner.setSelection(selection);
			}
		}
	}

	@Override
	protected void onResume() {
		super.onResume();
		onDictionariesSpinnerVisibilityChanged();
		onSwitchDirectionButtonViewStateChanged();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		if (null != mDictionariesCollectionView) {
			mDictionariesCollectionView.unregisterListener(this);
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		for (int id : BUTTON_IDS) {
			int showAsAction = MenuItem.SHOW_AS_ACTION_ALWAYS;
			for (int id_in_overflow : BUTTON_IDS_IN_OVERFLOW) {
				if (id == id_in_overflow) {
					showAsAction = MenuItem.SHOW_AS_ACTION_NEVER;
					break;
				}
			}
			menu.add(Menu.NONE, id, Menu.NONE, "").setVisible(false).setEnabled(false).setShowAsAction(showAsAction);
		}
		return true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		boolean result = super.onPrepareOptionsMenu(menu);
		if (null != getArticleController() && null != menu) {
			VisibilityState visibility = getShareController().getDictionariesSpinnerVisibility();
			updateButton(menu, R.id.share_go_to_dictionary, new ButtonState(visibility, CheckState.uncheckable));

			if (isNoResultVisible()) {
				for (int id : BUTTON_IDS) {
					if (id != R.id.share_go_to_dictionary) {
						updateButton(menu, id, new ButtonState(VisibilityState.gone, CheckState.uncheckable));
					}
				}
			}
		}
		return result;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		boolean res = super.onOptionsItemSelected(item);
		int id = item.getItemId();
		if (id == R.id.share_go_to_dictionary) {
			getShareController().openDictionarySearch(this);
			res = true;
		}
		return res;
	}

	@Override
	public void onSearchUIVisibilityChanged() {
		supportInvalidateOptionsMenu();
		ActionBar actionBar = getSupportActionBar();
		if ((actionBar != null) && (getArticleController() != null)) {
			boolean show = !getArticleController().getSearchUIVisibility().equals(VisibilityState.gone);
			BaseSearchInArticleInputField searchField = getSearchField(actionBar);
			if (searchField != null) {
				searchField.setVisibility(show ? View.VISIBLE : View.INVISIBLE);
				if (show) {
					searchField.requestFocus();
				}
			}
		}
	}

	@Nullable
	@Override
	protected BaseArticleButtons getArticleButtons() {
		return new ArticleButtons();
	}

	@Nullable
	@Override
	protected BaseSearchInArticleInputField getSearchField(@Nullable ActionBar actionBar) {
		View view = findViewById(R.id.share_search_in_artcle_input_field);
		return view instanceof BaseSearchInArticleInputField ? (BaseSearchInArticleInputField) view : null;
	}

	@Override
	public void onDictionariesSpinnerVisibilityChanged() {
		ShareControllerAPI shareController = getShareController();
		VisibilityState visibilityState = shareController.getDictionariesSpinnerVisibility();
		View spinner = getDictionariesSpinner();
		if (null != spinner) {
			spinner.setVisibility(!VisibilityState.gone.equals(visibilityState) ? View.VISIBLE : View.INVISIBLE);
		}
		if (null != mSlidingPanelLayout) {
			int visibilitySlidingPanel = shareController.getDictionaries().getCount() > 0 ? View.VISIBLE : View.INVISIBLE;
			mSlidingPanelLayout.setVisibility(visibilitySlidingPanel);
		}
	}


	@Override
	public void onSwitchDirectionButtonViewStateChanged() {
		if (null != mDirectionImageSwitcher && null != mDirectionTextSwitcher && null != mDirectionClickableLayout) {
			ShareControllerAPI shareController = getShareController();
			DirectionButtonViewInfo buttonInfo = shareController.getSwitchDirectionButtonViewInfo();
			boolean enabled = buttonInfo.getButtonState().getVisibility().equals(VisibilityState.enabled);
			mDirectionClickableLayout.setEnabled(enabled);
			boolean visible = !buttonInfo.getButtonState().getVisibility().equals(VisibilityState.gone);
			mDirectionImageSwitcher.setVisibility(visible ? View.VISIBLE : View.GONE);
			mDirectionTextSwitcher.setVisibility(visible ? View.VISIBLE : View.GONE);
			if (visible && null != buttonInfo.getBitmap()) {
				mDirectionImageSwitcher.setImageDrawable(new BitmapDrawable(getResources(), buttonInfo.getBitmap()));
			}
			if (visible && null != buttonInfo.getLanguageFrom() && null != buttonInfo.getLanguageTo()) {
				mDirectionTextSwitcher.setText(languageToString(buttonInfo.getLanguageFrom(), buttonInfo.getLanguageTo()));
			}
		}
	}

	@Nullable
	private AppCompatSpinner getDictionariesSpinner() {
		return findViewById(R.id.dictionary_spinner);
	}

	@Override
	public void onItemSelected(AdapterView<?> adapterView, View view, int position, long id) {
		if (null != mDictionariesCollectionView) {
			Dictionary dictionary = mDictionariesCollectionView.getItem(position);
			if (null != dictionary) {
				getShareController().selectDictionary(dictionary.getId());
			}
		}
	}

	@Override
	public void onNothingSelected(AdapterView<?> adapterView) {

	}

	@Override
	public void onDictionariesMissingPurchasesUIVisibility() {
		DialogFragment dialogFragment = findDialogFragment(TAG_DICTIONARIES_MISSING_PURCHASES_UI);
		if (VisibilityState.gone != getShareController().getDictionariesMissingPurchasesUIVisibility()) {
			if (null == dialogFragment) {
				dialogFragment = new DictionariesMissingPurchasesDialogFragment();
			}
			showDialogFragment(dialogFragment, TAG_DICTIONARIES_MISSING_PURCHASES_UI);
		} else {
			dismissDialogFragment(dialogFragment);
		}
	}

	@Override
	public void onDictionariesMissingFullBaseUIVisibility() {
		DialogFragment dialogFragment = findDialogFragment(TAG_DICTIONARIES_MISSING_FULL_BASE_UI);
		if (VisibilityState.gone != getShareController().getDictionariesMissingFullBaseUIVisibility()) {
			if (null == dialogFragment) {
				dialogFragment = new DictionariesMissingFullBaseDialogFragment();
			}
			showDialogFragment(dialogFragment, TAG_DICTIONARIES_MISSING_FULL_BASE_UI);
		} else {
			dismissDialogFragment(dialogFragment);
		}
	}

	@Override
	public void onDictionariesFetchUIVisibilityChanged() {
		DialogFragment dialogFragment = findDialogFragment(TAG_DICTIONARIES_FETCH_UI);
		if (VisibilityState.gone != getShareController().getDictionariesFetchUIVisibility()) {
			if (null == dialogFragment) {
				dialogFragment = new DictionariesFetchDialogFragment();
			}
			showDialogFragment(dialogFragment, TAG_DICTIONARIES_FETCH_UI);
		} else {
			dismissDialogFragment(dialogFragment);
		}
	}
}
