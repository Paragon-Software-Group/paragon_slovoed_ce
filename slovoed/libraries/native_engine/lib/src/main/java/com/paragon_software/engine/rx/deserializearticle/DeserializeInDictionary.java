package com.paragon_software.engine.rx.deserializearticle;

import android.content.Context;
import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import io.reactivex.Observable;
import io.reactivex.ObservableSource;
import io.reactivex.functions.Function;

public class DeserializeInDictionary
    implements Function< Task< SingleDictionaryParams >, ObservableSource< Task< Result > > >
{
  private final Context              applicationContext;
  private final DictionaryManagerAPI dictionaryManager;

  @NonNull
  private final RunOn< Task< Result > > runOn;

  DeserializeInDictionary( Context _context, DictionaryManagerAPI _dictionaryManager, @NonNull RunOn< Task< Result > > _runOn )
  {
    applicationContext = _context;
    dictionaryManager = _dictionaryManager;
    runOn = _runOn;
  }

  @Override
  public ObservableSource< Task< Result > > apply( Task< SingleDictionaryParams > task )
  {
    return Observable.create(new DeserializerSource(applicationContext, dictionaryManager, task)).compose(runOn);
  }
}
