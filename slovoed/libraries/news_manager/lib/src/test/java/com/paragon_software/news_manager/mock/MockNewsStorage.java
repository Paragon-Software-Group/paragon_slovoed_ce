package com.paragon_software.news_manager.mock;

import android.content.Context;
import android.database.Cursor;
import android.database.MatrixCursor;
import androidx.annotation.NonNull;

import com.paragon_software.news_manager.NewsItem;

import java.util.List;

import shdd.android.components.news.data.NewsItemDB;
import shdd.android.components.news.storage.NewsStorage;

/**
 * Created by Ivan Kuzmin on 21.02.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public class MockNewsStorage extends NewsStorage
{
    private Cursor mNewsCursor;

    public MockNewsStorage( @NonNull Context context, @NonNull List<NewsItem> newsList )
    {
        super(context);
        mNewsCursor = getMockNewsCursor(newsList);
    }

    @Override
    public Cursor getContainerNewsCursor()
    {
        return mNewsCursor;
    }

    @Override
    public Cursor getContainerNewsCursorById(Integer newsId)
    {
        String[] columnNames = mNewsCursor.getColumnNames();
        MatrixCursor resultCursor = new MatrixCursor(columnNames);
        for (mNewsCursor.moveToFirst(); !mNewsCursor.isAfterLast(); mNewsCursor.moveToNext())
        {
            int id = mNewsCursor.getInt(mNewsCursor.getColumnIndex("id"));
            if (id == newsId) {
                MatrixCursor.RowBuilder rowBuilder = resultCursor.newRow();
                for(String col: columnNames)
                {
                    rowBuilder.add(mNewsCursor.getString(mNewsCursor.getColumnIndex(col)));
                }
                break;
            }
        }
        return resultCursor;
    }

    @Override
    public int markAllAsRead()
    {
        String[] columnNames = mNewsCursor.getColumnNames();
        MatrixCursor resultCursor = new MatrixCursor(columnNames);

        for (mNewsCursor.moveToFirst(); !mNewsCursor.isAfterLast(); mNewsCursor.moveToNext())
        {
            MatrixCursor.RowBuilder rowBuilder = resultCursor.newRow();
            for(String col: columnNames)
            {
                if (!col.equals("is_read")) {
                    rowBuilder.add(mNewsCursor.getString(mNewsCursor.getColumnIndex(col)));
                } else {
                    rowBuilder.add(1);
                }
            }
        }
        mNewsCursor = resultCursor;

        return 1;
    }

    @Override
    public int updateNews(NewsItemDB item)
    {
        int isUpdateSuccess = 0;

        String[] columnNames = mNewsCursor.getColumnNames();
        MatrixCursor resultCursor = new MatrixCursor(columnNames);

        for (mNewsCursor.moveToFirst(); !mNewsCursor.isAfterLast(); mNewsCursor.moveToNext())
        {
            int id = mNewsCursor.getInt(mNewsCursor.getColumnIndex("id"));
            if (item.getId() == id) {
                resultCursor.addRow(new Object[] {item.getId(), item.getDate(), item.getBody()
                        , item.getTitle(), item.getLocale(), item.isRead() ? 1 : 0});
                isUpdateSuccess = 1;
            } else {
                MatrixCursor.RowBuilder rowBuilder = resultCursor.newRow();
                for(String col: columnNames)
                {
                    rowBuilder.add(mNewsCursor.getString(mNewsCursor.getColumnIndex(col)));
                }
            }
        }
        mNewsCursor = resultCursor;

        return isUpdateSuccess;
    }

    @Override
    public int getUnreadCount()
    {
        int unreadCount = 0;
        for (mNewsCursor.moveToFirst(); !mNewsCursor.isAfterLast(); mNewsCursor.moveToNext())
        {
            boolean isRead = mNewsCursor.getInt(mNewsCursor.getColumnIndex("is_read")) == 1;
            if (!isRead)
                unreadCount++;
        }
        return unreadCount;
    }

    private Cursor getMockNewsCursor(List<NewsItem> mockNewsList)
    {
        String[] columns = new String[] { "id", "date", "body", "title", "locale", "is_read" };
        MatrixCursor cursor = new MatrixCursor(columns);
        for (NewsItem newsItem: mockNewsList)
        {
            cursor.addRow(new Object[] {newsItem.getId(), newsItem.getDate(), newsItem.getBody()
                    , newsItem.getTitle(), newsItem.getLocale(), newsItem.isRead() ? 1 : 0});
        }
        return cursor;
    }
}
