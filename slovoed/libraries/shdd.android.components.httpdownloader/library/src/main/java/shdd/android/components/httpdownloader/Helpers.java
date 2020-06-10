package shdd.android.components.httpdownloader;

import java.net.URLConnection;
import java.util.Locale;

import android.os.Looper;

class Helpers {
	
	static long getHeaderFieldLong(URLConnection conn, String field, long defaultValue) {
        try {
            return Long.parseLong(conn.getHeaderField(field));
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }
	
	static String normalizeMimeType(String type) {//from Intent.java of 17 api level
        if (type == null)
            return null;
        type = type.trim().toLowerCase(Locale.US);
        final int semicolonIndex = type.indexOf(';');
        if (semicolonIndex != -1)
            type = type.substring(0, semicolonIndex);
        return type;
    }
	
	static boolean isStatusRetryable(Status status) {
        switch (status) {
        	case NETWORK_UNAVAILABLE:
            case HTTP_DATA_ERROR:
            case HTTP_CODE_503_UNAVAILABLE:
            case HTTP_CODE_500_INTERNAL_ERROR:
                return true;
            default:
                return false;
        }
    }
	
	static void assertMainTread() throws IllegalAccessError {
		if (Looper.myLooper() != Looper.getMainLooper())
			throw new IllegalAccessError("Call must be from Main thread!");
	}
	
	static boolean guessInsufficient(Exception e) { String message;
		if (e == null || (message = e.getMessage()) == null) return false;
		message = message.toLowerCase(Locale.ENGLISH);
		return message.contains(" space") || message.contains(" left") || message.contains(" enough") || message.contains(" free");
	}
	
}
