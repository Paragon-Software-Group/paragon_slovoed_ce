package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.Collection;
import java.util.Collections;
import java.util.Set;
import java.util.TreeSet;

//
//      fullpack wiki:
//      https://prm-wiki.paragon-software.com/pages/viewpage.action?pageId=100207279 
//

public final class DictionaryPack extends CatalogItem {
    private int mBoughtMin, mBoughtMax;

    @NonNull
    private final Set<Dictionary.DictionaryId> mDictionaryIds;

    @NonNull
    private final FeatureName mFeatureName;

    public DictionaryPack(@NonNull Dictionary.DictionaryId id, @NonNull LocalizedString title, @NonNull LocalizedString description,
                          @NonNull IDictionaryIcon icon, @NonNull Dictionary.DICTIONARY_STATUS status, MarketingData marketingData, boolean isRemovedFromSale,
                          int boughtMin, int boughtMax, @NonNull Collection<Dictionary.DictionaryId> dictionaryIds) {
        super(id, title, description, icon, status, marketingData, isRemovedFromSale);
        mBoughtMin = boughtMin;
        mBoughtMax = boughtMax;
        mDictionaryIds = Collections.unmodifiableSet(new TreeSet<>(dictionaryIds));
        mFeatureName = new FeatureName("pack_" + id.toString());
    }

    private DictionaryPack(@Nullable DictionaryPack other) {
        super(other);
        mBoughtMin = other.mBoughtMin;
        mBoughtMax = other.mBoughtMax;
        mDictionaryIds = other.mDictionaryIds;
        mFeatureName = other.mFeatureName;
    }

    @NonNull
    public Set<Dictionary.DictionaryId> getDictionaryIds() {
        return mDictionaryIds;
    }

    @NonNull
    @Override
    public FeatureName getPurchaseFeatureName() {
        return mFeatureName;
    }

    @NonNull
    @Override
    DictionaryPack cloneItem() {
        return new DictionaryPack(this);
    }

    @Override
    public String toString()
    {
        // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
        return "DictionaryPack{" + "mId=" + getId()
                + ", mStatus=" + getStatus()
                + ", mBoughtMin=" + mBoughtMin
                + ", mBoughtMax=" + mBoughtMax
                + ", mDictionaryIds" + mDictionaryIds
                + ", mMarketingData=" + getMarketingData()
                + ", mRemovedFromSale=" + isRemovedFromSale() + '}';
    }

    boolean checkAvailabilityCondition(@Nullable CatalogItem[] catalogItems) {
        boolean res = false;
        if(catalogItems != null) {
            int n = 0;
            for(CatalogItem catalogItem : catalogItems)
                if((catalogItem != null) && mDictionaryIds.contains(catalogItem.getId())) {
                    n++;
                    if(n > mBoughtMax)
                        break;
                }
            res = (mBoughtMin <= n) && (n <= mBoughtMax);
        }
        return res;
    }
}
