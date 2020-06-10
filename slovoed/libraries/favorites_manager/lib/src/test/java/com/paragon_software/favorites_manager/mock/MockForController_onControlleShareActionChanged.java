package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.OnControllerShareActionChange;
import com.paragon_software.favorites_manager.OnControllerSortingActionChange;

import static org.junit.Assert.assertTrue;

public abstract class MockForController_onControlleShareActionChanged
        implements OnControllerShareActionChange {

    private final int mMaxNumberEnabledOfCall;
    private int mNumberOfVisibilityCalls;
    private int mNumberOfEnableCalls;

    private final int mMaxNumberVisibilityOfCalls;

    public MockForController_onControlleShareActionChanged(int maxNumberVisibilityOfCalls, int maxNumberEnabledOfCall) {
        mMaxNumberVisibilityOfCalls = maxNumberVisibilityOfCalls;
        mMaxNumberEnabledOfCall = maxNumberEnabledOfCall;
    }

    @Override
    public void onShareActionVisibilityStatusChange() {
        assertTrue(mNumberOfVisibilityCalls++ < mMaxNumberVisibilityOfCalls);
    }

    public void onShareActionEnableStatusChange() {
        assertTrue(mNumberOfEnableCalls++ < mMaxNumberEnabledOfCall);
    }
}