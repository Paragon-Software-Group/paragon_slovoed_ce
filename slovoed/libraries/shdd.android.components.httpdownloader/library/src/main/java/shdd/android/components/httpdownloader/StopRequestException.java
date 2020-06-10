package shdd.android.components.httpdownloader;

class StopRequestException extends Exception {
	
	final Status status;
	
	StopRequestException(Status status) {
		this.status = status;
	}
	StopRequestException(Status status, String message) {
		super(message);
		this.status = status;
	}
	StopRequestException(Status status, Throwable cause) {
		super(cause);
		this.status = status;
	}
	StopRequestException(Status status, String message, Throwable cause) {
		super(message, cause);
		this.status = status;
	}
	
	static StopRequestException throwUnhandledHttpError(int code, String message) throws StopRequestException {
        String error = "Unhandled HTTP response: " + code + " " + message;
        if (code >= 400 && code < 600)
            throw new StopRequestException(Status.HTTP_CODE_from_400_to_600_UNHANDLED_HTTP_OR_SERVER, error);
        else if (code >= 300 && code < 400)
            throw new StopRequestException(Status.HTTP_CODE_from_300_to_400_UNHANDLED_REDIRECT, error);
        throw new StopRequestException(Status.HTTP_CODE_all_UNHANDLED_OTHER, error);
    }
	
}
