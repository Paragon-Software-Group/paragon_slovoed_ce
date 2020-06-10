package com.paragon_software.hint_manager;

import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

import androidx.fragment.app.FragmentManager;

import static junit.framework.TestCase.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;


public class HintManagerTest {
    @Rule
    public MockitoRule rule = MockitoJUnit.rule();

    @Mock
    private HintUIHelperAPI mHintUIHelper;

    @Mock
    private FragmentManager mFragmentManager;

    @Mock
    Runnable hintOkRunnable = () -> {
    };

    private MockSettingsManager mSettingsManager;

    private HintManagerAPI mHintManager;
    private HintParams mHintParamsFirst = new HintParams.Builder().setOnFirstActionRunnable(hintOkRunnable).build();
    private HintParams mHintParamsDismiss = new HintParams.Builder().setOnDismissRunnable(hintOkRunnable).build();


    @Before
    public void setUp() {
        mSettingsManager = Mockito.spy(MockSettingsManager.class);
        hintOkRunnable = Mockito.spy(Runnable.class);
        mHintManager = new BaseHintManagerFactory().
                registerSettingsManager(mSettingsManager).
                registerHintUIHelper(mHintUIHelper).create();
    }

    @Test
    public void testGetSetHintParam() {
        assertNull(mHintManager.getHintParams(HintType.History));
        mHintManager.setHintParams(HintType.History, mHintParamsFirst);
        assertSame(mHintParamsFirst, mHintManager.getHintParams(HintType.History));
        assertNotNull(mHintManager.getHintParams(HintType.History).getOnFirstActionRunnable());
        assertNull(mHintManager.getHintParams(HintType.History).getOnDismissRunnable());

        mHintManager.setHintParams(HintType.News, mHintParamsDismiss);
        assertSame(mHintParamsDismiss, mHintManager.getHintParams(HintType.News));
        assertNotNull(mHintManager.getHintParams(HintType.News).getOnDismissRunnable());
        assertNull(mHintManager.getHintParams(HintType.News).getOnFirstActionRunnable());
    }

    @Test
    public void testNeedToShowHint() throws ManagerInitException, WrongTypeException, LocalResourceUnavailableException {
        mHintManager.setNeedToShowHint(HintType.History, false);
        verify(mSettingsManager, times(1)).save("need_to_show_hint_history", false, true);
        assertFalse(mHintManager.isNeedToShowHint(HintType.History));
        verify(mSettingsManager, times(1)).load("need_to_show_hint_history", true);

        assertTrue(mHintManager.isNeedToShowHint(HintType.News));
        verify(mSettingsManager, times(1)).load("need_to_show_hint_news", true);
        mHintManager.setNeedToShowHint(HintType.News, false);
        assertFalse(mHintManager.isNeedToShowHint(HintType.News));
    }

    @Test
    public void testShowHintDialog() {
        mHintManager.showHintDialog(HintType.History, mFragmentManager, mHintParamsFirst);
        verify(mHintUIHelper, times(1)).showHintDialog(HintType.History, mFragmentManager);
        assertSame(mHintParamsFirst, mHintManager.getHintParams(HintType.History));
        assertNotNull(mHintManager.getHintParams(HintType.History).getOnFirstActionRunnable());
        assertNull(mHintManager.getHintParams(HintType.History).getOnDismissRunnable());
    }

    @Test
    public void testHintManagerHolder() {
        assertNull(HintManagerHolder.getManager());
        HintManagerHolder.setManager(mHintManager);
        assertSame(mHintManager, HintManagerHolder.getManager());
    }
}
