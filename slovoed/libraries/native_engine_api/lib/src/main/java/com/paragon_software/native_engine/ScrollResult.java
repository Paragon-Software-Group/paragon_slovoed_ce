package com.paragon_software.native_engine;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.collections.CollectionView;

public interface ScrollResult
{
    CollectionView<ArticleItem, Dictionary.Direction> getArticleItemList();

    CollectionView<ArticleItem, Void> getMorphoArticleItemList();

    boolean startsWith( @NonNull String text );
}
