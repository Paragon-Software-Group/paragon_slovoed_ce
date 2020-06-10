package shdd.android.components.httpdownloader;

import android.content.Context;

class StorageManager {
	
    /** how often do we need to perform checks on space to make sure space is available */
    private static final int FREQUENCY_OF_CHECKS_ON_SPACE_AVAILABILITY = 1024 * 1024; // 1MiB
    
    private int mBytesDownloadedSinceLastCheckOnSpace;
    private final Context mContext;

    public StorageManager(Context context) {
        mContext = context;
    }

    void verifySpaceBeforeWritingToFile(DownloadInfo info, long bytesRead) throws StopRequestException {
        // do this check only once for every 1MiB of downloaded data
        if (incrementBytesDownloadedSinceLastCheckOnSpace(bytesRead) < FREQUENCY_OF_CHECKS_ON_SPACE_AVAILABILITY)
            return;
        verifySpace(info, bytesRead);
    }
    void verifySpace(DownloadInfo info, long bytesRead) throws StopRequestException {
        resetBytesDownloadedSinceLastCheckOnSpace();
        if (bytesRead == 0) return;
        long bytesAvailable = Utils.getAvailableBytesInFileSystemAt(info.file);
        if (bytesAvailable < Math.max(bytesRead, info.request.fileSize - info.file.length()))
            throw new StopRequestException(Status.STORAGE_INSUFFICIENT_SPACE, "Not enough free space (" + bytesAvailable +" bytes) in the filesystem root of the file: " + info.file);
    }
    private int incrementBytesDownloadedSinceLastCheckOnSpace(long val) {
        return mBytesDownloadedSinceLastCheckOnSpace += val;
    }
    private void resetBytesDownloadedSinceLastCheckOnSpace() {
        mBytesDownloadedSinceLastCheckOnSpace = 0;
    }
    
}
