package com.paragon_software.article_manager;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.Nullable;

public class ShareActivity extends Activity {

	@Override
	protected void onCreate(@Nullable Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		ArticleManagerAPI articleManager = ArticleManagerHolder.getManager();
		if (articleManager != null && articleManager.getShareActivity() != null) {
			Intent intent = new Intent(this, articleManager.getShareActivity());
			intent.putExtra(Intent.EXTRA_TEXT, getIntent().getStringExtra(Intent.EXTRA_TEXT));
			startActivity(intent);
		}
		finish();

	}
}
