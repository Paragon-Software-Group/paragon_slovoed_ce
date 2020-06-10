package com.paragon_software.odapi_ui;

import androidx.annotation.NonNull;

class ClientApiNotSupportedResult extends CommonErrorResult {
    ClientApiNotSupportedResult(int clientApiVersionCode, int minApiVersionCode) {
        super(Kind.WrongParams, getError(clientApiVersionCode, minApiVersionCode));
    }

    @NonNull
    @SuppressWarnings("all")
    static String getError(int clientApiVersionCode, int minApiVersionCode) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("Open Dictionary API of client with version \"");
        stringBuilder.append(clientApiVersionCode);
        stringBuilder.append("\" are not supported. Min supported version: \"");
        stringBuilder.append(minApiVersionCode);
        stringBuilder.append("\". Please, checkout latest version of the Open Dictionary API library.");
        return stringBuilder.toString();
    }
}
