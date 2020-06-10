package shdd.android.components.httpdownloader;

import java.io.File;
import java.util.HashMap;
import java.util.LinkedList;

import android.util.Pair;

public class Request {
	
	public String url;
	public File destDir;
	public String destFilename;
	public long fileSize;
	public boolean showNotification;
	public String userAgent;
	public LinkedList<Pair<String, String>> headers;
	public boolean noIntegrity;
	public String eTag;
	public HashMap<String, Object> meta;
	
	public Request() {
		meta = new HashMap<String, Object>(2);
	}
	
	@Override
	public boolean equals(Object o) {
		return o instanceof Request && url.equals(((Request)o).url);
	}
	@Override
	public int hashCode() {
		return url.hashCode();
	}
	@Override
	public String toString() {
		return url;
	}
	
	private static Request oneTime = new Request();
	public static Request oneTime(String url) {
		oneTime.url = url;
		return oneTime;
	}
	
}
