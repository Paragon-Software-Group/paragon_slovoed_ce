package shdd.android.components.news.registration;

import android.support.annotation.NonNull;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.iid.FirebaseInstanceId;
import com.google.firebase.iid.InstanceIdResult;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import shdd.android.components.news.utils.ShddLog;

public class FirebaseCm {
    public static String getTokenSync() {
        Task<InstanceIdResult> instanceId = FirebaseInstanceId.getInstance().getInstanceId();
        ShddLog.d("shdd", "Firebase.getTokenSync(); Initially isComplete : " + instanceId.isComplete());
        final CountDownLatch latch = new CountDownLatch(1);
        if (!instanceId.isComplete()) {
            instanceId.addOnCompleteListener(new OnCompleteListener<InstanceIdResult>() {
                @Override
                public void onComplete(@NonNull Task<InstanceIdResult> task) {
                    latch.countDown();
                }
            });

            try {
                latch.await(5, TimeUnit.SECONDS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            ShddLog.d("shdd", "Firebase.getTokenSync(); return : " + (instanceId.isSuccessful() ? instanceId.getResult().getToken() : "null") + "; isSuccessful() : " + instanceId.isSuccessful());
            return instanceId.isSuccessful() ? instanceId.getResult().getToken() : null;

        } else {
            try {
                ShddLog.d("shdd", "Firebase.getTokenSync(); return : " + (instanceId.isSuccessful() ? instanceId.getResult().getToken() : "null") + "; isSuccessful() : " + instanceId.isSuccessful());
                return instanceId.isSuccessful() ? instanceId.getResult().getToken() : null;
            } catch (Exception ignored) {
                return null;
            }
        }
    }
}
