package shdd.android.components.news.storage;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;
import android.text.TextUtils;
import android.util.DisplayMetrics;

import shdd.android.components.news.data.AdsAd;
import shdd.android.components.news.data.AdsAd.BannerInfo;
import shdd.android.components.news.utils.ShddLog;

import java.net.URL;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Locale;

public class BannerStorage extends SQLiteOpenHelper {
	
	public static class BannerSize {
		public final int width;
		public final int height;
		
		public BannerSize(int width, int height) {
			super();
			this.width = width;
			this.height = height;
			check();
		}
		
		public BannerSize(DisplayMetrics dm) {
			this.width = Math.min(dm.widthPixels,dm.heightPixels);
			this.height = Math.max(dm.widthPixels,dm.heightPixels)/6;
			check();
		}
		
		public BannerSize(String size) {
			try {
				String[] split = size.split("x");
				width = Integer.valueOf(split[0]);
				height = Integer.valueOf(split[1]);
			} catch (NumberFormatException e) {
				throw new IllegalArgumentException("Invalid argument format:"+size);
			}
			check();
		}

		private void check() {
			if(width < 0 || height < 0 )
				throw new IllegalArgumentException("Width and height must > 0");
		}
		
		@Override
		public String toString() {
			return String.format(Locale.ENGLISH,"%dx%d", width, height);
		}
		
	}

	private static final String BANNER_DATABASE_NAME = "banner_base";
	private static final String ADS_TABLE_NAME = "ads_table";
	private static final String ADS_MESSAGE_TABLE_NAME = "ads_message_table";
	private static final String TABLE = "single";
	private static final Object SYNC_OBJECT = new Object();

	public BannerStorage(Context context) {
		super(context, BANNER_DATABASE_NAME, null, 1);
	}

	private static class AdColumn implements BaseColumns {
		public static final String ID = "id";
		public static final String START_DATE = "start_date";
		public static final String FINISH_DATE = "finish_date";
		public static final String URL = "url";
		public static final String IS_SHOWN = "is_shown";

	}

	private static class AdMessageColumn implements BaseColumns {
		public static final String AD_ID = "id";
		public static final String LANGUAGE = "language";
		public static final String LOCALE = "locale";
		public static final String URL_IMG = "url_img";
		public static final String RESOLUTION_IMG = "resolution_img";
		public static final String BYTES_IMG = "bytes_img";
	}
	
	@Override
	public void onOpen(SQLiteDatabase db) {
		super.onOpen(db);
		if (!db.isReadOnly()) {
			// Enable foreign key constraints
			db.execSQL("PRAGMA foreign_keys=ON;");
		}
	}
	
	@Override
	public void onCreate(SQLiteDatabase db) {
		super.onOpen(db);
		db.execSQL("DROP TABLE IF EXISTS " + ADS_TABLE_NAME + ";");
		db.execSQL("DROP TABLE IF EXISTS " + ADS_MESSAGE_TABLE_NAME + ";");
		db.execSQL("DROP VIEW IF EXISTS " + TABLE + ";");

		db.execSQL("CREATE TABLE " + ADS_TABLE_NAME + " ("
				+ AdColumn._ID + " INTEGER PRIMARY KEY, "
				+ AdColumn.ID + " INTEGER, " 
				+ AdColumn.START_DATE + " INTEGER, " 
				+ AdColumn.FINISH_DATE + " INTEGER, " 
				+ AdColumn.URL + " TEXT, " 
				+ AdColumn.IS_SHOWN + " INTEGER DEFAULT 0,"
				+ "UNIQUE (" + AdColumn.ID + ") ON CONFLICT REPLACE" +
				");"

		);
		db.execSQL("CREATE TABLE " + ADS_MESSAGE_TABLE_NAME + " ("
				+ AdMessageColumn._ID + " INTEGER PRIMARY KEY, "
				+ AdMessageColumn.LANGUAGE + " TEXT, "
				+ AdMessageColumn.LOCALE + " TEXT, "
				+ AdMessageColumn.URL_IMG + " TEXT, "
				+ AdMessageColumn.RESOLUTION_IMG + " TEXT, "
				+ AdMessageColumn.BYTES_IMG + " BLOB NOT NULL, "
				+ AdMessageColumn.AD_ID + " INTEGER, "
				+ "FOREIGN KEY(" + AdMessageColumn.AD_ID + ") "
				+ " REFERENCES " + ADS_TABLE_NAME + "(" + AdColumn.ID
				+ ") ON DELETE CASCADE ON UPDATE CASCADE," + "UNIQUE ("
				+ AdMessageColumn.AD_ID + ", "
				+ AdMessageColumn.LOCALE + ") ON CONFLICT REPLACE" +
				");"

		);
		db.execSQL("CREATE VIEW IF NOT EXISTS " + TABLE + " AS " + " SELECT "
				+ ADS_MESSAGE_TABLE_NAME + "." + AdMessageColumn._ID + ", "
				+ ADS_TABLE_NAME + "." + AdColumn.ID + ", "
				+ ADS_TABLE_NAME + "." + AdColumn.START_DATE + ", "
				+ ADS_TABLE_NAME + "." + AdColumn.FINISH_DATE + ", "
				+ ADS_TABLE_NAME + "." + AdColumn.URL + ", "
				+ ADS_TABLE_NAME + "." + AdColumn.IS_SHOWN + ", "
				+ ADS_MESSAGE_TABLE_NAME + "." + AdMessageColumn.URL_IMG + ", "
				+ ADS_MESSAGE_TABLE_NAME + "." + AdMessageColumn.RESOLUTION_IMG + ", "
				+ ADS_MESSAGE_TABLE_NAME + "." + AdMessageColumn.BYTES_IMG + ", "
				+ ADS_MESSAGE_TABLE_NAME + "." + AdMessageColumn.LANGUAGE + ", "
				+ ADS_MESSAGE_TABLE_NAME + "." + AdMessageColumn.LOCALE 
				+ " " +
				" FROM " + ADS_TABLE_NAME + " INNER JOIN "
				+ ADS_MESSAGE_TABLE_NAME + " ON " + ADS_TABLE_NAME + "."
				+ AdColumn.ID + "=" + ADS_MESSAGE_TABLE_NAME + "."
				+ AdMessageColumn.AD_ID + ";");

	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) { }

	public void clear() {
		onCreate(getWritableDatabase());
	}
	
	public void insert(AdsAd ad) {
		insert(Collections.singletonList(ad));
	}
	
	/**
	 *	If Ad exist - replace all data 
	 */
	public void insert(List<AdsAd> ads) {
		synchronized (SYNC_OBJECT) {
			
			SQLiteDatabase db = getWritableDatabase();
			ContentValues values = new ContentValues();
	
			db.beginTransaction();
			try {
				for (AdsAd ad : ads) {
					
					values.clear();
					values.put(AdColumn.ID, ad.getmId());
					values.put(AdColumn.START_DATE, ad.getmStartDate().getTime());
					values.put(AdColumn.FINISH_DATE, ad.getmFinishDate().getTime());
					values.put(AdColumn.URL, ad.getmAdUrl() == null? "": ad.getmAdUrl().toString());
					values.put(AdColumn.IS_SHOWN, ad.isShown()? "1": "0");
					if( db.insert(ADS_TABLE_NAME, null, values) < 0 ) {
						ShddLog.d("shdd","Cant insert to table:"+ADS_TABLE_NAME+"; AdsId:"+ad.getmId());
					}
						
					
					for (String locale : ad.getAllLocales()) {
						BannerInfo message = ad.getMessageByLocale(locale);
						
						values.clear();
						values.put(AdMessageColumn.AD_ID, ad.getmId());
						values.put(AdMessageColumn.LANGUAGE, message.getLanguage());
						values.put(AdMessageColumn.LOCALE, message.getLocale());
						values.put(AdMessageColumn.URL_IMG, message.getURL().toString());
						values.put(AdMessageColumn.RESOLUTION_IMG, message.getResolutionImg());
						values.put(AdMessageColumn.BYTES_IMG, message.getBytesImg());
						if(db.insert(ADS_MESSAGE_TABLE_NAME, null, values) < 0 ) {
							ShddLog.d("shdd","Cant insert to table:"+ADS_MESSAGE_TABLE_NAME+"; AdsId:"+ad.getmId()+
									"; imgUrl:"+message.getURL().toString()+" (Possible image load failed)");
						}
							
					}
				}
	
				db.setTransactionSuccessful();
			} finally {
				db.endTransaction();
			}
			db.close();
			
		}
	}
	
	/**
	 * Update ONLY metadata of Ad. <br/>
	 * NOT CHANGE - IS_SHOWN <br/>
	 * NOT CHANGE - rows in table ADS_MESSAGE_TABLE_NAME <br/>
	 * @param ad - to get ID of Ad
	 */
	public void updateMetadata (AdsAd ad) {
		synchronized (SYNC_OBJECT) {
			SQLiteDatabase db = getWritableDatabase();
			ContentValues values = new ContentValues();
	
			db.beginTransaction();
			try {
					values.clear();
					values.put(AdColumn.ID, ad.getmId());
					values.put(AdColumn.START_DATE, ad.getmStartDate().getTime());
					values.put(AdColumn.FINISH_DATE, ad.getmFinishDate().getTime());
					values.put(AdColumn.URL, ad.getmAdUrl() == null? "": ad.getmAdUrl().toString());
					db.update(ADS_TABLE_NAME, values, AdColumn.ID + " = ?", new String[] { String.valueOf(ad.getmId()) });
	
				db.setTransactionSuccessful();
			} finally {
				db.endTransaction();
			}
			db.close();
		}
	}
	/**
	 * Return current Ad in storage with id == ad.getmId() <br/>
	 * @return Return null if not exist 
	 */
	public AdsAd getStorageAd(AdsAd ad, boolean loadImgBytes) {
		synchronized (SYNC_OBJECT) {
			Integer adId = ad.getmId();
			SQLiteDatabase db = getReadableDatabase();
			Cursor adsMessageCursor = db.query(TABLE, null, AdColumn.ID + " = ?", new String[] {adId.toString()}, null, null, AdColumn.START_DATE + " DESC");
			adsMessageCursor.moveToFirst();
			AdsAd ads = cursorToAdsAd(adsMessageCursor, loadImgBytes);
			adsMessageCursor.close();
			db.close();
			return ads;
		}
	}

	private AdsAd cursorToAdsAd(Cursor cursor, boolean loadImgBytes) {
		if(cursor.getCount() <= 0)
			return null;
		
		try {
			int adId = cursor.getInt(cursor.getColumnIndex(AdColumn.ID));
			long startDate = cursor.getLong(cursor.getColumnIndex(AdColumn.START_DATE));
			long finishDate = cursor.getLong(cursor.getColumnIndex(AdColumn.FINISH_DATE));
			String urlStr = cursor.getString(cursor.getColumnIndex(AdColumn.URL));
			URL adUrl = TextUtils.isEmpty(urlStr)? null: new URL(urlStr);
			int isShown = cursor.getInt(cursor.getColumnIndex(AdColumn.IS_SHOWN));
				
			String language = cursor.getString(cursor.getColumnIndex(AdMessageColumn.LANGUAGE));
			String locale = cursor.getString(cursor.getColumnIndex(AdMessageColumn.LOCALE));
			URL urlImage = new URL(cursor.getString(cursor.getColumnIndex(AdMessageColumn.URL_IMG)));
			String resolutionImg = cursor.getString(cursor.getColumnIndex(AdMessageColumn.RESOLUTION_IMG));
			
			BannerInfo bannerInfo = new BannerInfo(adId,language, locale, urlImage);
			bannerInfo.setResolutionImg(resolutionImg);
			
			if(loadImgBytes)
				bannerInfo.setBytesImg(cursor.getBlob(cursor.getColumnIndex(AdMessageColumn.BYTES_IMG)));
			
			AdsAd adsAd = new AdsAd();
			adsAd.setmId(adId);
			adsAd.setmStartDate(new Date(startDate));
			adsAd.setmFinishDate(new Date(finishDate));
			adsAd.setmAdUrl(adUrl);
			adsAd.setIsShown(isShown == 0? false: true);
			adsAd.addMessage(bannerInfo);
			
			return adsAd;
		} catch (Exception e) {
			ShddLog.e("shdd", e.toString());
			return null;
		}		
	}
	
	public int getCount() {
		synchronized (SYNC_OBJECT) {
			SQLiteDatabase db = getReadableDatabase();
			Cursor cursor = db.query(TABLE, null, null, null, null, null, null);
			int count = cursor.getCount();
			cursor.close();
			db.close();
			return count;
		}
	}
	
	public int getCount(String tableName) {
		synchronized (SYNC_OBJECT) {
			SQLiteDatabase db = getReadableDatabase();
			Cursor cursor = db.query(tableName, null, null, null, null, null, null);
			int count = cursor.getCount();
			cursor.close();
			db.close();
			return count;
		}
	}
	
	public int deleteFinishedAds(long currentDate) {
		synchronized (SYNC_OBJECT) {
			int delCount = -1;
			SQLiteDatabase db = getWritableDatabase();
			db.beginTransaction();
			try {
				delCount = db.delete(ADS_TABLE_NAME, AdColumn.FINISH_DATE+" < ?", new String[]{ String.valueOf(currentDate) });
				db.setTransactionSuccessful();
			} finally {
				db.endTransaction();
			}
			db.close();
			return delCount;
		}
	}
	
	public int delete ( AdsAd ad ) {
		synchronized (SYNC_OBJECT) {
			int delCount = -1;
			SQLiteDatabase db = getWritableDatabase();
			db.beginTransaction();
			try {
				delCount = db.delete(ADS_TABLE_NAME, AdColumn.ID+" = ?", new String[]{ String.valueOf(ad.getmId()) });
				db.setTransactionSuccessful();
			} finally {
				db.endTransaction();
			}
			db.close();
			return delCount;
		}
	}

	public static boolean isBannerUrlOrLocaleChanged(AdsAd lhs, AdsAd rhs) {
		BannerInfo lMsg = lhs.getMessageByLocale(null);
		BannerInfo rMsg = rhs.getMessageByLocale(null);
		return !lMsg.getURL().equals(rMsg.getURL()) || !lMsg.getLocale().equals(rMsg.getLocale());
	}

	public AdsAd getActiveBanner(long curTime) {
		synchronized (SYNC_OBJECT) {
			SQLiteDatabase db = getReadableDatabase();
			Cursor adsMessageCursor = db.query(TABLE, null, AdColumn.START_DATE + " < ? AND ? < " + AdColumn.FINISH_DATE , new String[] {String.valueOf(curTime),String.valueOf(curTime)}, null, null, AdColumn.ID + " DESC");
			adsMessageCursor.moveToFirst();
			AdsAd activeAd = cursorToAdsAd(adsMessageCursor, true);
			db.close();
			adsMessageCursor.close();
			return activeAd;
		}
	}

	public AdsAd getActiveBannerToNotify(long curTime) {
		AdsAd activeBanner = getActiveBanner(curTime);
		return activeBanner == null || activeBanner.isShown()? null : activeBanner;
	}
	
	public void updateAdShownState(AdsAd ad) {
		synchronized (SYNC_OBJECT) {
			
			SQLiteDatabase db = getWritableDatabase();
			ContentValues values = new ContentValues();
	
			db.beginTransaction();
			try {
					values.clear();
					values.put(AdColumn.IS_SHOWN, ad.isShown()? "1": "0");
					db.update(ADS_TABLE_NAME, values, AdColumn.ID + " = ?", new String[] { String.valueOf(ad.getmId()) });
	
				db.setTransactionSuccessful();
			} finally {
				db.endTransaction();
			}
			db.close();
		}
	}

}
