package shdd.android.components.httpdownloader;

public interface DownloadRawListener {
	
	void onDownloadRaw(DownloadInfo info, byte[] raw, int size);

}
