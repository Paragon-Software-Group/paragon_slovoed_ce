package com.paragon_software.engine.rx.getadditional;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;

import java.util.Collections;
import java.util.List;

class Result {
    @NonNull
    final List<List<ArticleItem>> items;

    Result(@Nullable List<List<ArticleItem>> _items) {
        if(_items != null)
            items = _items;
        else
            items = Collections.emptyList();
    }
}
