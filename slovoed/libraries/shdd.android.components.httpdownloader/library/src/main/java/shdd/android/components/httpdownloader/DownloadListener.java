package shdd.android.components.httpdownloader;

public interface DownloadListener {
	
	void onDownloadChanged(DownloadInfo info, DownloadListener.Type type);
	
	enum Type {
		STATE,
		PROGRESS;
	}
	
}
