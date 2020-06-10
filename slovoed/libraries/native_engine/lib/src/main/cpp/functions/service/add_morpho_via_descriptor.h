//
// Created by kovtunov on 20.11.18.
//

#ifndef SLOVOED_ADD_MORPHO_VIA_DESCRIPTOR_H
#define SLOVOED_ADD_MORPHO_VIA_DESCRIPTOR_H


#include "../dictionary_context_function.h"
#include "../../context/env_aware.h"

class AddMorphoViaDescriptor  : public CDictionaryContextFunction
{
public:
    static jobject AddMorphoHelper(
            JNIEnv* env,
            EnvAwareSdcReader* pFileReader,
            CDictionaryContext* context
    );

protected:
    jobject
    native(
            JNIEnv* env,
            CDictionaryContext* context,
            jobject descriptor,
            jobject offset,
            jobject size
    ) const noexcept override;
};


#endif
