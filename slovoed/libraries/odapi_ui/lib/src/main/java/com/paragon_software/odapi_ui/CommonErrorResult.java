package com.paragon_software.odapi_ui;

import androidx.annotation.NonNull;

class CommonErrorResult extends ErrorResult {
    enum Kind {
        DatabaseIsUnavailable("A database of words is not available"),
        InternalError("An internal error"),
        WrongParams("Incorrect parameters"),
        AppStateError("Error state of the application");

        @NonNull
        private final String mError;

        Kind(@NonNull String error) {
            mError = error;
        }

        @NonNull
        String getError() {
            return mError;
        }
    }

    CommonErrorResult(@NonNull Kind kind, @NonNull String error) {
        super(kind.getError(), error);
    }
}
