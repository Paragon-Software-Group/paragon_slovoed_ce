package shdd.android.components.news.registration;


import android.content.Context;
import android.text.TextUtils;

import shdd.android.components.news.HttpAdsClient;
import shdd.android.components.news.AdsManager;
import shdd.android.components.news.Resources;
import shdd.android.components.news.utils.ShddLog;

import java.net.URI;
import java.net.URISyntaxException;

public class ParagonGcmRegisterer {
    private static final String PREFS_KEY_PARAGON_REGISTER_ID = "PREFS_KEY_PARAGON_REGISTER_ID";

    public ParagonGcmRegisterer() {
    }

    public boolean register(String googleRegId) {
        try {
            boolean successRegAndUnreg = true;
            URI uri = new URI(HttpAdsClient.getAdsUri());
            if (isRegistered()) {
                successRegAndUnreg = HttpAdsClient.getAdsClient().unregisterToPush(uri, getRegId());
                ShddLog.d("shdd","Registration; [" + (successRegAndUnreg? "COMPLETE": "FAIL") + "] unregister previous registration_id on Paragon Ads Server");
            }

            if (successRegAndUnreg) {
                successRegAndUnreg = HttpAdsClient.getAdsClient().registerToPush(uri, googleRegId);
                ShddLog.d("shdd","Registration; [" + (successRegAndUnreg? "COMPLETE": "FAIL") + "] register new registration_id on Paragon Ads Server");
            }

            if (successRegAndUnreg) {
                setRegId(googleRegId);
                if (Resources.getters.isInTestMode()) {
                    ShddLog.e("shdd", "[ads] SIGN IN as DEVELOPER");
                }
            }
            return successRegAndUnreg;
        } catch (URISyntaxException e) {
            return false;
        }
    }

    public boolean unRegister() {
        try {
            if (!isRegistered()) {
                return true;
            }

            URI uri = new URI(HttpAdsClient.getAdsUri());
            if (HttpAdsClient.getAdsClient().unregisterToPush(uri,getRegId())) {
                ShddLog.d("shdd","Registration; [COMPLETE] unregister previous registration_id on Paragon Ads Server");
                setRegId(null);
                return true;
            } else {
                return false;
            }

        } catch (URISyntaxException e) {
            return false;
        }
    }

    private static void setRegId(String regId) {
        AdsManager.getPreferences().edit().putString(PREFS_KEY_PARAGON_REGISTER_ID, regId).apply();
    }

    public static boolean isRegistered() {
        return !TextUtils.isEmpty(getRegId());
    }

    public static String getRegId() {
        return AdsManager.getPreferences().getString(PREFS_KEY_PARAGON_REGISTER_ID, null);
    }

    public boolean isRegisteredForToken(String freshToken) {
        boolean registered = null != freshToken && freshToken.equals(getRegId());
        ShddLog.d("shdd", "ParagonGcmRegisterer.isRegisteredForToken() : " + registered + "; freshToken : " + freshToken + "; getRegId() : " + getRegId());
        return registered;
    }
}
