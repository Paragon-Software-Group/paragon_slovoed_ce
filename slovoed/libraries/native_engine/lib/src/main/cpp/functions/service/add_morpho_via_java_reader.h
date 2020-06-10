//
// Created by kovtunov on 27.11.18.
//

#ifndef SLOVOED_ADD_MORPHO_VIA_JAVA_READER_H
#define SLOVOED_ADD_MORPHO_VIA_JAVA_READER_H

#include "../dictionary_context_function.h"
#include "../../context/env_aware.h"

class AddMorphoViaJavaReader : public CDictionaryContextFunction
{
    protected:
        jobject
        native(
                JNIEnv* env,
                CDictionaryContext* context,
                jobject isOpenedCallback,
                jobject readCallback,
                jobject getSizeCallback ) const noexcept override;
};


#endif //SLOVOED_ADD_MORPHO_VIA_JAVA_READER_H
