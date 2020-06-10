package com.paragon_software.toolbar_manager.Notifiers;

import java.util.ArrayList;

public abstract class MockForController_OnDeleteSelectedActionClickNotifier {

    private final ArrayList<OnDeleteSelectedActionClick> mDeleteActionClicks = new ArrayList<>();

    public void registerNotifier(OnDeleteSelectedActionClick notifier) {
        if (!mDeleteActionClicks.contains(notifier)) {
            mDeleteActionClicks.add(notifier);
        }
    }

    public void onDeleteSelectedActionClick() {
        for (final OnDeleteSelectedActionClick listener : mDeleteActionClicks) {
            listener.onDeleteSelectedActionClick();
        }
    }
}