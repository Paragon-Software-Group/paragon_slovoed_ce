package com.paragon_software.news_manager;

import android.database.Cursor;

import java.io.Serializable;

public class NewsItem implements Serializable {
	private static final long serialVersionUID = 1L;
	private final int mId;
	private final long mDate;
	private final String mBody;
	private final String mTitle;
	private final String mLocale;
	private boolean isRead;

	NewsItem(int id, long date, String body, String title, String locale, boolean isRead) {
		this.mId = id;
		this.mDate = date;
		this.mBody = body;
		this.mTitle = title;
		this.mLocale = locale;
		this.isRead = isRead;
	}

	static NewsItem createFrom(Cursor cursor) {
		int id = cursor.getInt(cursor.getColumnIndex("id"));
		long date = cursor.getLong(cursor.getColumnIndex("date"));
		String body = cursor.getString(cursor.getColumnIndex("body"));
		String title = cursor.getString(cursor.getColumnIndex("title"));
		String locale = cursor.getString(cursor.getColumnIndex("locale"));
		boolean isRead = cursor.getInt(cursor.getColumnIndex("is_read")) == 1;
		return new NewsItem(id, date, body, title, locale, isRead);
	}

	public int getId() {
		return this.mId;
	}

	public long getDate() {
		return this.mDate;
	}

	public String getBody() {
		return this.mBody;
	}

	public String getTitle() {
		return this.mTitle;
	}

	public String getLocale() {
		return this.mLocale;
	}

	public boolean isRead() {
		return this.isRead;
	}
}
