#include "dictionary_context.h"

CDictionaryContext::CDictionaryContext(
    CSldDictionary* pDictionary,
    EnvAwareSdcReader* pSdcFile,
    LayerAccess* pLayerAccess
)
    :
    mpDictionary( pDictionary ), mpSDCFile( pSdcFile ), mpLayerAccess( pLayerAccess )
{
}

CDictionaryContext::~CDictionaryContext()
{
  if ( mpDictionary )
  {
    delete mpDictionary;
  }
  if ( mpSDCFile )
  {
    delete mpSDCFile;
  }
  if ( mpLayerAccess )
  {
    delete mpLayerAccess;
  }
  for (uint i = 0; i < mpMorphoContextList.size(); ++i)
  {
      delete mpMorphoContextList.at(i);
  }

}

CSldDictionary* CDictionaryContext::GetDictionary()
{
  return mpDictionary;
}

LayerAccess* CDictionaryContext::GetLayerAccess()
{
  return mpLayerAccess;
}

bool CDictionaryContext::IsInternalMorphoAvailable( UInt32 language, CDictionaryContext::MorphoUsage usage )
{
    ESldError error;
    Int32 Count;
    error = mpDictionary->GetNumberOfLists(&Count);
    if (error != eOK)
        return false;

    bool isMorphologyBaseForm = false;
    bool isMorphologyInflectionForm = false;

    for (Int32 i = 0; i < Count; i++) {
        const CSldListInfo *listInfo = nullptr;
        error = mpDictionary->GetWordListInfo(i, &listInfo);
        if (error != eOK)
            return false;
        if (listInfo->GetUsage() == eWordListType_MorphologyBaseForm && listInfo->GetLanguageFrom() == language) {
            isMorphologyBaseForm = true;
        }
        if (listInfo->GetUsage() == eWordListType_MorphologyInflectionForm && listInfo->GetLanguageFrom() == language) {
            isMorphologyInflectionForm = true;
        }
        if (isMorphologyBaseForm && isMorphologyInflectionForm) return true;
    }
    return false;
}

MorphoData* CDictionaryContext::GetExternalMorpho( UInt32 language, CDictionaryContext::MorphoUsage usage )
{
  MorphoData* morphoData = nullptr;
  for (uint i = 0; i < mpMorphoContextList.size(); ++i)
  {
    MorphoData* data = mpMorphoContextList.at(i)->getMorphoData();
    if (data->GetLanguageCode() == language)
    {
      morphoData = data;
      break;
    }
  }
  return morphoData;
}

void CDictionaryContext::AddMorphoContext(MorphoContext *morphoContext)
{
    mpMorphoContextList.push_back(morphoContext);
}

void CDictionaryContext::SetEnv( JNIEnv* env )
{
  mpSDCFile->SetEnv( env );
  mpLayerAccess->SetEnv( env );
  for (uint i = 0; i < mpMorphoContextList.size(); ++i)
  {
    mpMorphoContextList.at(i)->SetEnv(env);
  }
}

ESldError CDictionaryContext::Close()
{
  for (uint i = 0; i < mpMorphoContextList.size(); ++i)
  {
    mpMorphoContextList.at(i)->getMorphoData()->Close();
  }

  return mpDictionary->Close();
}


