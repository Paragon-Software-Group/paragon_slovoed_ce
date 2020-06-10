package shdd.android.components.httpdownloader;

import android.os.Build;
import android.text.TextUtils;

class Constants {
	
	static final boolean LOGV = false;
	static final String TAG = "shdd";
	
	static final int BUFFER_SIZE = 4096;
	
	static final int MAX_DOWNLOADS = 6;
	
	static final long MAX_REDIRECTS = 5;
    
    static final int RETRY_DELAY_MS = 2500;
    static final int MAX_RETRIES = 6;
    
    /** The minimum amount of progress that has to be done before the progress bar gets updated */
    static final int MIN_PROGRESS_STEP = 4096;
    /** The minimum amount of time that has to elapse before the progress bar gets updated, in ms */
    static final long MIN_PROGRESS_TIME = 1500;
	
	/** The default user agent used for downloads */
    static final String DEFAULT_USER_AGENT;
    
    static boolean DEVEL_EMULATE_SLOW_DOWNLOADING = false;
    static final long DEVEL_EMULATE_SLOW_DOWNLOADING_INTERVAL = 300;

    static {
        final StringBuilder builder = new StringBuilder();

        final boolean validRelease = !TextUtils.isEmpty(Build.VERSION.RELEASE);
        final boolean validId = !TextUtils.isEmpty(Build.ID);
        final boolean includeModel = "REL".equals(Build.VERSION.CODENAME)
                && !TextUtils.isEmpty(Build.MODEL);

        builder.append("SHDD HTTP Download Component");
        if (validRelease) {
            builder.append("/").append(Build.VERSION.RELEASE);
        }
        builder.append(" (Linux; U; Android");
        if (validRelease) {
            builder.append(" ").append(Build.VERSION.RELEASE);
        }
        if (includeModel || validId) {
            builder.append(";");
            if (includeModel) {
                builder.append(" ").append(Build.MODEL);
            }
            if (validId) {
                builder.append(" Build/").append(Build.ID);
            }
        }
        builder.append(")");

        DEFAULT_USER_AGENT = builder.toString();
    }

}
