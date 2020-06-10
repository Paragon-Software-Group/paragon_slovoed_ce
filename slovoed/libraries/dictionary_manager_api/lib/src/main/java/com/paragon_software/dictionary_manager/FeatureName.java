package com.paragon_software.dictionary_manager;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;

import java.io.Serializable;

public class FeatureName implements Parcelable, Serializable, Comparable<FeatureName>
{

  public static final Parcelable.Creator< FeatureName > CREATOR = new Parcelable.Creator< FeatureName >()
  {
    @Override
    public FeatureName createFromParcel( Parcel source )
    {
      return new FeatureName(source);
    }

    @Override
    public FeatureName[] newArray( int size )
    {
      return new FeatureName[size];
    }
  };
  @NonNull
  private final String mName;

  private FeatureName()
  {
    mName = "";
  }

  public FeatureName( @NonNull final String name )
  {
    if ( name.trim().length() <= 0 )
    {
      throw new IllegalArgumentException("Feature name can not be empty");
    }

    mName = name;
  }

  private FeatureName( Parcel in )
  {
    mName = in.readString();
  }

  public static FeatureName getEmpty()
  {
    return new FeatureName();
  }

  @Override
  public int hashCode()
  {
    return mName.hashCode();
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
    FeatureName other = (FeatureName) obj;
    return other.mName.equals(this.mName);
  }

  @NonNull
  @Override
  public String toString()
  {
    return mName;
  }

  @Override
  public int describeContents()
  {
    return 0;
  }

  @Override
  public void writeToParcel( Parcel dest, int flags )
  {
    dest.writeString(mName);
  }

  public boolean isEmpty()
  {
    return mName.isEmpty();
  }

  public static boolean isEmpty( FeatureName featureName )
  {
    return null == featureName || featureName.isEmpty();
  }

  public static FeatureName[] of( String[] strings )
  {
    FeatureName[] featureNames = new FeatureName[strings.length];
    for ( int i = 0 ; i < strings.length ; i++ )
    {
      featureNames[i] = new FeatureName(strings[i]);
    }
    return featureNames;
  }

  @Override
  public int compareTo( @NonNull FeatureName featureName )
  {
    return mName.compareTo(featureName.mName);
  }
}