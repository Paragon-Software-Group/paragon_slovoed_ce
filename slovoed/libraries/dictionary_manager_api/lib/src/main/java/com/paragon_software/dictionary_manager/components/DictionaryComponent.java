package com.paragon_software.dictionary_manager.components;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import com.paragon_software.dictionary_manager.FeatureName;

// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public abstract class DictionaryComponent implements Parcelable
{
  private final Type        type;
  private final String      url;
  private final long        size;
  private final boolean     demo;
  private final String      version;
  private final String      langFrom;
  @NonNull
  private final String      sdcId;
  private       FeatureName featureName;

  @NonNull
  public static FeatureName featureNameOf(@Nullable DictionaryComponent component) {
    FeatureName res = FeatureName.getEmpty();
    if (component != null)
      if (!FeatureName.isEmpty(component.getFeatureName()))
        res = component.getFeatureName();
    return res;
  }

  public DictionaryComponent( Type type, String url, long size, boolean demo, String version
                                                        , String langFrom, @NonNull String sdcId )
  {
    this(type, url, size, demo, version, langFrom, sdcId, new FeatureName(type + "_" + sdcId + ( demo ? "_demo" : "" )));
  }

  @VisibleForTesting
  public DictionaryComponent( Type type, String url, long size, boolean demo, String version
                      , String langFrom, @NonNull String sdcId, @NonNull FeatureName featureName )
  {
    this.type = type;
    this.url = url;
    this.size = size;
    this.demo = demo;
    this.sdcId = sdcId;
    this.version = version;
    this.langFrom = langFrom;
    this.featureName = featureName;
  }

  protected DictionaryComponent(Parcel in) {
    type = Type.values()[in.readInt()];
    url = in.readString();
    size = in.readLong();
    demo = in.readByte() != 0;
    version = in.readString();
    langFrom = in.readString();
    sdcId = in.readString();
    featureName = in.readParcelable(FeatureName.class.getClassLoader());
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    dest.writeInt(type.ordinal());
    dest.writeString(url);
    dest.writeLong(size);
    dest.writeByte((byte) (demo ? 1 : 0));
    dest.writeString(version);
    dest.writeString(langFrom);
    dest.writeString(sdcId);
    dest.writeParcelable(featureName, flags);
  }

  @Override
  public int describeContents() {
    return 0;
  }

  public Type getType() {
    return type;
  }

  public String getUrl()
  {
    return url;
  }

  public long getSize()
  {
    return size;
  }

  public boolean isDemo()
  {
    return demo;
  }

  public String getVersion()
  {
    return version;
  }

  public String getLangFrom()
  {
    return langFrom;
  }

  public String getSdcId()
  {
    return sdcId;
  }

  public FeatureName getFeatureName()
  {
    return featureName;
  }

  @Override
  public boolean equals( Object obj )
  {
    if ( this == obj )
    {
      return true;
    }
    if ( null == obj )
    {
      return false;
    }
    if ( getClass() != obj.getClass() )
    {
      return false;
    }
    DictionaryComponent other = (DictionaryComponent) obj;
    return other.toString().equals(this.toString());
  }

  @NonNull
  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "DictionaryComponent{" + "type=" + type + ", demo=" + demo + ", version='" + version + '\'' + ", langFrom='"
        + langFrom + '\'' + ", sdcId='" + sdcId + '\'' + '}';
  }

  public enum Type
  {
    WORD_BASE,
    PICT,
    SOUND,
    WORD_FORM,
    GAME,
    MORPHO,
    PENREADER;

    @Override
    public String toString()
    {
      switch ( this )
      {
        case WORD_BASE:
          return "wordbase";
        case PICT:
          return "pict";
        case SOUND:
          return "sound";
        case WORD_FORM:
          return "wordform";
        case GAME:
          return "game";
        case MORPHO:
          return "morpho";
        case PENREADER:
          return "penreader";
      }
      return super.toString();
    }
  }
}
