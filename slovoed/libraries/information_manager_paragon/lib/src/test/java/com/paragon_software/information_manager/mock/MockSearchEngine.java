package com.paragon_software.information_manager.mock;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.mockito.Mockito;

public abstract class MockSearchEngine implements SearchEngineAPI {
    private MockCollectionView mockCollection;

    public MockSearchEngine() {
        mockCollection = Mockito.spy(MockCollectionView.class);
    }

    @Override
    public CollectionView<ArticleItem, Void> getAdditionalArticles(Dictionary.DictionaryId id) {
        mockCollection.setDictId(id);
        return ( MockCollectionView<ArticleItem, Void>)mockCollection;
    }
}
