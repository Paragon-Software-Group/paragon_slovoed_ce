package com.paragon_software.favorites_manager.mock;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;

/**
 * Created by Ivan Kuzmin on 21.01.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public class StaticConsts
{
    public static final float DELTA = 0.01f;

    public static final Dictionary.DictionaryId DICT1_ID = new Dictionary.DictionaryId("111");
    public static final Dictionary.DictionaryId DICT2_ID = new Dictionary.DictionaryId("222");
    public static final Dictionary.DictionaryId DICT3_ID = new Dictionary.DictionaryId("333");

    public static final ArticleItem ARTICLE1 = new ArticleItem.Builder(DICT1_ID, 1, 1).build();
    public static final ArticleItem ARTICLE2 = new ArticleItem.Builder(DICT2_ID, 1, 1).build();
    public static final ArticleItem ARTICLE3 = new ArticleItem.Builder(DICT2_ID, 1, 2).build();

    public static final Dictionary.Direction DIRECT1 = new Dictionary.Direction(1,2, null);
    public static final Dictionary.Direction DIRECT2 = new Dictionary.Direction(2,3, null);
    public static final Dictionary.Direction DIRECT3 = new Dictionary.Direction(3,4, null);
    public static final Dictionary.Direction DIRECT4 = new Dictionary.Direction(4,1, null);
}
