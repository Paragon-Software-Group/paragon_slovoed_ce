package shdd.android.components.httpdownloader;

public enum Status {
	
	CONNECTING,
	DOWNLOADING,
	PAUSED,
	CANCELED,
	SUCCESSFULL,
	NETWORK_UNAVAILABLE,
	HTTP_DATA_ERROR,
	HTTP_CODE_500_INTERNAL_ERROR,
	HTTP_CODE_503_UNAVAILABLE,
	HTTP_CODE_from_300_to_400_UNHANDLED_REDIRECT,
	HTTP_CODE_from_400_to_600_UNHANDLED_HTTP_OR_SERVER,
	HTTP_CODE_all_UNHANDLED_OTHER,
	HTTP_CANNOT_RESUME,
	TOO_MANY_REDIRECTS,
	FILE_ERROR,
	STORAGE_INSUFFICIENT_SPACE,
	UNKNOWN_ERROR;
	
}
