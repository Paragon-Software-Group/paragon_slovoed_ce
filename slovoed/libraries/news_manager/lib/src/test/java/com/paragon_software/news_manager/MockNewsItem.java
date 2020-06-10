package com.paragon_software.news_manager;

import java.util.Date;

/**
 * Created by Ivan Kuzmin on 20.02.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public class MockNewsItem extends NewsItem
{
    MockNewsItem(int id)
    {
        this(id, false);
    }

    MockNewsItem(int id, boolean isRead)
    {
        super(id, new Date().getTime(), "Body_" + id
                , "Title_" + id, "Locale_" + id, isRead);
    }
}
