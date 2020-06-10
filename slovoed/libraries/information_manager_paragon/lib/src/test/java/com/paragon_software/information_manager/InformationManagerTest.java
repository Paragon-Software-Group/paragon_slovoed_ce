package com.paragon_software.information_manager;

import android.content.Context;
import android.content.Intent;

import com.paragon_software.about_manager.AboutManagerAPI;
import com.paragon_software.article_manager.ArticleControllerType;
import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

import static junit.framework.TestCase.assertNotSame;
import static junit.framework.TestCase.assertSame;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;


public class InformationManagerTest {
    @Rule
    public MockitoRule rule = MockitoJUnit.rule();

    @Mock
    private DictionaryManagerAPI mDictionaryManager;

    @Mock
    private SearchEngineAPI mSearchEngine;
    @Mock
    private AboutManagerAPI mAboutManager;

    @Mock
    private ArticleManagerAPI mArticleManager;

    @Mock
    private Intent mPrivacyPolicyIntent;

    @Mock
    private Intent mEulaIntent;

    @Mock
    private ArticleItem mArticleItem;

    private InformationManagerAPI mInformationManager;

    @Mock
    private Context mContext;

    private static final Dictionary.DictionaryId DICT1_ID = new Dictionary.DictionaryId("111");
    private static final Dictionary.DictionaryId DICT2_ID = new Dictionary.DictionaryId("222");

    @Before
    public void setUp() {
        mInformationManager = new InformationManagerFactory().create(
                mDictionaryManager,
                mSearchEngine,
                mAboutManager,
                ArticleControllerType.CONTROLLER_ID_ABOUT,
                mArticleManager,
                ArticleControllerType.CONTROLLER_ID_ADDITIONAL_INFO,
                mPrivacyPolicyIntent,
                mEulaIntent,
            null);
    }

    @Test
    public void testOpenInformationItem() {
        String text = "test";
        InformationItem informationItemEula = InformationItem.create(InformationItemType.EULA, text, DICT1_ID);
        mInformationManager.openInformationItem(mContext, informationItemEula);
        verify(mContext, times(1)).startActivity(mEulaIntent);
        assertEquals(text, informationItemEula.getText());

        InformationItem informationItem = InformationItem.create(InformationItemType.PRIVACY_POLICY, text, DICT1_ID);
        assertNotEquals(informationItemEula, informationItem);

        mInformationManager.openInformationItem(mContext, informationItem);
        verify(mContext, times(1)).startActivity(mPrivacyPolicyIntent);

        informationItem = InformationItem.create(InformationItemType.ABOUT, text, DICT1_ID);
        mInformationManager.openInformationItem(mContext, informationItem);
        verify(mAboutManager, times(1)).showAbout(mContext, ArticleControllerType.CONTROLLER_ID_ABOUT, DICT1_ID);

        informationItem = InformationItem.createAdditionalArticleItem(mArticleItem);
        informationItem.getText();
        verify(mArticleItem, times(1)).getShowVariantText();

        mInformationManager.openInformationItem(mContext, informationItem);
        verify(mArticleManager, times(1)).showArticleActivity(mArticleItem, ArticleControllerType.CONTROLLER_ID_ADDITIONAL_INFO, mContext);

        assertNotEquals(informationItemEula.hashCode(), informationItem.hashCode());
    }

    @Test
    public void testGetController() {
        InformationControllerAPI controller1 = mInformationManager.getController("test1");
        InformationControllerAPI controller2 = mInformationManager.getController("test2");
        InformationControllerAPI controller3 = mInformationManager.getController("test1");
        assertNotSame(controller1, controller2);
        assertSame(controller1, controller3);
    }

    @Test
    public void testInformationMagerHolder() {
        InformationMangerHolder.setManager(mInformationManager);
        assertSame(mInformationManager, InformationMangerHolder.getManager());
    }
}
