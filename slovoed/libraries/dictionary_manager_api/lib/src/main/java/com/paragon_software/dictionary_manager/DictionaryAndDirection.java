package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;
import com.google.gson.annotations.SerializedName;

import java.io.Serializable;

public class DictionaryAndDirection implements Serializable {
    // WARNING!!! Don't change @SerializedName values. They already
    // was used in persistent storage of real users devices.
    // (it has so strange names because of ProGuard)
    @SerializedName("a")
    @NonNull
    private final Dictionary.DictionaryId mDictionaryId;

    @SerializedName("b")
    @NonNull
    private final Dictionary.Direction mDirection;

    public DictionaryAndDirection(@NonNull Dictionary.DictionaryId dictionaryId, @NonNull Dictionary.Direction direction) {
        mDictionaryId = dictionaryId;
        mDirection = direction;
    }

    @NonNull
    public Dictionary.DictionaryId getDictionaryId() {
        return mDictionaryId;
    }

    @NonNull
    public Dictionary.Direction getDirection() {
        return mDirection;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        DictionaryAndDirection that = (DictionaryAndDirection) o;

        if (!mDictionaryId.equals(that.mDictionaryId)) return false;
        return mDirection.equals(that.mDirection);
    }

    @Override
    public int hashCode() {
        int result = mDictionaryId.hashCode();
        result = 31 * result + mDirection.hashCode();
        return result;
    }

    @Override
    public String toString() {
        return "DictionaryAndDirection{" +
                "mDictionaryId=" + mDictionaryId +
                ", mDirection=" + mDirection +
                '}';
    }
}
