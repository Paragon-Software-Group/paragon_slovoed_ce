package shdd.android.components.news;

import android.content.Context;
import android.content.Intent;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;

import shdd.android.components.news.utils.ShddLog;

public class MessagingService extends FirebaseMessagingService {


    @Override
    public void onMessageReceived(RemoteMessage remoteMessage) {
        ShddLog.d("shdd", "MessagingService.onMessageReceived()");
        remoteMessageToLog(remoteMessage);
        Context context = this.getApplicationContext();
        Intent service = new Intent(context, AdsIntentService.class).putExtra(AdsIntentService.PUSH_KEY, true).putExtra(AdsIntentService.PUSH_REMOTE_MESSAGE, remoteMessage);
        AdsIntentService.enqueueWork(context, AdsIntentService.class, AdsManager.JOB_ID, service);
    }

    @Override
    public void onNewToken(String s) {
        ShddLog.d("shdd", "MessagingService.onNewToken() t : " + s);
        Context context = this.getApplicationContext();
        Intent intent = new Intent(context, AdsIntentService.class).putExtra(AdsIntentService.INIT_KEY, true);
        AdsIntentService.enqueueWork(context, AdsIntentService.class, AdsManager.JOB_ID, intent);
    }

    @Override
    public void onDeletedMessages() {
        ShddLog.d("shdd", "MessagingService.onDeletedMessages()");
        Context context = this.getApplicationContext();
        Intent intent = new Intent(context, AdsIntentService.class).putExtra(AdsIntentService.CHECK_NEWS_KEY, true);
        AdsIntentService.enqueueWork(context, AdsIntentService.class, AdsManager.JOB_ID, intent);
    }

    private void remoteMessageToLog(RemoteMessage rm) {
        if (null == rm) {
            ShddLog.e("shdd", "Remote Message is null");
            return;
        }

        try {
            ShddLog.d("shdd", "RemoteMessage getTtl() :" + rm.getTtl());
            ShddLog.d("shdd", "RemoteMessage getSentTime() :" + rm.getSentTime());
            ShddLog.d("shdd", "RemoteMessage getOriginalPriority() :" + rm.getOriginalPriority());
            ShddLog.d("shdd", "RemoteMessage getPriority() :" + rm.getPriority());
            ShddLog.d("shdd", "RemoteMessage getFrom() :" + rm.getFrom());
            ShddLog.d("shdd", "RemoteMessage getTo() :" + rm.getTo());
            ShddLog.d("shdd", "RemoteMessage getMessageId() :" + rm.getMessageId());
            ShddLog.d("shdd", "RemoteMessage getMessageType() :" + rm.getMessageType());
            ShddLog.d("shdd", "RemoteMessage getCollapseKey() :" + rm.getCollapseKey());
            for (String key : rm.getData().keySet()) {
                ShddLog.d("shdd", "RemoteMessage getData() [ " + key + " : " + rm.getData().get(key) + " ]");
            }
            ShddLog.d("shdd", "RemoteMessage getNotification() :" + rm.getNotification());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
