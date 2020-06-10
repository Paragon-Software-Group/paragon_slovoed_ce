package com.paragon_software.article_manager;

import android.graphics.Bitmap;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.constraintlayout.widget.ConstraintSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Checkable;
import android.widget.ImageView;
import android.widget.TextView;

import com.paragon_software.article_manager_ui.R;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;
import com.paragon_software.utils_slovoed_ui.StringsUtils;

import java.util.Arrays;
import java.util.EnumSet;
import java.util.Set;

public class ArticleFragment extends BaseArticleFragment
		implements View.OnClickListener, OnDictionaryTitleChangedListener, OnDictionaryIconChangedListener,
		OnDemoBannerVisibilityChangedListener, OnBackButtonStateChangedListener,
		OnForwardButtonStateChangedListener, OnOpenMyDictionariesVisibilityChangedListener,
		OnTrialStatusVisibilityChangedListener, OnTrialLengthChangedListener {


	private ImageView mDictIcon;
	private TextView mDictTitleSimple, mDictTitleDemo, mDictPurchaseLabel, mDownloadFullBaseLabel, mTrialStatusLabel;
	private Button mBackButton, mForwardButton, mBuyButton, mOpenMyDictionaries;

	@Override
	@Nullable
	public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
							 @Nullable Bundle savedInstanceState) {
		View view = inflater.inflate(R.layout.fragment_article, container, false);
		mDictIcon = view.findViewById(R.id.dict_icon);
		mDictTitleSimple = view.findViewById(R.id.dict_title_simple);
		mDictTitleDemo = view.findViewById(R.id.dict_title_demo);
		mDictPurchaseLabel = view.findViewById(R.id.dict_purchase_description);
		mDownloadFullBaseLabel = view.findViewById(R.id.download_full_base_description);
		mTrialStatusLabel = view.findViewById(R.id.dict_trial_status);
		mBackButton = view.findViewById(R.id.back_button);
		mForwardButton = view.findViewById(R.id.forward_button);
		mBuyButton = view.findViewById(R.id.buy);
		mOpenMyDictionaries = view.findViewById(R.id.go_to_my_dictionaries_button);
		mProgressBarBackground = view.findViewById(R.id.progress_bar_background);
		mProgressBar = view.findViewById(R.id.progress_bar);
		mArticleContentFrame = view.findViewById(R.id.article_content_frame);

		for (Button button : new Button[]{mBackButton, mForwardButton, mBuyButton, mOpenMyDictionaries}) {
			button.setOnClickListener(this);
		}

		if (null != mController) {
			applyVisibilityState(mDictIcon, mController.getDictionaryIconVisibility());
			applyVisibilityState(mDictTitleSimple, mController.getDictionaryTitleVisibility());
		}
		updateGraySpaceVisibility();

		super.onCreateView(inflater, container, savedInstanceState);
		return view;
	}

	@Override
	public void onResume() {
		super.onResume();
		onDictionaryTitleChanged();
		onDictionaryIconChanged();
		onDemoBannerVisibilityChanged();
		onOpenMyDictionariesVisibilityChanged();
		onTrialStatusVisibilityChanged();
		onTrialLengthChanged();
		onBackButtonStateChanged();
		onForwardButtonStateChanged();
	}

	@Override
	public void onClick(View view) {
		int id = view.getId();
		if (id == R.id.buy) {
			mController.buy(getActivity());
		} else if (id == R.id.go_to_my_dictionaries_button) {
			mController.openMyDictionariesUI(getActivity());
		} else if (id == R.id.back_button) {
			mController.back();
		} else if (id == R.id.forward_button) {
			mController.forward();
		}
	}

	@Override
	public void onDictionaryTitleChanged() {
		String title = mController.getDictionaryTitle();
		if (title == null) {
			title = "";
		}
		mDictTitleSimple.setText(title);
		mDictTitleDemo.setText(title);
	}

	@Override
	public void onDictionaryIconChanged() {
		Bitmap bmp = mController.getDictionaryIcon();
		mDictIcon.setImageBitmap(bmp);
	}

	@Override
	public void onOpenMyDictionariesVisibilityChanged() {
		VisibilityState v = mController.getOpenMyDictionariesVisibility();
		mOpenMyDictionaries.setEnabled(v == VisibilityState.enabled);
		mOpenMyDictionaries.setVisibility(v != VisibilityState.gone ? View.VISIBLE : View.GONE);
		mDownloadFullBaseLabel.setVisibility(v != VisibilityState.gone ? View.VISIBLE : View.GONE);
		updateDictPurchaseLabel();
		updateGraySpaceVisibility();
	}

	@Override
	public void onTrialStatusVisibilityChanged() {
		VisibilityState trialVisibility = mController.getTrialStatusVisibility();
		mTrialStatusLabel.setVisibility((trialVisibility != VisibilityState.gone) ? View.VISIBLE : View.GONE);
		updateDictPurchaseLabel();
		updateGraySpaceVisibility();
	}

	@Override
	public void onTrialLengthChanged() {
		mTrialStatusLabel.setText(StringsUtils.createTrialLengthString(getContext(), mController.getTrialLengthInMinutes()));
		updateGraySpaceVisibility();
	}

	@Override
	public void onDemoBannerVisibilityChanged() {
		View view = getView();
		VisibilityState v = mController.getDemoBannerVisibility();
		mBuyButton.setEnabled(v == VisibilityState.enabled);
		if (view instanceof ConstraintLayout) {
			boolean demo = (v != VisibilityState.gone);
			ConstraintSet constraintSet = new ConstraintSet();
			constraintSet.clone((ConstraintLayout) view);
			int titleDefaultVisibility =
					mController.getDictionaryTitleVisibility().equals(VisibilityState.gone) ? View.GONE : View.VISIBLE;
			constraintSet.setVisibility(R.id.dict_title_simple, demo ? View.GONE : titleDefaultVisibility);
			constraintSet.setVisibility(R.id.dict_title_demo, demo ? View.VISIBLE : View.GONE);
			constraintSet.setVisibility(R.id.buy, demo ? View.VISIBLE : View.GONE);
			constraintSet.connect(R.id.dict_icon, ConstraintSet.RIGHT,
					demo ? R.id.icon_right_guideline_large : R.id.icon_right_guideline_small,
					ConstraintSet.LEFT);
			constraintSet.connect(R.id.dict_icon, ConstraintSet.TOP, demo ? R.id.icon_top_margin : R.id.barrier1,
					ConstraintSet.BOTTOM);
			constraintSet.applyTo((ConstraintLayout) view);
		}
		updateDictPurchaseLabel();
		updateGraySpaceVisibility();
	}

	@Override
	public void onBackButtonStateChanged() {
		applyButtonState(mBackButton, mController.getBackButtonState());
		updateGraySpaceVisibility();
	}

	@Override
	public void onForwardButtonStateChanged() {
		applyButtonState(mForwardButton, mController.getForwardButtonState());
		updateGraySpaceVisibility();
	}

	private void updateDictPurchaseLabel() {
		boolean trial = (mController.getTrialStatusVisibility() != VisibilityState.gone);
		boolean demo = (mController.getDemoBannerVisibility() != VisibilityState.gone);
		boolean needToDownload = (mController.getOpenMyDictionariesVisibility() != VisibilityState.gone);
		mDictPurchaseLabel.setVisibility((demo && !needToDownload && !trial) ? View.VISIBLE : View.GONE);
	}

	private void updateGraySpaceVisibility() {
		Set<VisibilityState> set = EnumSet.noneOf(VisibilityState.class);
		set.add(mController.getBackButtonState().getVisibility());
		set.add(mController.getForwardButtonState().getVisibility());
		set.add(mController.getDictionaryIconVisibility());
		set.add(mController.getDictionaryTitleVisibility());
		set.add(mController.getTrialStatusVisibility());
		set.add(mController.getDemoBannerVisibility());
		set.add(mController.getOpenMyDictionariesVisibility());
		set.add(mController.getRunningHeadsButtonState(false).getVisibility());
		set.add(mController.getRunningHeadsButtonState(true).getVisibility());
		View view = getView();
		if(view instanceof ConstraintLayout) {
			ConstraintSet constraintSet = new ConstraintSet();
			constraintSet.clone((ConstraintLayout) view);
			int visible = set.equals(EnumSet.of(VisibilityState.gone)) ? View.GONE : View.VISIBLE;
			for(int spaceId : Arrays.asList(R.id.navigation_buttons_margin, R.id.icon_top_margin, R.id.article_top_margin))
				constraintSet.setVisibility(spaceId, visible);
			constraintSet.applyTo((ConstraintLayout) view);
		}
	}

	private static void applyButtonState(Button btn, ButtonState state) {
		applyVisibilityState(btn, state.getVisibility());
		CheckState checkState = state.getCheckState();
		if ((btn instanceof Checkable) && (checkState != CheckState.uncheckable)) {
			((Checkable) btn).setChecked(checkState == CheckState.checked);
		}
	}

	@Override
	void openFullImage( String imgSrcPath )
	{
		// TODO open Slovoed activity to show full image
	}
}
