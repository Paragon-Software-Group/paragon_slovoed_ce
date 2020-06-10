//
// Created by borbat on 01.10.2018.
//

#include <SldHTMLBuilder.h>
#include "Translate.h"
#include "java/java_objects.h"

jobject
Translate::native( JNIEnv* env, CDictionaryContext* context, jobject _listIndex, jobject _localIndex, jobject _htmlParams) const noexcept
{
  jint listIndex = JavaObjects::GetInteger( env, _listIndex );
  jint localIndex = JavaObjects::GetInteger( env, _localIndex );
  static UInt16 EmptyString[1] = {0u};
  CSldDictionary* dictionary = context->GetDictionary();
  LayerAccess* layerAccess = context->GetLayerAccess();
  layerAccess->InitTranslation( dictionary, env, _htmlParams);

  dictionary->TranslateClear();
  const UInt16* res = EmptyString;

   //function fixed in v.133.124
   //https://jira-shdd.paragon-software.com/browse/NSA-478
   ESldError error = dictionary->Translate( listIndex, localIndex, 0, eTranslationFull );

//  ESldError error = dictionary->SetCurrentWordlist( listIndex );
//  if ( error == eOK )
//  {
//    error = dictionary->Translate( localIndex, 0, eTranslationFull );
//  }

  if ( error == eOK )
  {
    res = layerAccess->GetTranslationHtml();
  }
  return JavaObjects::GetString( env, res );
}
