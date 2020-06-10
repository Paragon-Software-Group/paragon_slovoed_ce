package com.paragon_software.dictionary_manager.components;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;

// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public class MorphoComponent extends DictionaryComponent implements Parcelable
{
  private final String mId;

  public MorphoComponent( String id, String url, long size, String langFrom,
                          @NonNull String sdcId )
  {
    super(Type.MORPHO, url, size, false, "0.0", langFrom, sdcId);
    this.mId = id;
  }

  protected MorphoComponent(Parcel in) {
    super(in);
    this.mId = in.readString();
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    super.writeToParcel(dest, flags);
    dest.writeString(mId);
  }

  @Override
  public int describeContents() {
    return 0;
  }

  public String getId() {
    return mId;
  }

  public static final Creator<MorphoComponent> CREATOR = new Creator<MorphoComponent>() {
    @Override
    public MorphoComponent createFromParcel(Parcel in) {
      return new MorphoComponent(in);
    }

    @Override
    public MorphoComponent[] newArray(int size) {
      return new MorphoComponent[size];
    }
  };
}
