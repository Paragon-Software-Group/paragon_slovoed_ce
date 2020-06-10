package com.paragon_software.engine.destructionqueue;

import java.lang.ref.Reference;
import java.lang.ref.ReferenceQueue;
import java.util.HashSet;
import java.util.Set;

class DestructionThread extends Thread
{
  private final ReferenceQueue< Object > referenceQueue = new ReferenceQueue<>();
  private final Set< Reference< ? > >    allReferences  = new HashSet<>();

  @Override
  public void run()
  {
    while ( true )
    {
      try
      {
        Reference< ? > reference = referenceQueue.remove();
        forgetReference(reference);
        if ( reference instanceof ManagedResource )
        {
          ( (ManagedResource) reference ).free();
        }
        reference.clear();
      }
      catch ( InterruptedException ignore )
      {

      }
    }
  }

  ReferenceQueue< Object > getReferenceQueue()
  {
    return referenceQueue;
  }

  void rememberReference(Reference< ? > reference)
  {
    allReferences.add(reference);
  }

  private synchronized void forgetReference( Reference< ? > reference )
  {
    allReferences.remove(reference);
  }
}
