package com.paragon_software.information_manager.mock;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.collections.CachedCollectionView;

import java.util.Arrays;
import java.util.List;

public abstract class MockCollectionView<ITEM_TYPE, LABEL_TYPE> extends CachedCollectionView<ITEM_TYPE, LABEL_TYPE> {

    private Dictionary.DictionaryId dictId;


    private void sendEventItemRangeInserted() {
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, 0, 1);
    }

    @Override
    public void registerListener(Notifier notifier) {
        super.registerListener(notifier);
        resetCache();
        cacheItems((List<ITEM_TYPE>) Arrays.asList(new ArticleItem.Builder(dictId, 1, 1).build(), new ArticleItem.Builder(dictId, 2, 2).build()));
        sendEventItemRangeInserted();
    }

    void setDictId(Dictionary.DictionaryId id) {
        dictId = id;
    }
}
