#ifndef CLOSE_SDC_H
#define CLOSE_SDC_H

#include "../byte_array_function.h"

class CloseSdc : public CByteArrayFunction
{
  protected:
    jobject native( JNIEnv* env, jbyteArray array ) const noexcept override;
};

#endif //CLOSE_SDC_H
