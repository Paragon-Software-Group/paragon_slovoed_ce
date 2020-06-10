package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.OnControllerDeleteAllActionChange;
import com.paragon_software.favorites_manager.OnControllerSortingActionChange;

import static org.junit.Assert.assertTrue;

public abstract class MockForController_onControllerDeleteAllActionChanged
        implements OnControllerDeleteAllActionChange {

    private final int mMaxNumberEnabledOfCall;
    private int mNumberOfVisibilityCalls;
    private int mNumberOfEnableCalls;

    private final int mMaxNumberVisibilityOfCalls;

    public MockForController_onControllerDeleteAllActionChanged(int maxNumberVisibilityOfCalls, int maxNumberEnabledOfCall) {
        mMaxNumberVisibilityOfCalls = maxNumberVisibilityOfCalls;
        mMaxNumberEnabledOfCall = maxNumberEnabledOfCall;
    }

    @Override
    public void onDeleteAllActionVisibilityStatusChange() {
        assertTrue(mNumberOfVisibilityCalls++ < mMaxNumberVisibilityOfCalls);
    }

    public void onDeleteAllActionEnableStatusChange() {
        assertTrue(mNumberOfEnableCalls++ < mMaxNumberEnabledOfCall);
    }
}