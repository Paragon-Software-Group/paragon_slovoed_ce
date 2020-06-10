package com.paragon_software.article_manager;

import android.content.Context;
import androidx.annotation.Nullable;
import android.util.AttributeSet;

import com.paragon_software.article_manager_ui.R;

/**
 * Created by Ivan Kuzmin on 13.01.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public class SearchInArticleInputField extends BaseSearchInArticleInputField {

    public SearchInArticleInputField(Context context) {
        super(context);
    }

    public SearchInArticleInputField(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public SearchInArticleInputField(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected int getEditTextInputId() {
        return R.id.text_input;
    }

    @Override
    protected int getSearchResultViewId() {
        return R.id.search_result;
    }
}
