//
// Created by kovtunov on 27.11.18.
//

#include "add_morpho_via_java_reader.h"
#include "add_morpho_via_descriptor.h"
#include "context/reader/java_reader.h"

jobject
AddMorphoViaJavaReader::native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject isOpenedCallback,
        jobject readCallback,
        jobject getSizeCallback
) const noexcept
{
    EnvAwareSdcReader* inputStreamReader = new JavaReader(
            env, isOpenedCallback, readCallback, getSizeCallback
    );
    return AddMorphoViaDescriptor::AddMorphoHelper( env, inputStreamReader, context );
}