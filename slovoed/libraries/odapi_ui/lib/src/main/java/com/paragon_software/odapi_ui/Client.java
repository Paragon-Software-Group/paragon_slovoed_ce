package com.paragon_software.odapi_ui;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

class Client {
    static ParamParser.ParamFactory<Client> FACTORY = new ParamParser.ParamFactory<Client>() {
        @NonNull
        @Override
        public Client create(@Nullable String paramString) {
            return new Client(paramString);
        }
    };

    private static final String META_OPEN_DICTIONARY_API_MIN_VERSION_CODE = "open.dictionary.api.minVersionCode";

    static int getMinApiVersionCode(Context context) throws Exception {
        if(context == null)
            throw new IllegalStateException();
        String packageName = context.getPackageName();
        PackageManager packageManager = context.getPackageManager();
        ApplicationInfo applicationInfo = packageManager.getApplicationInfo(packageName, PackageManager.GET_META_DATA);
        return applicationInfo.metaData.getInt(META_OPEN_DICTIONARY_API_MIN_VERSION_CODE);
    }

    private final int mApiVersionCode;

    private Client(@Nullable String client) {
        Uri uri = Uri.parse(client);
        mApiVersionCode = Integer.parseInt(uri.getQueryParameter("open_dictionary_api_version_code"));
    }

    int getApiVersionCode() {
        return mApiVersionCode;
    }
}
