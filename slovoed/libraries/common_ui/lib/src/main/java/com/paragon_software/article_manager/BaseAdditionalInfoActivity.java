package com.paragon_software.article_manager;

import android.os.Bundle;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.appcompat.app.ActionBar;

import com.paragon_software.common_ui.R;

public abstract class BaseAdditionalInfoActivity extends BaseArticleActivity
		implements OnArticleDataChangedListener {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_base_additional_info);

		Bundle b = getIntent().getExtras();
		String controllerId = null;
		if ( b != null )
		{
			ArticleManagerAPI articleManager = ArticleManagerHolder.getManager();
			controllerId = b.getString(ArticleManagerAPI.EXTRA_CONTROLLER_ID);
			if ( ( articleManager != null ) && ( controllerId != null ) )
			{
				setArticleController(articleManager.getArticleController(controllerId));
				getArticleController().setActive();
			}
		}

		if ( savedInstanceState == null )
		{
			Fragment articleFragment = getArticleFragment();
			if ( controllerId != null )
			{
				b = new Bundle(1);
				b.putString(ArticleManagerAPI.EXTRA_CONTROLLER_ID, controllerId);
				articleFragment.setArguments(getIntent().getExtras());
			}
			getSupportFragmentManager().beginTransaction().add(R.id.article_fragment, articleFragment).commit();
		}

		ActionBar actionBar = getSupportActionBar();
		if ( actionBar != null )
		{
			actionBar.setDisplayHomeAsUpEnabled(true);
			actionBar.setDisplayShowTitleEnabled(true);
		}
	}

	protected abstract Fragment getArticleFragment();

	@Override
	public void onResume()
	{
		super.onResume();
		getArticleController().registerNotifier(this);
		onArticleDataChanged();
	}

	@Override
	public void onPause()
	{
		super.onPause();
		getArticleController().unregisterNotifier(this);
	}

	@Override
	public void onSearchUIVisibilityChanged() {

	}

	@Nullable
	@Override
	protected BaseSearchInArticleInputField getSearchField(@Nullable ActionBar actionBar) {
		return null;
	}

	@Override
	protected void onDestroy() {
		Bundle b = getIntent().getExtras();
		String controllerId;
		if (b != null) {
			ArticleManagerAPI articleManager = ArticleManagerHolder.getManager();
			controllerId = b.getString(ArticleManagerAPI.EXTRA_CONTROLLER_ID);
			if ((articleManager != null) && (controllerId != null)) {
				articleManager.freeArticleController(controllerId);
				setArticleController(null);
			}
		}
		super.onDestroy();
	}

	@Override
	public boolean onSupportNavigateUp()
	{
		super.onBackPressed();
		return true;
	}

	@Override
	public void onArticleDataChanged()
	{
		ActionBar actionBar = getSupportActionBar();
		if ( actionBar != null )
		{
			actionBar.setTitle(getArticleController().getArticleShowVariantText());
		}
	}
}
