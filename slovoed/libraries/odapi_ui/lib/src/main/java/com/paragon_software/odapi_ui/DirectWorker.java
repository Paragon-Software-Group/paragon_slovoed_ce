package com.paragon_software.odapi_ui;

import android.content.Context;
import android.os.Message;
import androidx.annotation.NonNull;

import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.analytics_manager.events.OdapiLaunchEvent;

final class DirectWorker extends Worker {
    @NonNull
    private final ErrorResult mResult;

    DirectWorker(@NonNull ErrorResult result) {
        mResult = result;
    }

    @NonNull
    @Override
    Result getResult() {
        return mResult;
    }

    @Override
    public void trackAnalyticsEvent( @NonNull Message message )
    {
        Context context = getContext();
        if ( null != context )
        {
            String callerPackageName = context.getPackageManager().getNameForUid(message.sendingUid);
            AnalyticsManagerAPI.get().logEvent(
                new OdapiLaunchEvent(mResult.getError(), mResult.getMessage(), callerPackageName));
        }
    }
}
