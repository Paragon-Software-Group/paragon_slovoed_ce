#ifndef DICTIONARY_FUNCTION_H
#define DICTIONARY_FUNCTION_H

#include "i_native_function.h"
#include "native_function.h"
#include "SldDictionary.h"

class CDictionaryFunction : public CNativeFunction<CSldDictionary*>, public INativeFunction<jbyteArray>
{
  public:
    virtual jobject call( JNIEnv* env, jbyteArray array, jobjectArray args ) const noexcept;
};

#endif //DICTIONARY_FUNCTION_H
