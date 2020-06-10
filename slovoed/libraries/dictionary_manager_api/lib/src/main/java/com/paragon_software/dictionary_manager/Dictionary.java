/*
 * dictionary manager api
 *
 *  Created on: 19.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.gson.annotations.SerializedName;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.extbase.MorphoInfo;
import com.paragon_software.dictionary_manager.extbase.PictureInfo;
import com.paragon_software.dictionary_manager.extbase.SoundInfo;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Represents dictionary.
 */
@SuppressWarnings( "unused" )
public final class Dictionary extends CatalogItem
{
  @NonNull
  private final DICTIONARY_TYPE             mType;
  @NonNull
  private final List< DictionaryComponent > mDictionaryComponents;
  @Nullable
  private       DictionaryLocation          mDictionaryLocation;
  @NonNull
  private       List<MorphoInfo>            mMorphoInfoList;
  @NonNull
  private       List<SoundInfo>             mSoundInfoList;
  @NonNull
  private       List<PictureInfo>           mPictureInfoList;
  @NonNull
  private final Collection<Direction>       mDirections;

  private final boolean mIsDemoFts;
  private final boolean mIsPromiseFtsInDemo;


  /**
   * Constructor to clone Dictionary object.
   * @param other Dictionary object to clone
   */
  private Dictionary( Dictionary other )
  {
    super(other);
    mType = other.mType;
    mDictionaryLocation = other.mDictionaryLocation;
    mMorphoInfoList = new ArrayList<>(other.mMorphoInfoList);
    mSoundInfoList = new ArrayList<>(other.mSoundInfoList);
    mPictureInfoList = new ArrayList<>(other.mPictureInfoList);
    mDirections = new ArrayList<>(other.mDirections);
    // do not clone DictionaryComponent items themselves
    // (different Dictionaries may have common components for example sound base components)
    mDictionaryComponents = new ArrayList<>(other.mDictionaryComponents);
    mIsDemoFts = other.mIsDemoFts;
    mIsPromiseFtsInDemo = other.mIsPromiseFtsInDemo;
  }

  private Dictionary( @NonNull final Builder builder )
  {
    super(builder.mId, builder.mTitle, builder.mDescription, builder.mIcon, builder.mStatus, builder.mMarketingData, builder.mRemovedFromSale);
    this.mType = builder.mType;
    this.mDirections = builder.mDirections;
    this.mDictionaryLocation = builder.mDictionaryLocation;
    this.mDictionaryComponents = builder.mDictionaryComponents;
    this.mIsDemoFts = builder.mIsDemoFts;
    this.mIsPromiseFtsInDemo = builder.mIsPromiseFtsInDemo;

    mSoundInfoList = new ArrayList<>();
    mMorphoInfoList = new ArrayList<>();
    mPictureInfoList = new ArrayList<>();
  }

  /**
   * @return dictionary type
   */
  @NonNull
  public DICTIONARY_TYPE getType()
  {
    return mType;
  }

  public @NonNull Collection<Direction> getDirections()
  {
    return mDirections;
  }

  @Nullable
  public DictionaryLocation getDictionaryLocation()
  {
    return mDictionaryLocation;
  }

  void setDictionaryLocation( @Nullable DictionaryLocation dictionaryLocation )
  {
    mDictionaryLocation = dictionaryLocation;
  }

  @NonNull
  public List<MorphoInfo> getMorphoInfoList() {
    return mMorphoInfoList;
  }

  void setMorphoInfoList(@NonNull List<MorphoInfo> morphoInfoList) {
    this.mMorphoInfoList = morphoInfoList;
  }

  @NonNull
  public List<SoundInfo> getSoundInfoList() {
    return mSoundInfoList;
  }

  void setSoundInfoList(@NonNull List<SoundInfo> SoundInfoList) {
    this.mSoundInfoList = SoundInfoList;
  }

  @NonNull
  public List<PictureInfo> getPictureInfoList() {
    return mPictureInfoList;
  }

  void setPictureInfoList(@NonNull List<PictureInfo> pictureInfoList) {
    this.mPictureInfoList = pictureInfoList;
  }

  @NonNull
  public List< DictionaryComponent > getDictionaryComponents()
  {
    return mDictionaryComponents;
  }

  @NonNull
  @Override
  public FeatureName getPurchaseFeatureName() {
    return DictionaryComponent.featureNameOf(getDictionaryWordBaseComponent(false));
  }

  @NonNull
  @Override
  Dictionary cloneItem() {
    return new Dictionary(this);
  }

  @Nullable
  DictionaryComponent getDictionaryWordBaseComponent(boolean demo) {
    for ( DictionaryComponent component : getDictionaryComponents() )
      if ( DictionaryComponent.Type.WORD_BASE.equals(component.getType()) && demo == component.isDemo() )
        return component;
    return null;
  }

  public boolean isIsDemoFts() {
    return mIsDemoFts;
  }

  public boolean isIsPromiseFtsInDemo() {
    return mIsPromiseFtsInDemo;
  }

  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "Dictionary{" + "mId=" + getId()
        + ", mType=" + mType + ", mDictionaryComponents=" + mDictionaryComponents + ", mStatus=" + getStatus()
        + ", mDictionaryLocation=" + mDictionaryLocation + ", mMorphoInfoList=" + mMorphoInfoList + ", mSoundInfoList="
        + mSoundInfoList + ", mPictureInfoList=" + mPictureInfoList + ", mDirections=" + mDirections + ", mMarketingData=" + getMarketingData()
        + ", mRemovedFromSale=" + isRemovedFromSale() + '}';
  }

  @SuppressWarnings( "unused" )
  public enum DICTIONARY_TYPE
  {
    BILINGUAL("bilingual"),
    EXPLANATORY("explanatory"),
    THESAURUS("thesaurus"),
    NONE("");

    private String mName;

    DICTIONARY_TYPE( String name )
    {
      mName = name;
    }

    public String getName()
    {
      return mName;
    }

    public static DICTIONARY_TYPE fromName( String name )
    {
      for ( DICTIONARY_TYPE type : DICTIONARY_TYPE.values() )
      {
        if ( type.mName.equalsIgnoreCase(name) )
        {
          return type;
        }
      }
      return DICTIONARY_TYPE.NONE;
    }
  }

  @Override
  public boolean equals( Object o )
  {
    if ( this == o )
    {
      return true;
    }
    if ( o == null || getClass() != o.getClass() )
    {
      return false;
    }
    Dictionary that = (Dictionary) o;
    return that.toString().equals(this.toString());
  }

  @Override
  public int hashCode()
  {
    return toString().hashCode();
  }

  @SuppressWarnings( "unused" )
  public enum DICTIONARY_STATUS
  {
    DEMO;

    public boolean canBePurchased()
    {
      return true;
    }
  }

  /**
   * standard builder pattern
   */
  public static class Builder
  {
    @NonNull
    private final DictionaryId    mId;
    @NonNull
    private final LocalizedString mTitle;
    @NonNull
    private final LocalizedString mDescription;
    @NonNull
    private final IDictionaryIcon mIcon;
    @NonNull
    private DICTIONARY_TYPE   mType          = DICTIONARY_TYPE.NONE;
    @NonNull
    private DICTIONARY_STATUS mStatus        = DICTIONARY_STATUS.DEMO;
    @Nullable
    private DictionaryLocation          mDictionaryLocation;
    @NonNull
    private List< DictionaryComponent > mDictionaryComponents;
    @NonNull
    private Collection< Direction > mDirections;

    private MarketingData mMarketingData = new MarketingData();

    private boolean mRemovedFromSale = false;

    private boolean mIsDemoFts = false;

    private boolean mIsPromiseFtsInDemo = false;


    public Builder( @NonNull final DictionaryId id,
                    @NonNull final LocalizedString title, @NonNull final LocalizedString description,
                    @NonNull final IDictionaryIcon icon )
    {
      this.mId = id;
      this.mTitle = title;
      this.mDescription = description;
      this.mIcon = icon;
      this.mDirections = new ArrayList<>();
      this.mDictionaryComponents = new ArrayList<>();
    }

    @NonNull
    public Builder setType( @NonNull final DICTIONARY_TYPE type )
    {
      this.mType = type;
      return this;
    }

    @NonNull
    public Builder setStatus( @NonNull final DICTIONARY_STATUS status )
    {
      this.mStatus = status;
      return this;
    }

    public Builder setDirections( final @NonNull Collection<Direction> directions )
    {
      mDirections = directions;
      return this;
    }

    public Builder setMarketingData( MarketingData marketingData )
    {
      this.mMarketingData = marketingData;
      return this;
    }

    public Builder setDictionaryLocation( @Nullable DictionaryLocation dictionaryLocation )
    {
      this.mDictionaryLocation = dictionaryLocation;
      return this;
    }

    public Builder setIsRemovedFromSale( final @Nullable Boolean removedFromSale )
    {
      if ( removedFromSale != null)
        mRemovedFromSale = removedFromSale;
      return this;
    }

    @NonNull
    public Dictionary build()
    {
      return new Dictionary(this);
    }

    public Builder setDictionaryComponents( List<DictionaryComponent > dictionaryComponents )
    {
      this.mDictionaryComponents = dictionaryComponents;
      return this;
    }

    public Builder setIsDemoFts(boolean isDemoFts)
    {
      this.mIsDemoFts = isDemoFts;
      return this;
    }

    public Builder setIsPromiseFtsInDemo(boolean isPromiseFtsInDemo)
    {
      this.mIsPromiseFtsInDemo = isPromiseFtsInDemo;
      return this;
    }
  }

  /**
   * dictionary id. Parcelable to send/receive ability
   */
  public static final class DictionaryId implements Parcelable, Serializable, Comparable<DictionaryId>
  {
    /**
     * Implementation of interface for generating instances of your Parcelable class from a Parcel.
     *
     * @see Creator
     */
    public static final Creator< DictionaryId > CREATOR = new Creator< DictionaryId >()
    {
      /**
       * Create a new instance of the Parcelable class, instantiating it
       * from the given Parcel whose data had previously been written by
       * {@link Parcelable#writeToParcel Parcelable.writeToParcel()}.
       *
       * @param source The Parcel to read the object's data from.
       * @return Returns a new instance of the Parcelable class.
       * @see     Creator#createFromParcel(Parcel)
       */
      @SerializedName("a")
      @Override
      public DictionaryId createFromParcel( Parcel source )
      {
        return new DictionaryId(source);
      }

      /**
       * Create a new array of the Parcelable class.
       *
       * @param size Size of the array.
       * @return Returns an array of the Parcelable class, with every entry
       * initialized to null.
       * @see     Creator#newArray(int)
       */
      @Override
      public DictionaryId[] newArray( int size )
      {
        return new DictionaryId[size];
      }
    };

    // WARNING!!! Don't change @SerializedName values. They already
    // was used in persistent storage of real users devices.
    // (it has so strange names because of ProGuard)
    @SerializedName("a")
    @NonNull
    private final String mId;

    public DictionaryId( @NonNull final String id )
    {
      mId = id;
    }

    private DictionaryId( Parcel in )
    {
      mId = in.readString();
    }

    @Override
    public int hashCode()
    {
      return mId.hashCode();
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
      DictionaryId other = (DictionaryId) obj;
      return other.mId.equals(this.mId);
    }

    @Override
    public String toString()
    {
      return mId;
    }

    /**
     * Describe the kinds of special objects contained in this Parcelable
     * instance's marshaled representation
     *
     * @return a bitmask indicating the set of special object types marshaled
     * by this Parcelable object instance.
     *
     * @see Parcelable#describeContents()
     */
    @Override
    public int describeContents()
    {
      return 0;
    }

    /**
     * Flatten this object in to a Parcel.
     *
     * @param dest  The Parcel in which the object should be written.
     * @param flags Additional flags about how the object should be written.
     *              May be 0 or {@link #PARCELABLE_WRITE_RETURN_VALUE}.
     *
     * @see Parcelable#writeToParcel(Parcel, int)
     */
    @Override
    public void writeToParcel( Parcel dest, int flags )
    {
      dest.writeString(mId);
    }

    @Override
    public int compareTo(DictionaryId o) {
      return mId.compareTo(o.mId);
    }
  }

  // Need to add annotation constraints and autotests for all methods
  public static final class Direction implements Serializable
  {
    @SerializedName("a")
    private final int mLanguageFrom;
    @SerializedName("b")
    private final int mLanguageTo;

    @Nullable
    private final transient IDictionaryIcon mIcon;

    public Direction(int languageFrom, int languageTo, @Nullable IDictionaryIcon icon)
    {
      mLanguageFrom = languageFrom;
      mLanguageTo = languageTo;
      mIcon = icon;
    }

    public int getLanguageFrom()
    {
      return mLanguageFrom;
    }

    public int getLanguageTo()
    {
      return mLanguageTo;
    }

    @Nullable
    public IDictionaryIcon getIcon()
    {
      return mIcon;
    }

    @Override
    public int hashCode() {
      int result = mLanguageFrom;
      result = 31 * result + mLanguageTo;
      return result;
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
      Direction other = (Direction) obj;
      return other.mLanguageFrom == this.mLanguageFrom && other.mLanguageTo == this.mLanguageTo;
    }

    @Override
    public String toString()
    {
      // NOTE When modify this method consider that it is used to create ParagonDictionaryManager object state trace
      return "Direction{" + "mLanguageFrom=" + mLanguageFrom + ", mLanguageTo=" + mLanguageTo + '}';
    }
  }
}
