package com.paragon_software.engine.rx.searchalldictionaries;

import android.content.Context;
import android.util.SparseArray;

import java.util.ArrayList;
import java.util.List;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.engine.nativewrapper.ArticleItemFactory;
import com.paragon_software.engine.nativewrapper.ListInfo;
import com.paragon_software.engine.nativewrapper.ListType;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.nativewrapper.WordVariant;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.native_engine.SortType;
import io.reactivex.ObservableEmitter;
import io.reactivex.ObservableOnSubscribe;

class FtsSource implements ObservableOnSubscribe< Task< Result > >
{
  private final Context applicationContext;

  private static final WordVariant[] VARIANTS_FROM_SEARCH = { WordVariant.Show };

  private final Task< SingleFileParams > task;

  FtsSource( Context _context, Task< SingleFileParams > _task )
  {
    applicationContext = _context;
    task = _task;
  }

  @Override
  public void subscribe( ObservableEmitter< Task< Result > > emitter )
  {
    //    Log.d("SLOVOED", "dict work: " + Thread.currentThread().getName());
    if ( !task.isCanceled() )
    {
      SingleFileParams params = task.get();
      NativeDictionary dictionary = NativeDictionary.open(applicationContext, params.dictionary, false, true);
      if ( dictionary != null )
      {
        SparseArray< ListInfo > ftsLists = dictionary.getLists(ListType.FullTextSearchHeadword);
        for ( int i = 0 ; i < ftsLists.size() ; i++ )
        {
          dictionary
              .fullTextSearch(ftsLists.keyAt(i), params.query, params.maxWords, SortType.Full);
        }
        SparseArray< ListInfo > searchResultLists = dictionary.getLists(ListType.RegularSearch);
        for ( int i = 0 ; i < searchResultLists.size() ; i++ )
        {
          int listIndex = searchResultLists.keyAt(i);
          ListInfo listInfo = searchResultLists.valueAt(i);
          int wordCount = dictionary.getListCurrentSize(listIndex);
          List< ArticleItem > items = new ArrayList<>(wordCount);
          int[] variants = listInfo.getVariants(VARIANTS_FROM_SEARCH);
          String words[] = new String[variants.length];
          int realLocation[] = new int[2];
          for ( int j = 0 ; j < wordCount ; j++ ) // TODO: replace with proper article items creation
          {
            dictionary.getWordByLocalIndex(listIndex, j, variants, words);
            if ( ( words[0] != null ) && dictionary.getRealWordLocation(listIndex, j, realLocation) )
            {
              items.add(ArticleItemFactory.createNormal(params.dictionary.getId(), dictionary, realLocation[0],
                                                        realLocation[1], variants, words, null));
            }
          }
          emitter.onNext(task.spawn(new Result(params.dictionary.getId(), items)));
        }
        dictionary.close();
      }
    }
    //    else
    //    {
    //      Log.d("SLOVOED", "dict work canceled");
    //    }
    emitter.onComplete();
  }
}
