package shdd.android.components.news.storage;

import java.util.ArrayList;
import java.util.Collection;

import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.net.Uri;
import android.provider.BaseColumns;

import shdd.android.components.news.data.NewsItem;
import shdd.android.components.news.data.NewsItem.Message;
import shdd.android.components.news.data.NewsItemDB;

/**
 * Класс для хранения новостей
 * 
 * @author privezentsev
 * 
 */
public class NewsStorage extends SQLiteOpenHelper {
	public final static String NEWS_DATABASE_NAME = "news_base";

	public final static String TABLE = "single";
	protected final static String NEWS_TABLE_NAME = "news";
	protected final static String MESSAGE_TABLE_NAME = "message";
	private Context mContext;
	private SharedPreferences mNewsPrefs;

	private static final String LAST_UPDATE = "last_update";
	private static final Object SYNC_OBJECT = new Object();

	static final String NEWS_PREFERENCES = "news_prefs";

	public NewsStorage(Context context) {
		super(context, NEWS_DATABASE_NAME, null, 1);
		mContext = context;
		mNewsPrefs = context.getSharedPreferences(NewsStorage.NEWS_PREFERENCES,
				Context.MODE_PRIVATE);

	}

	private static class NewsItemColumn implements BaseColumns {

		public static final String ID = "id";
		public static final String DATE = "date";
		public static final String IS_READ = "is_read";

	}

	private static class NewsItemMessageColumns implements BaseColumns {
		public static final String NEWS_ID = "id";
		public static final String LANGUAGE = "language";
		public static final String LOCALE = "locale";
		public static final String TITLE = "title";
		public static final String BODY = "body";
	}

	/**
	 * news id
	 */
	public final static String ID_COLUMN = NewsItemColumn.ID;
	/**
	 * Date of news. Represent as integer in unix time seconds
	 */
	public final static String DATE_COLUMN = NewsItemColumn.DATE;
	/**
	 * news title
	 */
	public final static String TITLE_COLUMN = NewsItemMessageColumns.TITLE;
	/**
	 * news body
	 */
	public final static String BODY_COLUMN = NewsItemMessageColumns.BODY;
	/**
	 * news locale
	 */
	public final static String LOCALE_COLUMN = NewsItemMessageColumns.LOCALE;

	/**
	 * news reading status - read or not
	 */
	public final static String IS_READ_COLUMN = NewsItemColumn.IS_READ;

	@Override
	public void onOpen(SQLiteDatabase db) {
		super.onOpen(db); // To change body of overridden methods use File |
							// Settings | File Templates.
		if (!db.isReadOnly()) {
			// Enable foreign key constraints
			db.execSQL("PRAGMA foreign_keys=ON;");
		}
	}

	@Override
	public void onCreate(SQLiteDatabase db) {
		super.onOpen(db);
		db.execSQL("DROP TABLE IF EXISTS " + NEWS_TABLE_NAME + ";");
		db.execSQL("DROP TABLE IF EXISTS " + MESSAGE_TABLE_NAME + ";");
		db.execSQL("DROP VIEW IF EXISTS " + TABLE + ";");

		db.execSQL("CREATE TABLE " + NEWS_TABLE_NAME + " ("
				+ NewsItemColumn._ID + " INTEGER PRIMARY KEY, "
				+ NewsItemColumn.ID + " INTEGER, " + NewsItemColumn.DATE
				+ " INTEGER, " + NewsItemColumn.IS_READ + " INTEGER DEFAULT 0,"
				+ "UNIQUE (" + NewsItemColumn.ID + ") ON CONFLICT REPLACE" +

				");"

		);
		db.execSQL("CREATE TABLE " + MESSAGE_TABLE_NAME + " ("
				+ NewsItemMessageColumns._ID + " INTEGER PRIMARY KEY, "
				+ NewsItemMessageColumns.BODY + " TEXT, "
				+ NewsItemMessageColumns.LANGUAGE + " TEXT, "
				+ NewsItemMessageColumns.LOCALE + " TEXT, "
				+ NewsItemMessageColumns.TITLE + " TEXT, "
				+ NewsItemMessageColumns.NEWS_ID + " INTEGER, "
				+ "FOREIGN KEY(" + NewsItemMessageColumns.NEWS_ID + ") "
				+ " REFERENCES " + NEWS_TABLE_NAME + "(" + NewsItemColumn.ID
				+ ") ON DELETE CASCADE ON UPDATE CASCADE," + "UNIQUE ("
				+ NewsItemMessageColumns.NEWS_ID + ", "
				+ NewsItemMessageColumns.LOCALE + ") ON CONFLICT REPLACE" +

				");"

		);
		db.execSQL("CREATE VIEW IF NOT EXISTS " + TABLE + " AS " + " SELECT "
				+ MESSAGE_TABLE_NAME + "." + NewsItemColumn._ID + ", "
				+ NEWS_TABLE_NAME + "." + NewsItemColumn.ID + ", "
				+ NEWS_TABLE_NAME + "." + NewsItemColumn.DATE + ", "
				+ NEWS_TABLE_NAME + "." + NewsItemColumn.IS_READ + ", "
				+ MESSAGE_TABLE_NAME + "." + NewsItemMessageColumns.TITLE
				+ ", " + MESSAGE_TABLE_NAME + "." + NewsItemMessageColumns.BODY
				+ ", " + MESSAGE_TABLE_NAME + "."
				+ NewsItemMessageColumns.LOCALE + " " +

				" FROM " + NEWS_TABLE_NAME + " INNER JOIN "
				+ MESSAGE_TABLE_NAME + " ON " + NEWS_TABLE_NAME + "."
				+ NewsItemColumn.ID + "=" + MESSAGE_TABLE_NAME + "."
				+ NewsItemMessageColumns.NEWS_ID + ";");

	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
	}

	public void clear() {
		onCreate(getWritableDatabase());
	}
	
	/**
	 * Добавление списка новостей. При конфликте id новостей, старые значения
	 * затираются.
	 * @return true - если были добавлены новые Новости (перезапись не считается добавлением).
	 */
	public boolean insertNews(Collection<NewsItem> news) {
		
		applyReadedStatusToNews(news);
		
		SQLiteDatabase db = getWritableDatabase();
		int countNewsBeforeInsert = getCountOfNews(db);
				
		boolean newItemsInserted = false;
		ContentValues values = new ContentValues();

		db.beginTransaction();
		try {
			for (NewsItem item : news) {
				values.clear();
				values.put(NewsItemColumn.ID, item.getId());
				values.put(NewsItemColumn.DATE, item.getDate().getTime());
				values.put(NewsItemColumn.IS_READ, item.isRead()?"1":"0");

				db.insert(NEWS_TABLE_NAME, null, values);
				for (String locale : item.getAllLocales()) {
					Message message = item.getMessageByLocale(locale);
					values.clear();
					values.put(NewsItemMessageColumns.LANGUAGE,
							message.getLang());
					values.put(NewsItemMessageColumns.LOCALE,
							message.getLocale());
					values.put(NewsItemMessageColumns.TITLE, message.getTitle());
					values.put(NewsItemMessageColumns.BODY, message.getBody());
					values.put(NewsItemMessageColumns.NEWS_ID, item.getId());

					db.insert(MESSAGE_TABLE_NAME, null, values);

				}

			}
			db.setTransactionSuccessful();
		} finally {
			db.endTransaction();
		}

		int countNewsAfterInsert = getCountOfNews(db);
		newItemsInserted = (countNewsAfterInsert - countNewsBeforeInsert ) > 0;
		db.close();
		deleteOutdatedNews();
		return newItemsInserted;
	}

	private void applyReadedStatusToNews(Collection<NewsItem> news) {
		if(news.isEmpty())
			return;
		
		Cursor readedCursor = getContainerNewsCursor();
		ArrayList<NewsItemDB> readedNews = cursorToNewsItemDbCollection(readedCursor);
		for (NewsItemDB readed : readedNews) {
			for (NewsItem toInsert : news) {
				if(toInsert.getId() == readed.getId())
					toInsert.setRead(readed.isRead());
			}
		}
	}

	private int getCountOfNews(SQLiteDatabase db) {
		Cursor cur = db.query(NEWS_TABLE_NAME, null, null, null, null, null,null);
		int countItemsBeforeInsert = cur.getCount(); 
		cur.close();
		return countItemsBeforeInsert;
	}

	private void deleteOutdatedNews() {
		SQLiteDatabase db = getWritableDatabase();
		db.beginTransaction();
		
		try {
			db.delete(NEWS_TABLE_NAME, NewsItemColumn.DATE + " < ?",
					new String[] { String.valueOf(System
							.currentTimeMillis()
							- 1000L
							* 60L
							* 60L
							* 24L
							* 365L) });
			db.setTransactionSuccessful();
		} finally {
			db.endTransaction();
		}
		db.close();
	}

	public Cursor getContainerNewsCursor() {
		synchronized (SYNC_OBJECT) {
			SQLiteDatabase db = getReadableDatabase();
			Cursor ret = db.rawQuery("SELECT * FROM  " + TABLE + " ORDER BY " + NewsItemColumn.DATE + " DESC, "
					+ NewsItemColumn.ID + " DESC", new String[] {});
			return ret;
		}
	}
	
	public Cursor getContainerNewsCursorById(Integer newsId) {
		synchronized (SYNC_OBJECT) {
			SQLiteDatabase db = getReadableDatabase();
			Cursor ret = db.rawQuery("SELECT * FROM  " + TABLE
					+ " WHERE " + NewsItemColumn.ID + "=?"
					+ " ORDER BY " + NewsItemColumn.DATE + " DESC, "
					+ NewsItemColumn.ID + " DESC", new String[] {newsId.toString()});
			return ret;
		}
	}
	
	/**
	 * Возвращает список новостей с данным языком новости. Язык указывается в
	 * двухбуквенном формате. При отсутствии новости на указанном языке,
	 * возвращает ее англоязычную версию.
	 * 
	 * @param locale
	 *            - язык новостей
	 * @return указатель на список. Названия полей -
	 *         {@link NewsStorage#ID_COLUMN}, {@link NewsStorage#DATE_COLUMN},
	 *         {@link NewsStorage#LOCALE_COLUMN},
	 *         {@link NewsStorage#TITLE_COLUMN}, {@link NewsStorage#BODY_COLUMN}
	 *         , {@link NewsStorage#IS_READ_COLUMN}
	 */
	public Cursor getStandaloneNewsCursorByLocale(String locale) {
		synchronized (SYNC_OBJECT) {

			SQLiteDatabase db = getReadableDatabase();
			Cursor ret = db.rawQuery("SELECT * FROM  " + TABLE + " WHERE  " +

			" ( " + NewsItemColumn._ID + " IN (" + "SELECT "
					+ NewsItemColumn._ID + " FROM  " + TABLE + " WHERE  "
					+ NewsItemMessageColumns.LOCALE + "=? )" + " AND "
					+ NewsItemColumn.ID + " NOT IN (" + " SELECT DISTINCT "
					+ NewsItemColumn.ID + " FROM " + TABLE + " WHERE "
					+ NewsItemMessageColumns.LOCALE + "=?)" + " ) " + " OR "
					+ NewsItemMessageColumns.LOCALE + "=? " + " OR " + " ( "
					+ NewsItemColumn._ID + " IN (" + "SELECT "
					+ NewsItemColumn._ID + " FROM " + TABLE + " WHERE "
					+ NewsItemColumn._ID + " IN  (" + "SELECT MIN("
					+ NewsItemColumn._ID + " )" + " FROM " + TABLE + " AS T"
					+ " GROUP BY  T." + NewsItemColumn.ID + " )) " +

					" AND " + NewsItemColumn.ID + " NOT  IN ("
					+ " SELECT DISTINCT " + NewsItemColumn.ID + " FROM  "
					+ TABLE + " WHERE  " + NewsItemMessageColumns.LOCALE
					+ " =? " + " OR " + NewsItemMessageColumns.LOCALE + "=?)"
					+ " ) " + " ORDER BY " + NewsItemColumn.DATE + " DESC, "
					+ NewsItemColumn.ID + " DESC"

			, new String[] { "en", locale, locale, "en", locale });

			return ret;
		}

	}

	public Cursor query(String[] columns, String selection,
			String[] selectionArgs, String groupBy, String having,
			String orderBy, String limit) {
		SQLiteDatabase mDb = getReadableDatabase();
		return mDb.query(NewsStorage.TABLE, columns, selection, selectionArgs,
				groupBy, having, orderBy);

	}

	public int updateNews(NewsItemDB item) {
		SQLiteDatabase db = getWritableDatabase();
		ContentValues values = new ContentValues();
		values.put(NewsItemColumn.IS_READ, item.isRead() ? 1 : 0);
		int affectedRows = db.update(NEWS_TABLE_NAME, values, NewsItemColumn.ID + "=?",
				new String[] { String.valueOf(item.getId()) });
		db.close();
		return affectedRows;
	}
	
	public int markAllAsRead() {
		SQLiteDatabase db = getWritableDatabase();
		ContentValues values = new ContentValues();
		values.put(NewsItemColumn.IS_READ, 1);
		int affectedRows = db.update(NEWS_TABLE_NAME, values, null,null);
		db.close();
		return affectedRows;
	}

	public int count() {
		SQLiteDatabase db = getReadableDatabase();
		int res = getCountOfNews(db);
		db.close();
		return res;

	}

	public long getLastSyncMillis() {
		return mNewsPrefs.getLong(LAST_UPDATE, 0);
	}
	
	public int getUnreadCount() {
		SQLiteDatabase db = this.getReadableDatabase();
		Cursor unreadCursor = getCursorWithUnreadStatus(db, true);
		int unreadCount = unreadCursor.getCount();
		db.close();
		return unreadCount;
	}

	protected Cursor getCursorWithUnreadStatus(SQLiteDatabase db, boolean isUnread) {
		Cursor cur = db.rawQuery("SELECT DISTINCT " + NewsItemColumn.ID + " FROM  " + TABLE 
				+ " WHERE  " + NewsStorage.IS_READ_COLUMN + "=?"
		, new String[] {isUnread?"0":"1"});
		return cur;
	}
	
	protected static ArrayList<NewsItemDB> cursorToNewsItemDbCollection(Cursor c) {
		ArrayList<NewsItemDB> items = new ArrayList<NewsItemDB>(c.getCount());
		for (int i = 0; i < c.getCount(); i++) {
			if(c.moveToPosition(i)) 
				items.add(NewsItemDB.createFrom(c));
		}
		c.close();
		return items;
	}
}
