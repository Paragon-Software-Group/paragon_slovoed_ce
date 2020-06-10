#include "open_sdc_via_descriptor.h"

#include "context/dictionary_context.h"
#include "context/builder/LayerAccess.h"
#include "context/reader/descriptor_reader.h"
#include "java/java_objects.h"

#include "SldDictionary.h"

jobject
OpenSdcViaDescriptor::OpenSdcHelper( JNIEnv* env, EnvAwareSdcReader* pFileReader, jbyteArray dictionaryContextPointer, jobject _cacheDir  )
{
  CSldDictionary* pDictionary = new CSldDictionary;
  JavaObjects::string cacheDir = JavaObjects::GetString( env, _cacheDir );
  LayerAccess* pLayerAccess = new LayerAccess(cacheDir);
  CDictionaryContext* pDictionaryContext = new CDictionaryContext( pDictionary, pFileReader, pLayerAccess );
  pDictionaryContext->SetEnv( env );
  ESldError error = pDictionary->Open( pFileReader, pLayerAccess );
  if ( error == eOK )
  {
    env->SetByteArrayRegion( dictionaryContextPointer, 0, sizeof( pDictionaryContext ), (jbyte*) &pDictionaryContext );
  }
  else
  {
    delete pDictionaryContext;
  }
  return JavaObjects::GetInteger( env, error );
}

jobject OpenSdcViaDescriptor::native(
    JNIEnv* env,
    jbyteArray array,
    jobject descriptor,
    jobject offset,
    jobject size,
    jobject cacheDir
) const noexcept
{
  EnvAwareSdcReader* descriptorReader = new DescriptorReader(
      JavaObjects::GetInteger( env, descriptor ),
      JavaObjects::GetLong( env, offset ),
      JavaObjects::GetLong( env, size )
  );
  return OpenSdcHelper( env, descriptorReader, array, cacheDir );
}
