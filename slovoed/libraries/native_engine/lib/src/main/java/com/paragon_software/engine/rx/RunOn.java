package com.paragon_software.engine.rx;

import android.os.Handler;
import android.os.Looper;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.concurrent.TimeUnit;

import io.reactivex.Observable;
import io.reactivex.ObservableSource;
import io.reactivex.ObservableTransformer;
import io.reactivex.Scheduler;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.schedulers.Schedulers;

public class RunOn< T > implements ObservableTransformer< T, T >
{
  public enum What
  {
    Subscribe
        {
          @NonNull
          @Override
          < T > Observable< T > apply( @NonNull Observable< T > upstream, @NonNull Scheduler scheduler,
                                       long debounceMs )
          {
            return upstream.subscribeOn(scheduler);
          }
        },
    Observe
        {
          @NonNull
          @Override
          < T > Observable< T > apply( @NonNull Observable< T > upstream, @NonNull Scheduler scheduler,
                                       long debounceMs )
          {
            Observable< T > res;
            if ( debounceMs < 0L )
            {
              res = upstream.observeOn(scheduler);
            }
            else
            {
              res = upstream.debounce(debounceMs, TimeUnit.MILLISECONDS, scheduler);
            }
            return res;
          }
        };

    @NonNull
    abstract < T > Observable< T > apply( @NonNull Observable< T > upstream, @NonNull Scheduler scheduler,
                                          long debounceMs );
  }

  public enum Where
  {
    Caller
        {
          @Override
          Scheduler getScheduler( @NonNull Looper looper )
          {
            return AndroidSchedulers.from(looper, true);
          }
        },
    Computation
        {
          @Override
          Scheduler getScheduler( @NonNull Looper looper )
          {
            return Schedulers.computation();
          }
        };

    abstract Scheduler getScheduler( @NonNull Looper looper );
  }

  @NonNull
  private final What what;

  @NonNull
  private final Where where;

  @Nullable
  private final Handler handler;

  private final long debounceMs;

  public RunOn( @NonNull What _what, @NonNull Where _where )
  {
    this(_what, _where, -1L);
  }

  public RunOn( @NonNull What _what, @NonNull Where _where, long _debounceMs )
  {
    what = _what;
    where = _where;
    Looper looper = Looper.myLooper();
    if ( looper != null )
    {
      handler = new Handler(looper);
    }
    else
    {
      handler = null;
    }
    debounceMs = _debounceMs;
  }

  @Override
  public ObservableSource< T > apply( Observable< T > upstream )
  {
    Observable< T > res = upstream;
    Looper looper = null;
    if ( handler != null )
    {
      looper = handler.getLooper();
    }
    if ( ( looper != null ) && ( res != null ) )
    {
      res = what.apply(res, where.getScheduler(looper), debounceMs);
    }
    return res;
  }
}
