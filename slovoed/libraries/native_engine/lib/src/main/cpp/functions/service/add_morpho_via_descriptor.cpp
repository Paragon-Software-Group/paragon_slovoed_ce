//
// Created by kovtunov on 20.11.18.
//


#include "add_morpho_via_descriptor.h"
#include "context/reader/descriptor_reader.h"
#include "context/dictionary_context.h"
#include "../dictionary_context_function.h"

jobject AddMorphoViaDescriptor::AddMorphoHelper(
        JNIEnv* env,
        EnvAwareSdcReader* pFileReader,
        CDictionaryContext* context
)
{
    pFileReader->SetEnv(env);
    JavaObjects::string cacheDir = JavaObjects::string();
    LayerAccess* pLayerAccess = new LayerAccess( cacheDir);
    pLayerAccess->SetEnv(env);
    MorphoData* data = new MorphoData;
    bool success = data->Init(pFileReader,pLayerAccess);
    if (success)
    {
        context->AddMorphoContext(new MorphoContext(data, pFileReader, pLayerAccess));
    }
    return JavaObjects::GetInteger( env, success );
}

jobject AddMorphoViaDescriptor::native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject descriptor,
        jobject offset,
        jobject size
) const noexcept
{
    EnvAwareSdcReader* descriptorReader = new DescriptorReader(
            JavaObjects::GetInteger( env, descriptor ),
            JavaObjects::GetLong( env, offset ),
            JavaObjects::GetLong( env, size )
    );
    return AddMorphoHelper( env, descriptorReader, context );
}