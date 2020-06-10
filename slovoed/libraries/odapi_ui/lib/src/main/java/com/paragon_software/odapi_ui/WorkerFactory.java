package com.paragon_software.odapi_ui;

import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.Pair;

final class WorkerFactory {
    @NonNull
    static Worker createWorker(@NonNull Bundle data) {
        Worker res = null;
        ErrorResult error = null;
        Pair<ErrorResult, Client> clientPair = parseParam(data, Worker.EXTRA_CLIENT, Client.FACTORY);
        Pair<ErrorResult, Query> queryPair = parseParam(data, Worker.EXTRA_QUERY, Query.FACTORY);
        Pair<ErrorResult, String> valuePair = parseParam(data, Worker.EXTRA_VALUE, ParamParser.IDENTITY);
        for(ErrorResult e : new ErrorResult[] {clientPair.first, queryPair.first, valuePair.first})
            if(e != null) {
                error = e;
                break;
            }
        if(error == null)
            error = checkClient(clientPair.second, Worker.EXTRA_CLIENT);
        if(error == null)
            error = checkLicense();
        if(error == null)
            if(Query.Type.Translate.equals(queryPair.second.type))
                res = new TranslateWorker(queryPair.second, valuePair.second);
            else if(Query.Type.Morphology.equals(queryPair.second.type))
                res = new MorphologyWorker(queryPair.second, valuePair.second);
            else
                error = new WrongParamsResult(Worker.EXTRA_QUERY, WrongParamsResult.What.Illegal);
        if(error != null)
            res = new DirectWorker(error);
        return res;
    }

    @NonNull
    private static <T> Pair<ErrorResult, T> parseParam(@NonNull Bundle bundle, @NonNull String key, @NonNull ParamParser.ParamFactory<T> paramFactory) {
        T res = null;
        ErrorResult error = null;
        try {
            res = ParamParser.parse(bundle, key, paramFactory);
        }
        catch (ParamParser.MissingParamException e) {
            error = new WrongParamsResult(key, WrongParamsResult.What.Missed);
        }
        catch (ParamParser.IllegalParamException e) {
            error = new WrongParamsResult(key, WrongParamsResult.What.Illegal);
        }
        return new Pair<>(error, res);
    }

    @Nullable
    private static ErrorResult checkClient(@NonNull Client client, @NonNull String clientKey) {
        ErrorResult res = null;
        try {
            int clientApiVersionCode = client.getApiVersionCode();
            int minApiVersionCode = Client.getMinApiVersionCode(Worker.getContext());
            if(clientApiVersionCode < minApiVersionCode)
                res = new ClientApiNotSupportedResult(clientApiVersionCode, minApiVersionCode);
        }
        catch (Exception e) {
            res = new WrongParamsResult(clientKey, WrongParamsResult.What.Illegal);
        }
        return res;
    }

    @Nullable
    private static ErrorResult checkLicense() {
        ErrorResult res = null;
        Boolean b = Worker.checkLicense();
        if(b == null)
            res = new CommonErrorResult(CommonErrorResult.Kind.AppStateError, "Initialization error");
        else if(!b)
            res = new CommonErrorResult(CommonErrorResult.Kind.AppStateError, "EULA not accepted");
        return res;
    }
}
