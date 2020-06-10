package com.paragon_software.dictionary_manager.components;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.paragon_software.dictionary_manager.FeatureName;

// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public class WordBaseComponent extends DictionaryComponent implements Parcelable
{
  private final String langTo;
  private final String wordsCount;

  @VisibleForTesting
  public WordBaseComponent( String url, long size, boolean demo, String version, String langFrom, String langTo,
                            @NonNull String sdcId, @NonNull FeatureName featureName, String wordsCount)
  {
    super(Type.WORD_BASE, url, size, demo, version, langFrom, sdcId, featureName);
    this.langTo = langTo;
    this.wordsCount = wordsCount;
  }

  public WordBaseComponent( String url, long size, boolean demo, String version, String langFrom, String langTo,
                            @NonNull String sdcId , String wordsCount)
  {
    super(Type.WORD_BASE, url, size, demo, version, langFrom, sdcId);
    this.langTo = langTo;
    this.wordsCount = wordsCount;
  }

  protected WordBaseComponent(Parcel in) {
    super(in);
    langTo = in.readString();
    wordsCount = in.readString();
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    super.writeToParcel(dest, flags);
    dest.writeString(langTo);
    dest.writeString(wordsCount);
  }

  @Override
  public int describeContents() {
    return 0;
  }

  public static final Creator<WordBaseComponent> CREATOR = new Creator<WordBaseComponent>() {
    @Override
    public WordBaseComponent createFromParcel(Parcel in) {
      return new WordBaseComponent(in);
    }

    @Override
    public WordBaseComponent[] newArray(int size) {
      return new WordBaseComponent[size];
    }
  };

  public String getLangTo()
  {
    return langTo;
  }

  public String getWordsCount() {
    return wordsCount;
  }
}
