package com.paragon_software.odapi_ui;

import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

final class ParamParser {
    static class MissingParamException extends Exception { }

    static class IllegalParamException extends Exception { }

    interface ParamFactory<T> {
        @NonNull
        T create(@Nullable String paramString) throws Exception;
    }

    static ParamFactory<String> IDENTITY = new ParamFactory<String>() {
        @NonNull
        @Override
        public String create(@Nullable String paramString) throws Exception {
            if(paramString != null)
                return paramString;
            else
                throw new IllegalParamException();
        }
    };

    @NonNull
    static <T> T parse(@NonNull Bundle bundle, @NonNull String key, @NonNull ParamFactory<T> paramFactory)
            throws MissingParamException, IllegalParamException {
        if(bundle.containsKey(key))
            try {
                return paramFactory.create(bundle.getString(key));
            }
            catch (Exception e) {
                throw new IllegalParamException();
            }
        else
            throw new MissingParamException();
    }
}
