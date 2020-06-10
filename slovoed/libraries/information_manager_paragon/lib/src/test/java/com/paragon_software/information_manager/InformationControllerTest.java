package com.paragon_software.information_manager;

import android.content.Context;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.information_manager.mock.MockDictionaryManager;
import com.paragon_software.information_manager.mock.MockSearchEngine;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;


public class InformationControllerTest {
    @Rule
    public MockitoRule rule = MockitoJUnit.rule();
    @Mock
    private InformationManagerAPI mInformationManager;

    private MockDictionaryManager mDictionaryManager;

    private InformationControllerAPI mInformationController;

    @Mock
    private Context mContext;

    @Before
    public void setUp() {

        mDictionaryManager = Mockito.spy(MockDictionaryManager.class);
        SearchEngineAPI mSearchEngine = Mockito.spy(MockSearchEngine.class);
        mInformationController = new InformationController(mInformationManager, mDictionaryManager, mSearchEngine);
    }

    @Test
    public void testGetInformationItem() {
        Map<InformationItemType, String> testMap = new HashMap<InformationItemType, String>() {{
            put(InformationItemType.EULA, "value1");
            put(InformationItemType.PRIVACY_POLICY, "value2");
            put(InformationItemType.ABOUT, "value3");
            put(InformationItemType.ADDITIONAL_ARTICLES, "value4");
        }};

        assertEquals(0, mInformationController.getInformationItems(Collections.EMPTY_MAP).getCount());

        CollectionView<? extends InformationItem, Void> collection = mInformationController.getInformationItems(testMap);

        //MockCollectionView add 2 AdditionalArticles
        assertEquals(testMap.size() + 1, collection.getCount());
        boolean isOrderCheck = true;
        for (int i = 0; i < collection.getCount(); i++) {
            //check InformationController.orderItems(informationItems )
            if (InformationItemType.ADDITIONAL_ARTICLES.equals(collection.getItem(i).getType()) && isOrderCheck) {
                isOrderCheck = false;
                assertEquals(collection.getItem(i).getType(), collection.getItem(i + 1).getType());
            } else {
                assertEquals(testMap.get(collection.getItem(i).getType()), collection.getItem(i).getText());
            }
            testMap.remove(collection.getItem(i).getType());
        }
        assertEquals(0, testMap.size());

        collection = mInformationController.getInformationItems(testMap);
        assertEquals(0, collection.getCount());
    }

    @Test
    public void testDictionaryListChange() {
        Map<InformationItemType, String> testMap = new HashMap<InformationItemType, String>() {{
            put(InformationItemType.ADDITIONAL_ARTICLES, "value1");
        }};

        CollectionView<? extends InformationItem, Void> collection = mInformationController.getInformationItems(testMap);

        ArticleItem article = collection.getItem(0).getArticleItem();
        //test mInformationController.openInformationItem
        mInformationController.openInformationItem(mContext, collection.getItem(0));
        verify(mInformationManager, times(1)).openInformationItem(mContext, collection.getItem(0));

        mDictionaryManager.onDictionaryListChanged(mDictionaryManager.dictionaryAndDirection2);
        assertNotEquals(article, collection.getItem(0).getArticleItem());

        testMap = new HashMap<InformationItemType, String>() {{
            put(InformationItemType.ABOUT, "value1");
        }};
        collection = mInformationController.getInformationItems(testMap);


        Dictionary.DictionaryId dictionaryId = collection.getItem(0).getDictionaryId();
        mDictionaryManager.onDictionaryListChanged(mDictionaryManager.dictionaryAndDirection);
        assertNotEquals(dictionaryId, collection.getItem(0).getDictionaryId());
    }
}
