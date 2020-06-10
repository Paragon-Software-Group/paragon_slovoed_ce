package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.text.LocalizedString;

abstract class CatalogItem {
    /**
     * Unique id for identify this dictionary
     */
    @NonNull
    private final Dictionary.DictionaryId mId;
    @NonNull
    private final LocalizedString mTitle;
    @NonNull
    private final LocalizedString mDescription;
    @NonNull
    private final IDictionaryIcon mIcon;
    @NonNull
    private Dictionary.DICTIONARY_STATUS mStatus;
    private final MarketingData mMarketingData;
    private final boolean mRemovedFromSale;

    CatalogItem(@NonNull Dictionary.DictionaryId id,
                @NonNull LocalizedString title,
                @NonNull LocalizedString description,
                @NonNull IDictionaryIcon icon,
                @NonNull Dictionary.DICTIONARY_STATUS status,
                MarketingData marketingData,
                boolean removedFromSale) {
        mId = id;
        mTitle = title;
        mDescription = description;
        mIcon = icon;
        mStatus = status;
        mMarketingData = marketingData;
        mRemovedFromSale = removedFromSale;
    }

    CatalogItem(@Nullable CatalogItem other) {
        if(other != null)
        {
            mId = other.getId();
            mTitle = other.getTitle();
            mDescription = other.getDescription();
            mIcon = other.getIcon();
            mStatus = other.getStatus();
            mMarketingData = other.getMarketingData();
            mRemovedFromSale = other.isRemovedFromSale();
        }
        else
            throw new IllegalStateException("Dictionary to clone cannot be null in constructor");
    }

    /**
     * @return dictionary id
     */
    @NonNull
    public Dictionary.DictionaryId getId()
    {
        return mId;
    }

    /**
     * @return dictionary title
     */
    @NonNull
    public LocalizedString getTitle()
    {
        return mTitle;
    }

    /**
     * @return dictionary description
     */
    @NonNull
    public LocalizedString getDescription()
    {
        return mDescription;
    }

    /**
     * @return dictionary icon
     */
    @NonNull
    public IDictionaryIcon getIcon()
    {
        return mIcon;
    }

    /**
     * @return dictionary status
     */
    @NonNull
    public Dictionary.DICTIONARY_STATUS getStatus()
    {
        return mStatus;
    }

    void setStatus( @NonNull Dictionary.DICTIONARY_STATUS status )
    {
        this.mStatus = status;
    }

    public MarketingData getMarketingData()
    {
        return mMarketingData;
    }

    public boolean isRemovedFromSale()
    {
        return mRemovedFromSale;
    }

    @NonNull
    public abstract FeatureName getPurchaseFeatureName();

    @NonNull
    abstract CatalogItem cloneItem();
}
