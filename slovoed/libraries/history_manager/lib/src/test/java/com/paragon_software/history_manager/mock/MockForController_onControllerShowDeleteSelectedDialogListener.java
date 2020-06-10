package com.paragon_software.history_manager.mock;

import com.paragon_software.history_manager.OnControllerShowDeleteSelectedDialogListener;

import static junit.framework.TestCase.assertTrue;

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
