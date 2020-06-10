package shdd.android.components.httpdownloader;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;

public class DownloadInfo {
	
	public final Request request;
    public final File file;
    public URL url;

    public boolean gotData;
    public int tryCount;
    public long totalBytes;
	public long currentBytes;
	public int redirectionCount;
	public String permanentRedirectUrl;
	public String headerETag;
	public String contentDisposition;
	public String contentLocation;
	public long contentLength;
	public String mimeType;
	public boolean continuingDownload;
	public long speed;/** Historical bytes/second speed of this download. */
	public long speedSampleStart;/** Time when current sample started. */
	public long speedSampleBytes;/** Bytes transferred since current sample started. */
	public boolean speedReal;
	public long bytesNotified;
	public long timeLastNotification;
	public long timeFirstShowNotification;
	public Status status;
	public DownloadInfo copyOf;
	private boolean consume;
	
	public DownloadInfo(Request request) {
		this.request = request;
		this.file = new File(request.destDir, request.destFilename);
		try {
			this.url = new URL(request.url);
		} catch (MalformedURLException e) {
			throw new IllegalArgumentException(e);
		}
	}
	DownloadInfo copy() {
		DownloadInfo i = new DownloadInfo(this.request);
		i.gotData = this.gotData;
		i.tryCount = this.tryCount;
		i.totalBytes = this.totalBytes;
		i.currentBytes = this.currentBytes;
		i.redirectionCount = this.redirectionCount;
		i.permanentRedirectUrl = this.permanentRedirectUrl;
		i.headerETag = this.headerETag;
		i.contentDisposition = this.contentDisposition;
		i.contentLocation = this.contentLocation;
		i.contentLength = this.contentLength;
		i.mimeType = this.mimeType;
		i.continuingDownload = this.continuingDownload;
		i.speed = this.speed;
		i.speedReal = this.speedReal;
		i.speedSampleStart = this.speedSampleStart;
		i.speedSampleBytes = this.speedSampleBytes;
		i.bytesNotified = this.bytesNotified;
		i.timeLastNotification = this.timeLastNotification;
		i.timeFirstShowNotification = this.timeFirstShowNotification;
		i.status = this.status;
		i.copyOf = this;
		i.consume = this.consume;
		return i;
	}
	void resetBeforeExecute() {
		totalBytes = 0;
		currentBytes = 0;
		redirectionCount = 0;
		permanentRedirectUrl = null;
		headerETag = null;
		contentDisposition = null;
		contentLocation = null;
		contentLength = 0;
		mimeType = null;
		speed = 0;
		speedReal = false;
		speedSampleStart = 0;
		speedSampleBytes = 0;
		bytesNotified = 0;
		timeLastNotification = 0;
		continuingDownload = false;
		consume = false;
	}
	
	public boolean consume() {
		return !consume && (consume = true);
	}
	public boolean canShowSpeed() {
		return status == Status.DOWNLOADING && speedReal;
	}
	public boolean canShowRemaining() {
		return canShowSpeed();
	}
	public static boolean canShowProgress(DownloadInfo info) {
		return info != null && info.status == Status.DOWNLOADING && info.currentBytes > 0;
	}
	
}
