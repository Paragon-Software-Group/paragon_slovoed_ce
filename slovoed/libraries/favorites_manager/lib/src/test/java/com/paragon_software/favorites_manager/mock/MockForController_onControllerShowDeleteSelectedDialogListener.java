package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.OnControllerShowDeleteSelectedDialogListener;

import static org.junit.Assert.assertTrue;

public abstract class MockForController_onControllerShowDeleteSelectedDialogListener
        implements OnControllerShowDeleteSelectedDialogListener {


    private int mNumberOfCalls;
    private int mMaxNumberOfCalls;

    public MockForController_onControllerShowDeleteSelectedDialogListener(int maxNumberOfCalls) {
        mMaxNumberOfCalls = maxNumberOfCalls;
    }

    @Override
    public void onControllerShowDeleteSelectedDialog() {
        assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    }
}