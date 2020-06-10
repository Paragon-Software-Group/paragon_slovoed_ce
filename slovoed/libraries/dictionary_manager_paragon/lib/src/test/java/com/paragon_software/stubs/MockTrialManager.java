package com.paragon_software.stubs;

import android.content.Context;
import androidx.annotation.NonNull;

import com.paragon_software.trial_manager.TrialManagerAPI;

import org.mockito.Mockito;

public abstract class MockTrialManager implements TrialManagerAPI
{
    public abstract static class Builder implements TrialManagerAPI.Builder
    {
        @NonNull
        @Override
        public TrialManagerAPI build(Context context)
        {
            return Mockito.spy(MockTrialManager.class);
        }
    }
}
