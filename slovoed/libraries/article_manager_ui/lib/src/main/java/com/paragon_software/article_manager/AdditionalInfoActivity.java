package com.paragon_software.article_manager;

import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

public class AdditionalInfoActivity extends BaseAdditionalInfoActivity {

	@Override
	protected Fragment getArticleFragment() {
		return new ArticleFragment();
	}

	@Nullable
	@Override
	protected BaseArticleButtons getArticleButtons() {
		return new ArticleButtons();
	}
}
