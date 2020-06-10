package com.paragon_software.dictionary_manager.components;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;

// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public class SoundComponent extends DictionaryComponent implements Parcelable
{
  private final String name;

  public SoundComponent(String url, long size, boolean demo, String version, String langFrom,
                        String name, @NonNull String sdcId )
  {
    super(Type.SOUND, url, size, demo, version, langFrom, sdcId);
    this.name = name;
  }

  protected SoundComponent(Parcel in) {
    super(in);
    name = in.readString();
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    super.writeToParcel(dest, flags);
    dest.writeString(name);
  }

  @Override
  public int describeContents() {
    return 0;
  }

  public static final Creator<SoundComponent> CREATOR = new Creator<SoundComponent>() {
    @Override
    public SoundComponent createFromParcel(Parcel in) {
      return new SoundComponent(in);
    }

    @Override
    public SoundComponent[] newArray(int size) {
      return new SoundComponent[size];
    }
  };

  public String getName()
  {
    return name;
  }
}
