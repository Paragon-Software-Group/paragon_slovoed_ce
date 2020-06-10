package com.paragon_software.utils_slovoed.device;

import android.content.Context;
import android.os.Build;
import androidx.annotation.NonNull;
import android.text.TextUtils;

import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.utils_slovoed.R;

import java.util.UUID;

public final class DeviceUtils {
    private static final String INSTALLATION_ID_KEY = "com.paragon_software.utils_slovoed.device.INSTALLATION_ID_KEY";
    private static final String DEVICE_MODEL_KEY = "com.paragon_software.utils_slovoed.device.DEVICE_MODEL_KEY";

    private static final String UNKNOWN_DEVICE = "Unknown Device";

    @NonNull
    private static String mInstallationId = "";
    @NonNull
    private static String mDeviceModel = "";
    private static boolean isTablet;

    private DeviceUtils() {}

    public static void init( @NonNull Context context)
    {
        isTablet = context.getResources().getBoolean(R.bool.isTablet);
    }

    //          https://developer.android.com/training/articles/user-data-ids#java
    @NonNull
    public static synchronized String getInstallationId(@NonNull SettingsManagerAPI settingsManager) {
        if(mInstallationId.isEmpty()) {
            mInstallationId = load(settingsManager, INSTALLATION_ID_KEY);
            if(mInstallationId.isEmpty()) {
                mInstallationId = UUID.randomUUID().toString();
                save(settingsManager, INSTALLATION_ID_KEY, mInstallationId);
            }
        }
        return mInstallationId;
    }

    @NonNull
    public static synchronized String getDeviceModel(@NonNull SettingsManagerAPI settingsManager) {
        if(mDeviceModel.isEmpty()) {
            mDeviceModel = load(settingsManager, DEVICE_MODEL_KEY);
            if(mDeviceModel.isEmpty()) {
                mDeviceModel = getDeviceName().trim();
                if(mDeviceModel.isEmpty())
                    mDeviceModel = UNKNOWN_DEVICE;
                save(settingsManager, DEVICE_MODEL_KEY, mDeviceModel);
            }
        }
        return mDeviceModel;
    }

    private static void save(@NonNull SettingsManagerAPI settingsManager, @NonNull String key, @NonNull String value) {
        try {
            settingsManager.save(key, value, true);
        }
        catch (ManagerInitException | LocalResourceUnavailableException ignore) { }
    }

    @NonNull
    private static String load(@NonNull SettingsManagerAPI settingsManager, @NonNull String key) {
        String res = "";
        try {
            res = settingsManager.load(key, "");
        }
        catch (WrongTypeException | ManagerInitException ignore) { }
        return res;
    }

    //          https://stackoverflow.com/questions/1995439/get-android-phone-model-programmatically
    /** Returns the consumer friendly device name */
    @NonNull
    private static String getDeviceName() {
        String manufacturer = Build.MANUFACTURER;
        String model = Build.MODEL;
        String res = "";
        if(!TextUtils.isEmpty(model)) {
            boolean needManufacturer = !TextUtils.isEmpty(manufacturer);
            if(needManufacturer)
                needManufacturer = !model.startsWith(manufacturer);
            if(needManufacturer)
                res = capitalize(manufacturer) + " " + model;
            else
                res = capitalize(model);
        }
        return res;
    }

    @NonNull
    private static String capitalize(@NonNull String str) {
        char[] arr = str.toCharArray();
        boolean capitalizeNext = true;
        StringBuilder phrase = new StringBuilder();
        for (char c : arr) {
            if (capitalizeNext && Character.isLetter(c)) {
                phrase.append(Character.toUpperCase(c));
                capitalizeNext = false;
                continue;
            } else if (Character.isWhitespace(c))
                capitalizeNext = true;
            phrase.append(c);
        }
        return phrase.toString();
    }

    public static boolean isTablet()
    {
        return isTablet;
    }
}
