package com.paragon_software.odapi_ui;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.Nullable;
import android.text.TextUtils;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.native_engine.SyncEngineAPI;

import java.util.Collection;

public class StartTranslateActivity extends Activity {
    private static final int TRANSLATION_RESULT_CODE_EXACT_MATCH = 10;
    private static final int TRANSLATION_RESULT_CODE_DATABASE_IS_UNAVAILABLE = 11;
//    private static final int TRANSLATION_RESULT_CODE_PARTIALLY_MATCH = 12;
//	  private static final int TRANSLATION_RESULT_CODE_SIMILAR_WORDS = 13;
    private static final int TRANSLATION_RESULT_CODE_NO_MATCH = 14;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        int res = Activity.RESULT_CANCELED;
        SyncEngineAPI engine = Worker.getEngine();
        ArticleManagerAPI articleManager = Worker.getArticleManager();
        if(intent.hasExtra(Worker.EXTRA_CLIENT) && intent.hasExtra(Worker.EXTRA_QUERY) && intent.hasExtra(Worker.EXTRA_VALUE) && (engine != null) && (articleManager != null)) {
            String value = intent.getStringExtra(Worker.EXTRA_VALUE);
            if(!TextUtils.isEmpty(value)) {
                DictionaryAndDirection dnd = Worker.getSelectedDictionaryAndDirection();
                Collection<DictionaryAndDirection> dnds = Worker.getDictionariesAndDirections();
                res = TRANSLATION_RESULT_CODE_DATABASE_IS_UNAVAILABLE;
                if(dnd != null)
                    if(dnds.contains(dnd)) {
                        ArticleItem article = engine.find(dnd.getDictionaryId(), dnd.getDirection(), value);
                        if (article != null) {
                            articleManager.showArticleActivity(article, Worker.CONTROLLER_KEY, this);
                            res = TRANSLATION_RESULT_CODE_EXACT_MATCH;
                        }
                        else
                            res = TRANSLATION_RESULT_CODE_NO_MATCH;
                    }
            }
        }
        setResult(res, new Intent());
        finish();
    }
}
