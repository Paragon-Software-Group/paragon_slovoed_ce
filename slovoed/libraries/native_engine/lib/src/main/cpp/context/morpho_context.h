//
// Created by kovtunov on 21.11.18.
//

#ifndef SLOVOED_MORPHOCONTEXT_H
#define SLOVOED_MORPHOCONTEXT_H


#include "Morphology/MorphoData.h"
#include "context/builder/LayerAccess.h"

class MorphoContext : public IEnvAware
{

  private:
    MorphoData* mpData;
    EnvAwareSdcReader* mpSDCFile;
    LayerAccess* mpLayerAccess;

  public:

    MorphoContext( MorphoData* pData, EnvAwareSdcReader* pSdcFile, LayerAccess* pLayerAccess );

    ~MorphoContext();

    MorphoData* getMorphoData();

    EnvAwareSdcReader* getSDCFile();

    LayerAccess* getLayerAccess();

    void SetEnv( JNIEnv* env ) override;
};


#endif //SLOVOED_MORPHOCONTEXT_H
