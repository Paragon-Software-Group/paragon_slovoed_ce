package shdd.android.components.news;

import android.content.Context;

public final class Resources {

    public static final String ADS_URI = "https://ads.penreader.com";
    public static final String ADS_STATISTIC = "https://ads.penreader.com/stats";
	public static final String SENDER_ID = "858248107703";

	public static Getters getters;

	public static enum Strings {
		HARDCODEDCONSTANTS_APPSTORE_BUILD_ADSMODIFICATIONNAME, CATALOG_OR_PRODUCT_ID, ADS_NOTIFICATION_BANNER_TITLE, LOCALE
	}

	public static enum Mode {
		STANDALONE_MODE, CONTAINER_MODE;
	}
	
	public static interface Getters {
		String getString(Strings s);
		
		boolean isInTestMode();

		boolean isAdsAgree();
		
		boolean isAllowNetwork();

		String getDeviceId();
		
		Mode getNewsMode ();
		
		/**
		 *  Comma separated product Id from pdahpc
		 */
		String getPrcs();

		/**
		 * see: https://pdahpc.penreader.com/doc/ads
		 * null - acceptable
		 */
		String getPkey();
		
        Context getAppContext();

		NotificationSpecs getNotificationSpecs();
	}
	
}
