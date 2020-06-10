package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;

import com.paragon_software.settings_manager.exceptions.AlreadyInProgressException;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.StorageTransferException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

import java.io.Serializable;
import java.util.Collection;
import java.util.Map;
import java.util.TreeMap;

public abstract class MockSettingsManager extends SettingsManagerAPI
{
    private Map<String, Serializable> mMap = new TreeMap<>();

    @Override
    public boolean sendToStorages() throws AlreadyInProgressException, LocalResourceUnavailableException, ManagerInitException, StorageTransferException
    {
        return false;
    }

    @Override
    public void save(@NonNull String name, @NonNull Serializable data, boolean needBackup) throws ManagerInitException, LocalResourceUnavailableException
    {
        mMap.put(name, data);
    }

    @Override
    public <T> T load(@NonNull String name, @NonNull T defValue) throws WrongTypeException, ManagerInitException
    {
        T res = defValue;
        if(mMap.containsKey(name))
            res = (T) mMap.get(name);
        return res;
    }

    @Override
    public boolean isRestored()
    {
        return false;
    }

    @Override
    public Collection<String> storageList()
    {
        return null;
    }

}
