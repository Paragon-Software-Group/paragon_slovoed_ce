#ifndef DICTIONARY_CONTEXT_FUNCTION_H
#define DICTIONARY_CONTEXT_FUNCTION_H

#include "i_native_function.h"
#include "native_function.h"
#include "../context/dictionary_context.h"

class CDictionaryContextFunction : CNativeFunction< CDictionaryContext* >, public INativeFunction< jbyteArray >
{
  public:
    static CDictionaryContext* LoadContext( JNIEnv* env, jbyteArray array );

    jobject call( JNIEnv* env, jbyteArray array, jobjectArray args ) const noexcept override;
};

#endif //DICTIONARY_CONTEXT_FUNCTION_H
