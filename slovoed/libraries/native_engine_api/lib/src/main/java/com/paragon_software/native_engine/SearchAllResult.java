package com.paragon_software.native_engine;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.collections.CollectionView;

public interface SearchAllResult {

    CollectionView<ArticleItem, Void> getArticleItemList();

    CollectionView<Dictionary.DictionaryId, Void> getDictionaryIdList();
}
