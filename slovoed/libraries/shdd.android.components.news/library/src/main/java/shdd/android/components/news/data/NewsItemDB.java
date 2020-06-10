package shdd.android.components.news.data;

import android.database.Cursor;

import shdd.android.components.news.storage.NewsStorage;

import java.io.Serializable;

public class NewsItemDB implements Serializable{
	private static final long serialVersionUID = 1L;
	private final  int mId;
	/**
	 * unix time in second
	 */
	private final long mDate;
	private final String mBody;
	private final String mTitle;
	private final String mLocale;
	private boolean isread; 

	public NewsItemDB(int mId, long mDate, String mBody, String mTitle,
			String mLocale,boolean isread) {
		super();
		this.mId = mId;
		this.mDate = mDate;
		this.mBody = mBody;
		this.mTitle = mTitle;
		this.mLocale = mLocale;
		this.isread = isread;
	}
	
	public static NewsItemDB createFrom(Cursor cursor) {
		
		int id = cursor.getInt(cursor.getColumnIndex(NewsStorage.ID_COLUMN));
		long date = cursor.getLong(cursor.getColumnIndex(NewsStorage.DATE_COLUMN));
		String body = cursor.getString(cursor.getColumnIndex(NewsStorage.BODY_COLUMN));
		String title = cursor.getString(cursor.getColumnIndex(NewsStorage.TITLE_COLUMN));
		String locale = cursor.getString(cursor.getColumnIndex(NewsStorage.LOCALE_COLUMN));
		boolean isread = cursor.getInt(cursor.getColumnIndex(NewsStorage.IS_READ_COLUMN)) == 1;
		return new NewsItemDB(id, date, body, title, locale,isread);
	}

	public int getId() {
		return mId;
	}

	public long getDate() {
		return mDate;
	}

	public String getBody() {
		return mBody;
	}

	public String getTitle() {
		return mTitle;
	}

	public String getLocale() {
		return mLocale;
	}
	public boolean isRead() {
		return isread;
	}
	public void setIsRead(boolean isread) {
		this.isread = isread;
	}
	
	
}
