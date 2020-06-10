//
// Created by kovtunov on 21.11.18.
//

#include "morpho_context.h"

MorphoContext::MorphoContext(
        MorphoData* pData,
        EnvAwareSdcReader* pSdcFile,
        LayerAccess* pLayerAccess
)
        :
        mpData( pData ), mpSDCFile( pSdcFile ), mpLayerAccess( pLayerAccess )
{
}

MorphoContext::~MorphoContext()
{
    if ( mpData )
    {
        delete mpData;
    }
    if ( mpSDCFile )
    {
        delete mpSDCFile;
    }
    if ( mpLayerAccess )
    {
        delete mpLayerAccess;
    }
}

MorphoData* MorphoContext::getMorphoData()
{
    return mpData;
}

EnvAwareSdcReader* MorphoContext::getSDCFile()
{
    return mpSDCFile;
}

LayerAccess* MorphoContext::getLayerAccess()
{
    return mpLayerAccess;
}

void MorphoContext::SetEnv(JNIEnv *env)
{
    mpSDCFile->SetEnv(env);
    mpLayerAccess->SetEnv(env);
}