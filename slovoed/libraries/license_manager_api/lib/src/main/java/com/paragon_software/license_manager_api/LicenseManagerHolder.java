/*
 * license_manager_api
 *
 *  Created on: 03.04.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.license_manager_api;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Collection;
import java.util.Collections;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

import com.paragon_software.dictionary_manager.FeatureName;
import com.paragon_software.license_manager_api.exceptions.*;

@SuppressWarnings( "unused" )
public final class LicenseManagerHolder implements LicenseManager
{
  @NonNull
  private final Queue< TaskHolder > tasks   = new ConcurrentLinkedQueue<>();
  @Nullable
  private       LicenseManager      manager = null;

  public void initialize( @NonNull final LicenseManager manager ) throws ActivateException
  {
    this.manager = manager;
    final Queue< TaskHolder > workTasks;
    synchronized ( tasks )
    {
      workTasks = new ConcurrentLinkedQueue<>(tasks);
      tasks.clear();
    }
    if ( 0 != workTasks.size() )
    {
      for ( TaskHolder task : workTasks )
      {
        task.process(manager);
      }
    }
  }

  @Override
  public void update( @NonNull Context context )
  {
    if ( null != manager )
    {
      manager.update(context);
    }
  }

  @Override
  public void consume( @NonNull Context appContext, @NonNull FeatureName featureName )
      throws ItemUnavailableException, ItemNotOwnedException
  {
    if ( null != manager )
    {
      manager.consume(appContext, featureName);
    }
  }

  @NonNull
  @Override
  public LicenseFeature.FEATURE_STATE checkFeature( @NonNull final FeatureName name )
  {
    return null != this.manager ? this.manager.checkFeature(name) : LicenseFeature.FEATURE_STATE.DISABLED;
  }

  @Nullable
  @Override
  public LicenseFeature getFeature( @NonNull final FeatureName name )
  {
    return null != this.manager ? this.manager.getFeature(name) : null;
  }

  @NonNull
  @Override
  public Collection< LicenseFeature > getFeatures()
  {
    return null != this.manager ? this.manager.getFeatures() : Collections.< LicenseFeature >emptyList();
  }

  @Override
  public void activationBegin( @NonNull final Activity activity, @NonNull final FeatureName featureName )
      throws ItemUnavailableException, ItemAlreadyOwnedException, UserAccountManagementException
  {
    if ( null != manager )
    {
      manager.activationBegin(activity, featureName);
    }
    else
    {
      synchronized ( tasks )
      {
        tasks.add(TaskHolder.activationBegin(activity, featureName, null));
      }
    }
  }

  @Override
  public void activationBegin( @NonNull Activity activity, @NonNull FeatureName featureName, SKU skuToActivate )
      throws ItemUnavailableException, ItemAlreadyOwnedException, UserAccountManagementException
  {
    if ( null != manager )
    {
      manager.activationBegin(activity, featureName, skuToActivate);
    }
    else
    {
      synchronized ( tasks )
      {
        tasks.add(TaskHolder.activationBegin(activity, featureName, null, skuToActivate));
      }
    }
  }

  @Override
  public void activationBegin( @NonNull final Activity activity, @NonNull final FeatureName featureName,
                               @NonNull final String sourceName )
      throws ItemUnavailableException, ItemAlreadyOwnedException, UserAccountManagementException,
             InvalidActivationSourceException
  {
    if ( null != manager )
    {
      manager.activationBegin(activity, featureName, sourceName);
    }
    else
    {
      synchronized ( tasks )
      {
        tasks.add(TaskHolder.activationBegin(activity, featureName, sourceName));
      }
    }
  }

  @Override
  public boolean activationEnd( final int requestCode, int resultCode, @NonNull final Intent intent ) throws ActivateException
  {
    if ( null != manager )
    {
      return manager.activationEnd(requestCode, resultCode, intent);
    }
    else
    {
      synchronized ( tasks )
      {
        tasks.add(TaskHolder.activationEnd(requestCode, resultCode, intent));
      }
    }
    return false;
  }

  @Override
  public void registerNotifier( @NonNull final Notifier notifier )
  {
    if ( null != manager )
    {
      manager.registerNotifier(notifier);
    }
    else
    {
      synchronized ( tasks )
      {
        tasks.add(TaskHolder.registerNotifier(notifier));
      }
    }
  }

  @Override
  public void unregisterNotifier( @NonNull final Notifier notifier )
  {
    if ( null != manager )
    {
      manager.unregisterNotifier(notifier);
    }
    else
    {
      synchronized ( tasks )
      {
        tasks.add(TaskHolder.unregisterNotifier(notifier));
      }
    }
  }

  private static class TaskHolder
  {
    @Nullable
    final         Activity activity;
    @Nullable
    final         FeatureName   featureName;
    @Nullable
    final         String   sourceName;
    private final int resultCode;
    @Nullable
    private       SKU sku;
    final int requestCode;
    @Nullable
    final         Intent   intent;
    @Nullable
    final         Notifier notifier;
    @NonNull
    private final TYPE     type;

    private TaskHolder( @Nullable final Activity activity, @Nullable final FeatureName featureName,
                        @Nullable final String sourceName, @Nullable SKU sku, final int requestCode, final int resultCode, @Nullable final Intent intent,
                        @Nullable final Notifier notifier, @NonNull final TYPE type )
    {
      this.activity = activity;
      this.featureName = featureName;
      this.sourceName = sourceName;
      this.sku = sku;
      this.requestCode = requestCode;
      this.resultCode = resultCode;
      this.intent = intent;
      this.notifier = notifier;
      this.type = type;
    }

    static TaskHolder activationBegin( @NonNull final Activity activity, @NonNull final FeatureName featureName,
                                       @Nullable final String sourceName, @NonNull final SKU sku )
    {
      return new TaskHolder(activity, featureName, sourceName, sku, -1, -1, null, null, TYPE.ACTIVATE_BEGIN);
    }

    static TaskHolder activationBegin( @NonNull final Activity activity, @NonNull final FeatureName featureName,
                                       @Nullable final String sourceName )
    {
      return new TaskHolder(activity, featureName, sourceName, null, -1, -1,null, null, TYPE.ACTIVATE_BEGIN);
    }

    static TaskHolder activationEnd( final int requestCode, final int resultCode, @NonNull final Intent intent )
    {
      return new TaskHolder(null, null, null, null, requestCode, resultCode, intent, null, TYPE.ACTIVATE_END);
    }

    static TaskHolder registerNotifier( @NonNull final Notifier notifier )
    {
      return new TaskHolder(null, null, null, null, -1, -1, null, notifier, TYPE.REGISTER_NOTIFIER);
    }

    static TaskHolder unregisterNotifier( @NonNull final Notifier notifier )
    {
      return new TaskHolder(null, null, null, null, -1, -1, null, notifier, TYPE.UNREGISTER_NOTIFIER);
    }

    void process( LicenseManager manager ) throws ActivateException
    {
      switch ( type )
      {
        case ACTIVATE_BEGIN:
          if ( null == sourceName )
          {
            if ( null != activity && null != featureName )
            {
              if (null != sku)
              {
                manager.activationBegin(activity, featureName, sku);
              }
              else
              {
                manager.activationBegin(activity, featureName);
              }
            }
          }
          else
          {
            if ( null != activity && null != featureName )
            {
              manager.activationBegin(activity, featureName, sourceName);
            }
          }
          break;
        case ACTIVATE_END:
          if ( null != intent )
          {
            manager.activationEnd(requestCode, resultCode, intent);
          }
          break;
        case REGISTER_NOTIFIER:
          if ( null != notifier )
          {
            manager.registerNotifier(notifier);
          }
          break;
        case UNREGISTER_NOTIFIER:
          if ( null != notifier )
          {
            manager.unregisterNotifier(notifier);
          }
          break;
      }
    }

    enum TYPE
    {
      ACTIVATE_BEGIN,
      ACTIVATE_END,
      REGISTER_NOTIFIER,
      UNREGISTER_NOTIFIER
    }
  }
}
