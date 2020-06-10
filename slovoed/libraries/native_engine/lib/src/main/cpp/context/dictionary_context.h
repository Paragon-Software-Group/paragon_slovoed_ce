#ifndef DICTIONARY_CONTEXT_H
#define DICTIONARY_CONTEXT_H

#include <vector>
#include "context/builder/LayerAccess.h"
#include "SldDictionary.h"
#include "env_aware.h"
#include "morpho_context.h"

class CDictionaryContext : public IEnvAware
{
  private:
    CSldDictionary* mpDictionary;
    EnvAwareSdcReader* mpSDCFile;
    LayerAccess* mpLayerAccess;
    std::vector<MorphoContext*> mpMorphoContextList;

  public:
    enum MorphoUsage
    {
        eFts, eWildCard, eHint, eGetWordReference
    };

    CDictionaryContext( CSldDictionary* pDictionary, EnvAwareSdcReader* pSdcFile, LayerAccess* pLayerAccess );

    ~CDictionaryContext();

    CSldDictionary* GetDictionary();

    LayerAccess* GetLayerAccess();

    bool IsInternalMorphoAvailable( UInt32 language, MorphoUsage usage );

    MorphoData* GetExternalMorpho( UInt32 language, MorphoUsage usage );

    void AddMorphoContext(MorphoContext *morphoContext);

    void SetEnv( JNIEnv* env ) override;

    ESldError Close();
};

#endif //DICTIONARY_CONTEXT_H
