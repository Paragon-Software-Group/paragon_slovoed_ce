package com.paragon_software.utils_slovoed.static_consts;

/**
 * Created by Ivan Kuzmin on 13.04.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public class BundleKeys
{
    private static String mClassName = BundleKeys.class.getName();

    public static final String BUNDLE_KEY_FOR_INTENT_EXTRA = "FOR_INTENT_EXTRA" + mClassName;

//    NEW DICTIONARY ID FOR DESCRIPTION SCREEN
    public static final String BUNDLE_KEY_NEW_DESCRIPTION_DICTIONARY_ID = "NEW_DESCRIPTION_DICTIONARY_ID" + mClassName;

//    IS FIRST RUN DICTIONARY (FOR TABLET)
    public static final String BUNDLE_KEY_IS_TABLET_DICTIONARY_FIRST_RUN = "IS_TABLET_DICTIONARY_FIRST_RUN" + mClassName;

//    IS NEED OPEN FREE PREVIEW WHEN OPEN DICTIONARY
    public static final String BUNDLE_KEY_IS_OPEN_FREE_PREVIEW = "IS_OPEN_FREE_PREVIEW" + mClassName;

//    IS NEED RESTORE PURCHASES
    public static final String BUNDLE_KEY_IS_RESTORE_PURCHASE = "IS_RESTORE_PURCHASE" + mClassName;
}
