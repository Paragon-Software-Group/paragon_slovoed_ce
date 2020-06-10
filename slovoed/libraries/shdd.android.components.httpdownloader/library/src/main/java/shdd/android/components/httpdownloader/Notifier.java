package shdd.android.components.httpdownloader;

import java.util.Date;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.text.format.DateFormat;
import android.view.View;
import android.widget.RemoteViews;

class Notifier {
	
	private static final String TAG_ACTIVE = "active";
	private static final String TAG_COMPLETE = "complete";
	private Context ctx;
	private NotificationManager nm;
	private DownloadClientHelper dch;
	private final NotificationSpecs notificationSpecs;
	private static boolean isNotificationChanelCreated = false;

	Notifier(Context ctx, DownloadClientHelper dch) {
		this.ctx = ctx;
		this.dch = dch;
		this.notificationSpecs = dch.getNotificationSpecs();
		this.nm = (NotificationManager) ctx.getSystemService(Context.NOTIFICATION_SERVICE);
		createNotificationChanel();
	}

	private void createNotificationChanel() {
		if (isNotificationChanelCreated || Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
			return;
		}

		NotificationManager manager = (NotificationManager) ctx.getSystemService(Context.NOTIFICATION_SERVICE);
		if (manager != null) {
            manager.createNotificationChannel(notificationSpecs.getChannel());
            isNotificationChanelCreated = true;
        }
	}

	void remove(Request r) {
		nm.cancel(TAG_COMPLETE, r.url.hashCode());
		nm.cancel(TAG_ACTIVE, r.url.hashCode());
	}

	void process(DownloadInfo info, DownloadListener.Type type) {
		switch (info.status) {
		case CONNECTING:
		case DOWNLOADING:
			nm.cancel(TAG_COMPLETE, info.request.url.hashCode());
			nm.notify(TAG_ACTIVE, info.request.url.hashCode(), createNotification(info, type, true));
			break;
		case PAUSED:
		case CANCELED:
			throw new IllegalStateException();
		case SUCCESSFULL:
			nm.cancel(TAG_ACTIVE, info.request.url.hashCode());
			nm.notify(TAG_COMPLETE, info.request.url.hashCode(), createNotification(info, type, false));
			break;
		default:
			nm.cancel(TAG_ACTIVE, info.request.url.hashCode());
			nm.notify(TAG_COMPLETE, info.request.url.hashCode(), createNotification(info, type, false));
		}
	}
	private Notification createNotification(DownloadInfo info, DownloadListener.Type type, boolean active) {
		return Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 ? createNotificationSystem(info, type, active) : createNotificationCustom(info, type, active); 
	}
	@RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
	private Notification createNotificationSystem(DownloadInfo info, DownloadListener.Type type, boolean active) {
		Notification.Builder b = new Notification.Builder(ctx);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			b.setChannelId(notificationSpecs.getChannelId());
		}
		b.setWhen(info.copyOf.timeFirstShowNotification == 0 ? info.copyOf.timeFirstShowNotification = System.currentTimeMillis() : info.copyOf.timeFirstShowNotification);
		b.setContentIntent(notificationSpecs.createPendingIntent(info, type));
		b.setLargeIcon(notificationSpecs.getLargeIcon(info));
		b.setContentTitle(dch.getString(info, DownloadClientHelper.STRINGS.TITLE));
		b.setContentText(dch.getString(info, DownloadClientHelper.STRINGS.TEXT));
		b.setOnlyAlertOnce(true);
		if (active) {
			b.setOngoing(true);
			b.setSmallIcon(notificationSpecs.getSmallIcon(info));
			b.setShowWhen(false);
			if (DownloadInfo.canShowProgress(info)) {
				if (info.canShowSpeed())
					b.setContentTitle(dch.getString(info, DownloadClientHelper.STRINGS.TITLE) + " (" +dch.getString(info, DownloadClientHelper.STRINGS.SPEED) + ")");
				int progress = (int)(((double)info.currentBytes/info.request.fileSize)*100);
				b.setProgress(100, progress, false);
				if (info.canShowRemaining()) {
					b.setContentInfo(dch.getString(info, DownloadClientHelper.STRINGS.REMAINING));
					b.setContentText(dch.getString(info, DownloadClientHelper.STRINGS.SIZE));
				}
			} else {
				b.setProgress(100, 0, true);
			}
		} else {
			b.setAutoCancel(true);
			b.setSmallIcon(notificationSpecs.getSmallIcon(info));
			String subText = dch.getString(info, DownloadClientHelper.STRINGS.SUB_TEXT);
			if (subText != null)
				b.setSubText(subText);
		}
		return b.build();
	}
	private Notification createNotificationCustom(DownloadInfo info, DownloadListener.Type type, boolean active) {
		Notification n = new Notification();
		n.when = info.copyOf.timeFirstShowNotification == 0 ? info.copyOf.timeFirstShowNotification = System.currentTimeMillis() : info.copyOf.timeFirstShowNotification;
		n.contentIntent = notificationSpecs.createPendingIntent(info, type);
		RemoteViews rv = new RemoteViews(ctx.getPackageName(), dch.getLayout(info, DownloadClientHelper.LAYOUT.NOTIFICATION));
		rv.setTextViewText(dch.getId(DownloadClientHelper.ID.WHEN), DateFormat.getTimeFormat(ctx).format(new Date(n.when)));
		rv.setViewVisibility(dch.getId(DownloadClientHelper.ID.WHEN), View.VISIBLE);
		rv.setImageViewBitmap(dch.getId(DownloadClientHelper.ID.LARGE_ICON), notificationSpecs.getLargeIcon(info));
		rv.setTextViewText(dch.getId(DownloadClientHelper.ID.TITLE), dch.getString(info, DownloadClientHelper.STRINGS.TITLE));
		rv.setTextViewText(dch.getId(DownloadClientHelper.ID.TEXT), dch.getString(info, DownloadClientHelper.STRINGS.TEXT));
		if (active) {
			n.flags |= Notification.FLAG_ONGOING_EVENT;
			n.icon = notificationSpecs.getSmallIcon(info);
			rv.setViewVisibility(dch.getId(DownloadClientHelper.ID.WHEN), View.GONE);
			if (DownloadInfo.canShowProgress(info)) {
				if (info.canShowSpeed())
					rv.setTextViewText(dch.getId(DownloadClientHelper.ID.TITLE), dch.getString(info, DownloadClientHelper.STRINGS.TITLE) + " (" +dch.getString(info, DownloadClientHelper.STRINGS.SPEED) + ")");
				int progress = (int)(((double)info.currentBytes/info.request.fileSize)*100);
				rv.setProgressBar(dch.getId(DownloadClientHelper.ID.PROGRESS), 100, progress, false);
				if (info.canShowRemaining()) {
					rv.setTextViewText(dch.getId(DownloadClientHelper.ID.TEXT), dch.getString(info, DownloadClientHelper.STRINGS.SIZE));
					rv.setTextViewText(dch.getId(DownloadClientHelper.ID.INFO), dch.getString(info, DownloadClientHelper.STRINGS.REMAINING));
					rv.setViewVisibility(dch.getId(DownloadClientHelper.ID.INFO), View.VISIBLE);
				} else {
					rv.setViewVisibility(dch.getId(DownloadClientHelper.ID.INFO), View.GONE);
				}
			} else {
				rv.setProgressBar(dch.getId(DownloadClientHelper.ID.PROGRESS), 100, 0, true);
				rv.setViewVisibility(dch.getId(DownloadClientHelper.ID.INFO), View.GONE);
			}
		} else {
			n.flags |= Notification.FLAG_AUTO_CANCEL;
			n.icon = notificationSpecs.getSmallIcon(info);
			String subText = dch.getString(info, DownloadClientHelper.STRINGS.SUB_TEXT);
			if (subText != null) {
				rv.setTextViewText(dch.getId(DownloadClientHelper.ID.SUB_TEXT), dch.getString(info, DownloadClientHelper.STRINGS.SUB_TEXT));
				rv.setViewVisibility(dch.getId(DownloadClientHelper.ID.SUB_TEXT), View.VISIBLE);
			}
		}
		rv.setImageViewResource(dch.getId(DownloadClientHelper.ID.SMALL_ICON), n.icon);
		n.contentView = rv;
		return n;
	}

}














