package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.OnControllerSortingActionChange;

import static org.junit.Assert.assertTrue;

public abstract class MockForController_onControllerSotringActionChanged
        implements OnControllerSortingActionChange {

    private final int mMaxNumberEnabledOfCall;
    private int mNumberOfVisibilityCalls;
    private int mNumberOfEnableCalls;

    private final int mMaxNumberVisibilityOfCalls;

    public MockForController_onControllerSotringActionChanged(int maxNumberVisibilityOfCalls, int maxNumberEnabledOfCall) {
        mMaxNumberVisibilityOfCalls = maxNumberVisibilityOfCalls;
        mMaxNumberEnabledOfCall = maxNumberEnabledOfCall;
    }

    @Override
    public void onSortingActionVisibilityStatusChange() {
        assertTrue(mNumberOfVisibilityCalls++ < mMaxNumberVisibilityOfCalls);
    }

    public void onSortingActionEnableStatusChange() {
        assertTrue(mNumberOfEnableCalls++ < mMaxNumberEnabledOfCall);
    }
}