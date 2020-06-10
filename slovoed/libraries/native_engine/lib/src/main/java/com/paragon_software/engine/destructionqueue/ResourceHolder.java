package com.paragon_software.engine.destructionqueue;

import androidx.annotation.NonNull;

import java.lang.ref.PhantomReference;
import java.util.concurrent.atomic.AtomicBoolean;

public class ResourceHolder< T extends ManagedResource > extends PhantomReference< Object > implements ManagedResource
{
  private static final DestructionThread DESTRUCTION_THREAD = new DestructionThread();

  static
  {
    DESTRUCTION_THREAD.setDaemon(true);
    DESTRUCTION_THREAD.start();
  }

  @NonNull
  private final T resource;

  private final AtomicBoolean disposed = new AtomicBoolean(false);

  public static < T extends ManagedResource > ResourceHolder< T > create( Object referent, @NonNull T resource )
  {
    synchronized ( DESTRUCTION_THREAD )
    {
      ResourceHolder< T > res = new ResourceHolder<>(referent, resource);
      DESTRUCTION_THREAD.rememberReference(res);
      return res;
    }
  }

  private ResourceHolder( Object _referent, @NonNull T _resource )
  {
    super(_referent, DESTRUCTION_THREAD.getReferenceQueue());
    resource = _resource;
  }

  @NonNull
  public T getResource()
  {
    return resource;
  }

  @Override
  public void free()
  {
    if ( disposed.compareAndSet(false, true) )
    {
      resource.free();
    }
  }
}
