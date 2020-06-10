package com.paragon_software.odapi_ui;

import androidx.annotation.NonNull;

class WrongParamsResult extends CommonErrorResult {
    enum What {
        Missed("missed"),
        Illegal("illegal");

        What(@NonNull String what) {
            mWhat = what;
        }

        @NonNull
        private final String mWhat;

        @NonNull
        String getString() {
            return mWhat;
        }
    }

    WrongParamsResult(@NonNull String error) {
        super(Kind.WrongParams, error);
    }

    WrongParamsResult(@NonNull String param, @NonNull What what) {
        this("\"" + param + "\" param is " + what.getString());
    }
}
