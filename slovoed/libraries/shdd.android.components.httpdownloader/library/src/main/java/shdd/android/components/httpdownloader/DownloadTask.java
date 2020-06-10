package shdd.android.components.httpdownloader;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Process;
import android.os.SystemClock;
import android.text.format.DateUtils;
import android.util.Log;
import android.util.Pair;

class DownloadTask implements Runnable {
	
	private static final int HTTP_REQUESTED_RANGE_NOT_SATISFIABLE = 416;
    private static final int HTTP_TEMP_REDIRECT = 307;

    private static final int DEFAULT_TIMEOUT = (int) (20 * DateUtils.SECOND_IN_MILLIS);
	
    private Context ctx;
	private Downloader downloaderImpl;
	private StorageManager storageManager;
	private DownloadInfo info;
	
	private boolean retry;
	private volatile Status interrupt;
	
	DownloadTask(Context ctx, Downloader downloaderImpl, Request request) {
		this.ctx = ctx;
		this.downloaderImpl = downloaderImpl;
		this.info = new DownloadInfo(request);
		this.storageManager = new StorageManager(ctx);
	}
	public void run() {
		Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);
		info.status = Status.CONNECTING;
		downloaderImpl.notifyDownloadChanged(this, info.copy(), DownloadListener.Type.STATE);
		do {
			runInternal();
		} while (retry);
	}
	private void runInternal() {
//android.util.Log.e("shdd", hashCode() + " DownloadTask.run(): info.status: " + info.status + "; interrupt: " + interrupt + "; retry: " + retry);
		retry = false;
        Throwable error = null;
        try {
        	checkInterrupted();
        	if (info.status != Status.CONNECTING) {
        		info.status = Status.CONNECTING;
        		downloaderImpl.notifyDownloadChanged(this, info.copy(), DownloadListener.Type.STATE);
        	}
        	if (info.tryCount > 0)
        		Thread.sleep(Constants.RETRY_DELAY_MS);
        	executeDownload();
        	finalizeDestinationFile();
        	info.status = Status.SUCCESSFULL;
        } catch (Throwable e) {
        	error = e;
        }
        if (isInterrupted()) {
        	info.status = interrupt;
        	cleanupDestination();
        	return;
        }
        if (error != null) {
        	if (error instanceof StopRequestException) {
				Status errorStatus = ((StopRequestException)error).status;
        		if (Helpers.isStatusRetryable(errorStatus) && info.tryCount++ < Constants.MAX_RETRIES) {
            		info.resetBeforeExecute();
					retry = true;
            		return;
            	} else {
            		info.status = errorStatus;
            	}
        	} else {
        		error.printStackTrace();
        		info.status = Status.UNKNOWN_ERROR;
        	}
        }
        cleanupDestination();
        downloaderImpl.onComplete(this, info.copy());
	}
	private void executeDownload() throws StopRequestException {
		setupDestinationFile();
		while (info.redirectionCount++ < Constants.MAX_REDIRECTS) {
            HttpURLConnection conn = null;
            try {
                checkConnectivity();
                conn = (HttpURLConnection) info.url.openConnection();
                conn.setInstanceFollowRedirects(false);
                conn.setConnectTimeout(DEFAULT_TIMEOUT);
                conn.setReadTimeout(DEFAULT_TIMEOUT);
                addRequestHeaders(info, conn);
                final int responseCode = conn.getResponseCode();
                if (info.status != Status.DOWNLOADING) {
                	info.status = Status.DOWNLOADING;
                	downloaderImpl.notifyDownloadChanged(this, info.copy(), DownloadListener.Type.STATE);
                }
                switch (responseCode) {
                    case HttpURLConnection.HTTP_OK:
                        if (info.continuingDownload)
                            throw new StopRequestException(Status.HTTP_CANNOT_RESUME, "Expected partial, but received OK");
                        processResponseHeaders(info, conn);
                        transferData(info, conn);
                        return;
                    case HttpURLConnection.HTTP_PARTIAL:
                        if (!info.continuingDownload)
                            throw new StopRequestException(Status.HTTP_CANNOT_RESUME, "Expected OK, but received partial");
                        transferData(info, conn);
                        return;
                    case HttpURLConnection.HTTP_MOVED_PERM:
                    case HttpURLConnection.HTTP_MOVED_TEMP:
                    case HttpURLConnection.HTTP_SEE_OTHER:
                    case HTTP_TEMP_REDIRECT:
                        final String location = conn.getHeaderField("Location");
                        info.url = new URL(info.url, location);
                        if (responseCode == HttpURLConnection.HTTP_MOVED_PERM) {
                            info.permanentRedirectUrl = info.url.toString();
                        }
                        continue;
                    case HTTP_REQUESTED_RANGE_NOT_SATISFIABLE:
                        throw new StopRequestException(Status.HTTP_CANNOT_RESUME, "Requested range not satisfiable");
                    case HttpURLConnection.HTTP_UNAVAILABLE:
                        throw new StopRequestException(Status.HTTP_CODE_503_UNAVAILABLE, conn.getResponseMessage());
                    case HttpURLConnection.HTTP_INTERNAL_ERROR:
                        throw new StopRequestException(Status.HTTP_CODE_500_INTERNAL_ERROR, conn.getResponseMessage());
                    default:
                        StopRequestException.throwUnhandledHttpError(responseCode, conn.getResponseMessage());
                }
            } catch (IOException e) {
                // Trouble with low-level sockets
                throw new StopRequestException(Status.HTTP_DATA_ERROR, e);
            } finally {
                if (conn != null) conn.disconnect();
            }
        }
        throw new StopRequestException(Status.TOO_MANY_REDIRECTS, "Too many redirects");
	}
	private void addRequestHeaders(DownloadInfo info, HttpURLConnection conn) {
		if (info.request.headers != null)
			for (Pair<String, String> header : info.request.headers)
				conn.addRequestProperty(header.first, header.second);
		if (conn.getRequestProperty("User-Agent") == null)
			conn.addRequestProperty("User-Agent", info.request.userAgent == null ? Constants.DEFAULT_USER_AGENT : info.request.userAgent);
		// Defeat transparent gzip compression, since it doesn't allow us to easily resume partial downloads.
		conn.setRequestProperty("Accept-Encoding", "identity");
		if (info.continuingDownload) {
			if (info.headerETag != null)
				conn.addRequestProperty("If-Match", info.headerETag);
			conn.addRequestProperty("Range", "bytes=" + info.currentBytes + "-");
		}
	}
	private void processResponseHeaders(DownloadInfo info, HttpURLConnection conn) throws StopRequestException {
        readResponseHeaders(info, conn);
        checkConnectivity();
    }
	private void readResponseHeaders(DownloadInfo info, HttpURLConnection conn) throws StopRequestException {
        info.contentDisposition = conn.getHeaderField("Content-Disposition");
        info.contentLocation = conn.getHeaderField("Content-Location");
        if (info.mimeType == null)
            info.mimeType = Helpers.normalizeMimeType(conn.getContentType());
        info.headerETag = conn.getHeaderField("ETag");
        String transferEncoding = conn.getHeaderField("Transfer-Encoding");
        if (transferEncoding == null) {
            info.contentLength = Helpers.getHeaderFieldLong(conn, "Content-Length", -1);
        } else {
            info.contentLength = -1;
        }
        info.totalBytes = info.contentLength;
        boolean noSizeInfo = info.contentLength == -1 && (transferEncoding == null || !transferEncoding.equalsIgnoreCase("chunked"));
        if (!info.request.noIntegrity && noSizeInfo)
            throw new StopRequestException(Status.HTTP_CANNOT_RESUME, "can't know size of download, giving up");
    }
	private void transferData(DownloadInfo info, HttpURLConnection conn) throws StopRequestException {
        InputStream in = null;
        OutputStream out = null;
        FileDescriptor outFd = null;
        try {
            try {
                in = conn.getInputStream();
            } catch (IOException e) {
                throw new StopRequestException(Status.HTTP_DATA_ERROR, e);
            }
            try {
            	out = new FileOutputStream(info.file, true);
            	outFd = ((FileOutputStream)out).getFD();
            } catch (IOException e) { e.printStackTrace();
            	throw new StopRequestException(Helpers.guessInsufficient(e) ? Status.STORAGE_INSUFFICIENT_SPACE : Status.FILE_ERROR, e);
            }
            // Start streaming data, periodically watch for pause/cancel commands and checking disk space as needed.
            transferData(info, in, out);
        } finally {
        	try {
        		if (in != null) in.close();
        	} catch (IOException e) {}
            try {
                if (out != null) out.flush();
                if (outFd != null) outFd.sync();
            } catch (IOException e) {
            } finally {
            	try {
            		if (out != null) out.close();
            	} catch (IOException e) {}
            }
        }
    }
	private void transferData(DownloadInfo info, InputStream in, OutputStream out) throws StopRequestException {
        final byte data[] = new byte[Constants.BUFFER_SIZE];
        for (;;) {
            int bytesRead = readFromResponse(info, data, in);
            if (bytesRead == -1) { // success, end of stream already reached
                handleEndOfStream(info);
                return;
            }
            info.gotData = true;
            writeDataToDestination(info, data, bytesRead, out);
            info.currentBytes += bytesRead;
            downloaderImpl.notifyDownloadRaw(this, info.copy(), data, bytesRead);
            reportProgress();
            if (Constants.LOGV)
                Log.v(Constants.TAG, "downloaded " + info.currentBytes + " for " + info.url);
            checkInterrupted();
            if (Constants.DEVEL_EMULATE_SLOW_DOWNLOADING)
            	try {Thread.sleep(Constants.DEVEL_EMULATE_SLOW_DOWNLOADING_INTERVAL);} catch (Exception e) {}
            info.tryCount = 0;
        }
    }
	private int readFromResponse(DownloadInfo info, byte[] data, InputStream entityStream) throws StopRequestException {
        try {
            return entityStream.read(data);
        } catch (IOException ex) {
            if ("unexpected end of stream".equals(ex.getMessage()))
                return -1;
            if (cannotResume(info)) {
                throw new StopRequestException(Status.HTTP_CANNOT_RESUME,
                		"Failed reading response: " + ex + "; unable to resume", ex);
            } else {
                throw new StopRequestException(Status.HTTP_DATA_ERROR,
                		"Failed reading response: " + ex, ex);
            }
        }
    }
	private boolean cannotResume(DownloadInfo info) {
        return info.currentBytes > 0 && !info.request.noIntegrity && info.headerETag == null;
    }
	private void handleEndOfStream(DownloadInfo info) throws StopRequestException {
        final boolean lengthMismatched = (info.contentLength != -1)
                && (info.currentBytes != info.contentLength);
        if (lengthMismatched) {
            if (cannotResume(info)) {
                throw new StopRequestException(Status.HTTP_CANNOT_RESUME,
                        "mismatched content length; unable to resume");
            } else {
                throw new StopRequestException(Status.HTTP_DATA_ERROR,
                        "closed socket before end of file");
            }
        }
    }
	private void writeDataToDestination(DownloadInfo info, byte[] data, int bytesRead, OutputStream out)
            throws StopRequestException {
        storageManager.verifySpaceBeforeWritingToFile(info, bytesRead);
        boolean forceVerified = false;
        while (true) {
        	checkInterrupted();
            try {
                out.write(data, 0, bytesRead);
                return;
            } catch (IOException ex) {
                if (!forceVerified) {
                    // couldn't write to file. are we out of space? check.
                    storageManager.verifySpace(info, bytesRead);
                    forceVerified = true;
                } else { ex.printStackTrace();
                	throw new StopRequestException(Helpers.guessInsufficient(ex) ? Status.STORAGE_INSUFFICIENT_SPACE : Status.FILE_ERROR, "Failed to write data: " + ex);
                }
            }
        }
    }
	
	private void setupDestinationFile() throws StopRequestException {
		// We're resuming a download that got interrupted
		File f = info.file;
		if (!f.exists()) return;
		if (Constants.LOGV) {
			Log.i(Constants.TAG, "resuming download for id: " + info.request.url +
					", and info.mFilename: " + info.file.getAbsolutePath());
		}
		long fileLength = f.length();
		if (fileLength == 0) {
			// The download hadn't actually started, we can restart from scratch
			if (Constants.LOGV) {
				Log.d(Constants.TAG, "setupDestinationFile() found fileLength=0, deleting "
						+ info.file);
			}
			f.delete();
		} else if (info.request.eTag == null && !info.request.noIntegrity) {
			// This should've been caught upon failure
			if (Constants.LOGV) {
				Log.d(Constants.TAG, "setupDestinationFile() unable to resume download, deleting "
						+ info.file.getAbsolutePath());
			}
			f.delete();
			throw new StopRequestException(Status.HTTP_CANNOT_RESUME,
					"Trying to resume a download that can't be resumed");
		} else {
			// All right, we'll be able to resume this download
			if (Constants.LOGV) {
				Log.i(Constants.TAG, "resuming download for id: " + info.request.url +
						", and starting with file of length: " + fileLength);
			}
			info.currentBytes = (int) fileLength;
			if (info.request.fileSize != -1) {
				info.contentLength = info.request.fileSize;
			}
			info.headerETag = info.request.eTag;
			info.continuingDownload = true;
			if (Constants.LOGV) {
				Log.i(Constants.TAG, "resuming download for id: " + info.url +
						", info.currentBytes: " + info.currentBytes +
						", and setting mContinuingDownload to true: ");
			}
		}
	}
	private void finalizeDestinationFile() { }
	private void cleanupDestination() {
		if (info.file != null &&
				(info.status == Status.CANCELED || info.status == Status.HTTP_CODE_from_400_to_600_UNHANDLED_HTTP_OR_SERVER)) {
            if (Constants.LOGV) {
                Log.d(Constants.TAG, "cleanupDestination() deleting " + info.file);
            }
            info.file.delete();
        }
	}
	private void checkConnectivity() throws StopRequestException {
		ConnectivityManager mConnectivityManager = (ConnectivityManager)ctx.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo mNetworkInfo = mConnectivityManager.getActiveNetworkInfo();
        if (mNetworkInfo != null && mNetworkInfo.isConnected()) return;
        throw new StopRequestException(Status.NETWORK_UNAVAILABLE);
    }
	private void reportProgress() {
		final long now = SystemClock.elapsedRealtime();
		final long sampleDelta = now - info.speedSampleStart;
        if (sampleDelta > 500) {
            final long sampleSpeed = ((info.currentBytes - info.speedSampleBytes) * 1000) / sampleDelta;
            if (info.speed == 0) {
                info.speed = sampleSpeed;
            } else {
                info.speed = ((info.speed * 3) + sampleSpeed) / 4;
                info.speedReal = true;
            }
            info.speedSampleStart = now;
            info.speedSampleBytes = info.currentBytes;
        }
        if (info.bytesNotified != info.request.fileSize &&
        		(info.currentBytes == info.request.fileSize || (info.currentBytes - info.bytesNotified > Constants.MIN_PROGRESS_STEP && now - info.timeLastNotification > Constants.MIN_PROGRESS_TIME))) {
            info.bytesNotified = info.currentBytes;
            info.timeLastNotification = now;
            downloaderImpl.notifyDownloadChanged(this, info.copy(), DownloadListener.Type.PROGRESS);
        }
	}
	private void checkInterrupted() throws StopRequestException {
        synchronized (info) {
            if (interrupt == Status.PAUSED)
                throw new StopRequestException(Status.PAUSED, "download paused");
            if (interrupt == Status.CANCELED)
                throw new StopRequestException(Status.CANCELED, "download canceled");
        }
    }
	void setInterrupted(Status status) {
		synchronized (info) {
			interrupt = status;
		}
	}
	boolean isInterrupted() {
		synchronized (info) {
			return interrupt != null;
		}
	}
	
}
