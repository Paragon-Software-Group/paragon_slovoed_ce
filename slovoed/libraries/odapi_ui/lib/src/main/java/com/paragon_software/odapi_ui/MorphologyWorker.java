package com.paragon_software.odapi_ui;

import android.content.Context;
import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.native_engine.SyncEngineAPI;

import java.util.Collection;

class MorphologyWorker extends QueryWorkerAbstract {

    MorphologyWorker(@NonNull Query query, @NonNull String value) {
        super(query, value);
    }

    @NonNull
    @Override
    Result getResult() {
        Result res;
        SyncEngineAPI engine = getEngine();
        Context context = getContext();
        if((engine != null) && (context != null)) {
            Collection<DictionaryAndDirection> dnds = getDictionariesAndDirections();
            DictionaryAndDirection dnd = getSelectedDictionaryAndDirection();
            if(dnd != null)
                if(dnds.contains(dnd)) {
                    String[] baseForms = engine.getBaseForms(dnd.getDictionaryId(), dnd.getDirection(), mValue);
                    if(Query.Specific.MorphoBaseForms.equals(mQuery.specific))
                        res = new MorphoResult(baseForms);
                    else if(mQuery.specific != null)
                        res = new WrongParamsResult("Query \"" + mQuery.specific.getName() + "\" is unsupported");
                    else
                        res = new WrongParamsResult("Query not found");
                }
                else
                    res = openCatalog(context, dnd.getDictionaryId());
            else
                res = openCatalog(context, null);
        }
        else
            res = new CommonErrorResult(CommonErrorResult.Kind.AppStateError, "Initialization error");
        return res;
    }
}
