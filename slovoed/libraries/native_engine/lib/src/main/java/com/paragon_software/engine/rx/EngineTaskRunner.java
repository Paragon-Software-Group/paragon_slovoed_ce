package com.paragon_software.engine.rx;

import android.content.Context;
import androidx.annotation.Nullable;

import java.util.Collection;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import io.reactivex.functions.Predicate;

public abstract class EngineTaskRunner< Params > implements DictionaryManagerAPI.IDictionaryListObserver
{
  protected Context              applicationContext = null;
  protected DictionaryManagerAPI dictionaryManager  = null;

  public void init( Context _context, DictionaryManagerAPI _dictionaryManager )
  {
    if ( ( applicationContext != null ) || ( dictionaryManager != null ) )
    {
      throw new IllegalStateException("init called twice");
    }
    applicationContext = _context.getApplicationContext();
    dictionaryManager = _dictionaryManager;
    dictionaryManager.registerDictionaryListObserver(this);
  }

  @Override
  public void onDictionaryListChanged()
  {
    Collection< Task< Params > > currentTasks = getCurrentTasks();
    for ( Task< Params > task : currentTasks )
    {
      task.cancel();
      emitNewTask(task.get());
    }
  }

  protected void assertInitCalled()
  {
    if ( ( dictionaryManager == null ) || ( applicationContext == null ) )
    {
      throw new IllegalStateException("init not called");
    }
  }

  @Nullable
  protected Dictionary findDictionary( @Nullable Dictionary.DictionaryId dictionaryId )
  {
    Dictionary res = null;
    if ( ( dictionaryId != null ) && ( dictionaryManager != null ) )
    {
      for ( Dictionary dictionary : dictionaryManager.getDictionaries() )
      {
        if ( dictionaryId.equals(dictionary.getId()) )
        {
          res = dictionary;
          break;
        }
      }
    }
    return res;
  }

  protected void emitNewTask( Params params )
  {
    emitTask(Task.createInitial(params));
  }

  protected abstract void emitTask( Task< Params > task );

  protected abstract Collection< Task< Params > > getCurrentTasks();

  protected static class OnlyUncanceled implements Predicate< Cancelable >
  {
    public OnlyUncanceled()
    {

    }

    @Override
    public boolean test( Cancelable task )
    {
      return !task.isCanceled();
    }
  }
}
