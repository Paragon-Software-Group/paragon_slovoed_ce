package shdd.android.components.news.data;

import java.io.Serializable;
import java.net.URL;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Set;


/**
 *   AdsAd содержит информацию о конкретной акции - время начала-конца акции, id и т.д.
 *  Смотреть также https://pdahpc.penreader.com/doc/ads
 *
 *
 */
public class AdsAd implements Comparable<AdsAd>, Serializable {
	private static final long serialVersionUID = 1L;
	private final Locale DEFAULT_LOCALE = Locale.ENGLISH;
	private int mId;
	private Date mStartDate;
	private Date mFinishDate;
	private URL mAdUrl;
	private boolean mIsShown;
	
	private Map<String, BannerInfo> mMessageUrl = new HashMap<String,BannerInfo>();

    /**
     * Информация о баннере
     */
	public static class BannerInfo implements Serializable {
		private static final long serialVersionUID = 1L;
		private final String mLanguage;
		private final String mLocale;
		private final URL mURl;
		private final int mAdId;
		private String mResolutionImg;
		private byte[] mBytesImg;
		
		public BannerInfo (int adId, String lang, String locale, URL url) {
			mAdId = adId;
			mLanguage = lang;
			mLocale = locale;
			mURl = url;
		}

        /**
         *
         * @return Английское название языка (например, Russian, German), как используется в sou
         */
		public String getLanguage() {
			return mLanguage;
		}

        /**
         *
         * @return двухбуквенный код языка (ISO 639-1),
         */
		public String getLocale() {
			return mLocale;
		}

        /**
         *
         * @return http ссылка, по которой можно загрузить изображение-баннер. клиент должен загрузить ее отдельным запросом. см. также запрос на скачивание картинки-баннера.
         */
		public URL getURL() {
			return mURl;
		}

		public String getResolutionImg() {
			return mResolutionImg;
		}

		public void setResolutionImg(String resolutionImg) {
			this.mResolutionImg = resolutionImg;
		}

		public byte[] getBytesImg() {
			return mBytesImg;
		}

		public void setBytesImg(byte[] bytesImg) {
			this.mBytesImg = bytesImg;
		}
		
	}
	
	public int getmId() {
		return mId;
	}
	public void setmId(int mId) {
		this.mId = mId;
	}

    /**
     *
     * @return Дата начала рекламной акции
     */
	public Date getmStartDate() {
		return mStartDate;
	}
	public void setmStartDate(Date mStartDate) {
		this.mStartDate = mStartDate;
	}

    /**
     *
     * @return   Дата окончания рекламной акции
     */
	public Date getmFinishDate() {
		return mFinishDate;
	}
	public void setmFinishDate(Date mFinishDate) {
		this.mFinishDate = mFinishDate;
	}
	/**
	 * 
	 * @return URL по которому переходим при нажатии на баннер
	 */
	public URL getmAdUrl() {
		return mAdUrl;
	}
	public void setmAdUrl(URL mAdUrl) {
		this.mAdUrl = mAdUrl;
	}
	/**
	 *
	 * @param locale - двухбуквенный код языка (ISO 639-1)
	 * @return информацию по баннеру
	 */
	public BannerInfo getMessageByLocale(String locale) {
		
		BannerInfo info = mMessageUrl.get(locale);
		if (info == null) {
			info = mMessageUrl.get(DEFAULT_LOCALE.getLanguage());
		}
		if (info == null && !mMessageUrl.isEmpty()) {
			info = mMessageUrl.values().iterator().next();
		}
		return info;
	}
	
	public BannerInfo addMessage(int adId,String locale, String lang, URL url) {
		 return mMessageUrl.put(locale, new BannerInfo(adId,lang, locale, url));		
	}
	
	public BannerInfo addMessage(BannerInfo banner) {
		 return mMessageUrl.put(banner.getLocale(), banner);		
	}
	
	@Override
	public String toString() {
		return "AdsAd [mId=" + mId + ", mAdUrl=" + mAdUrl +  "]";
	}
	@Override
	public boolean equals(Object o) {
		return o != null && o instanceof AdsAd && mId == ((AdsAd)o).mId;
	}
	
	@Override
	public int hashCode() {
		 return mId;
		
	}
	@Override
	public int compareTo(AdsAd another) {
		return mId - another.getmId();
	}
	public boolean isShown() {
		return mIsShown;
	}
	public AdsAd setIsShown(boolean isShown) {
		this.mIsShown = isShown;
		return this;
	}
	public Set<String> getAllLocales() {
		return mMessageUrl.keySet();
	}

}
