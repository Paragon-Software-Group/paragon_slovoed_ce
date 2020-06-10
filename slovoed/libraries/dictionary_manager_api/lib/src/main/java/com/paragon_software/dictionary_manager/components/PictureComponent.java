package com.paragon_software.dictionary_manager.components;

import android.os.Parcel;
import androidx.annotation.NonNull;

public class PictureComponent extends DictionaryComponent
{
  public PictureComponent( String url, long size, boolean demo, String version,
                           String langFrom, @NonNull String sdcId )
  {
    super(Type.PICT, url, size, demo, version, langFrom, sdcId);
  }

  protected PictureComponent( Parcel in ) {
    super(in);
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    super.writeToParcel(dest, flags);
  }

  @Override
  public int describeContents() {
    return 0;
  }

  public static final Creator<PictureComponent> CREATOR = new Creator<PictureComponent>() {
    @Override
    public PictureComponent createFromParcel(Parcel in) {
      return new PictureComponent(in);
    }

    @Override
    public PictureComponent[] newArray(int size) {
      return new PictureComponent[size];
    }
  };
}
