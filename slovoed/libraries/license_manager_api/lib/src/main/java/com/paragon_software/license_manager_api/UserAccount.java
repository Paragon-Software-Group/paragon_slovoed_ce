/*
 * license_manager_api
 *
 *  Created on: 08.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.license_manager_api;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.UnsupportedEncodingException;

/**
 * Information about user account
 */
public final class UserAccount implements Cloneable
{
  @Nullable
  private final String id;
  @Nullable
  private final String accountSource;
  @Nullable
  private final String name;
  @Nullable
  private final String eMail;
  @Nullable
  private final String password;

  /*
   * Caching calculated parameters
   */
  @Nullable
  private String displayCache = null;
  private int    hashCache    = -1;

  private UserAccount( @Nullable final String id, @Nullable final String accountSource, @Nullable final String name,
                       @Nullable final String eMail, @Nullable final String password )
  {
    this.id = id;
    this.accountSource = accountSource;
    this.name = name;
    this.eMail = eMail;
    this.password = password;
  }

  public UserAccount( @NonNull final UserAccount account, @NonNull final String id,
                      @NonNull final String accountSource )
  {
    this.id = id;
    this.accountSource = accountSource;
    this.name = account.name;
    this.eMail = account.eMail;
    this.password = account.password;
  }

  @NonNull
  public static UserAccount userAccountByName( @NonNull final String name )
  {
    return new UserAccount(null, null, name, null, null);
  }

  @NonNull
  public static UserAccount userAccountByName( @NonNull final String name, @NonNull final String password )
  {
    return new UserAccount(null, null, name, null, password);
  }

  @NonNull
  public static UserAccount userAccountByEMail( @NonNull final String eMail )
  {
    return new UserAccount(null, null, null, eMail, null);
  }

  @NonNull
  public static UserAccount userAccountByEMail( @NonNull final String eMail, @NonNull final String password )
  {
    return new UserAccount(null, null, null, eMail, password);
  }

  @NonNull
  public static UserAccount userAccountByNameAndEMail( @NonNull final String name, @NonNull final String eMail )
  {
    return new UserAccount(null, null, name, eMail, null);
  }

  @NonNull
  public static UserAccount userAccountByNameAndEMail( @NonNull final String name, @NonNull final String eMail,
                                                       @NonNull final String password )
  {
    return new UserAccount(null, null, name, eMail, password);
  }

  @NonNull
  public static UserAccount userAccountByIdAndNameAndEMail( @NonNull final String id, @NonNull final String name,
                                                            @NonNull final String eMail,
                                                            @NonNull final String password )
  {
    return new UserAccount(id, name + "-source", name, eMail, password);
  }

  /**
   * Get User account information from byte array
   *
   * @param data data with serialized information
   *
   * @return restored user account information
   *
   * @throws UnsupportedEncodingException if system not support "UTF-8"
   */
  @Nullable
  static public UserAccount fromBytes( @NonNull final byte[] data ) throws UnsupportedEncodingException
  {
    final String result = new String(data, "UTF-8");
    final String[] elements = result.split("[|][%][|]");
    if ( 4 == elements.length )
    {
      final String accountSource = 0 == elements[0].length() ? null : elements[0];
      final String id = 0 == elements[1].length() ? null : elements[1];
      final String name = 0 == elements[2].length() ? null : elements[2];
      final String eMail = 0 == elements[3].length() ? null : elements[3];
      return new UserAccount(id, accountSource, name, eMail, null);
    }
    return null;
  }

  /**
   * User account was registered
   *
   * @return true if account was registered
   */
  public boolean isRegistered()
  {
    return null != id;
  }

  /**
   * Account source getter
   *
   * @return account source
   */
  @Nullable
  public String getAccountSource()
  {
    return accountSource;
  }

  /**
   * User id getter
   *
   * @return User id
   */
  @Nullable
  public String getId()
  {
    return id;
  }

  /**
   * User name getter
   *
   * @return user name
   */
  @Nullable
  public String getName()
  {
    return name;
  }

  /**
   * User e-mail getter
   *
   * @return User e-mail
   */
  @Nullable
  public String getEMail()
  {
    return eMail;
  }

  /**
   * User password getter
   *
   * @return user password
   */
  @Nullable
  public String getPassword()
  {
    return password;
  }

  /**
   * Calculate hash from object
   *
   * @return hashCode
   */
  @Override
  public int hashCode()
  {
    if ( -1 == hashCache )
    {
      hashCache = 1;
      if ( null != id && null != accountSource )
      {
        hashCache = 17 + accountSource.hashCode();
        hashCache = 31 * hashCache + id.hashCode();
      }
      else
      {
        hashCache = 961 * hashCache + ( name != null ? name.hashCode() : 11 );
        hashCache = 29791 * hashCache + ( eMail != null ? eMail.hashCode() : 13 );
        hashCache = 923521 * hashCache + ( password != null ? password.hashCode() : 17 );
      }
    }
    return hashCache;
  }

  /**
   * Compare object
   * Object equal if id is equals, or name and password is equals, or eMail and password is equal
   *
   * @param o other object
   *
   * @return true if object is equals
   */
  @Override
  public boolean equals( final Object o )
  {
    if ( this == o )
    {
      return true;
    }
    if ( o instanceof UserAccount )
    {
      final UserAccount that = (UserAccount) o;
      if ( null != id && null != accountSource )
      {
        return id.equals(that.id) && accountSource.equals(that.accountSource);
      }
      if ( ( null == name || name.equals(that.name) ) && ( null == eMail || eMail.equals(that.eMail) ) )
      {
        return null == password || password.equals(that.password);
      }
    }
    return false;
  }

  /**
   * Clone object
   *
   * @return new Object with current state
   *
   * @throws CloneNotSupportedException on clone problem
   */
  @NonNull
  @Override
  public Object clone() throws CloneNotSupportedException
  {
    return super.clone();
  }

  /**
   * Display class content
   *
   * @return Human readable user account information
   */
  @NonNull
  @Override
  public String toString()
  {
    if ( null == displayCache )
    {
      boolean addComma = false;
      StringBuilder result = new StringBuilder("{");
      if ( null != id )
      {
        result.append("id='");
        result.append(id);
        result.append("', source='");
        result.append(accountSource);
        result.append('\'');
        addComma = true;
      }
      if ( null != name )
      {
        if ( addComma )
        {
          result.append(", ");
        }
        result.append("name='");
        result.append(name);
        result.append('\'');
        addComma = true;
      }
      if ( null != eMail )
      {
        if ( addComma )
        {
          result.append(", ");
        }
        result.append("e-mail='");
        result.append(eMail);
        result.append('\'');
        addComma = true;
      }
      if ( null != password )
      {
        if ( addComma )
        {
          result.append(", ");
        }
        result.append("password='");
        result.append(password);
        result.append('\'');
      }
      result.append('}');
      displayCache = result.toString();
    }
    return displayCache;
  }

  /**
   * Get byte array from user account information
   *
   * @return serialized information
   *
   * @throws UnsupportedEncodingException if system not support "UTF-8"
   */
  @Nullable
  public byte[] toBytes() throws UnsupportedEncodingException
  {
    if ( null != id )
    {
      final StringBuilder result = new StringBuilder(accountSource);
      result.append("|%|");
      result.append(id);
      result.append("|%|");
      if ( null != name )
      {
        result.append(name);
      }
      result.append("|%|");
      if ( null != eMail )
      {
        result.append(eMail);
      }
      return result.toString().getBytes("UTF-8");
    }
    return null;
  }
}
