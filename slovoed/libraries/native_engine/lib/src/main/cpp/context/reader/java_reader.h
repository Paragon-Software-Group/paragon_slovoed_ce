#ifndef JAVA_READER_H
#define JAVA_READER_H

#include <jni.h>
#include "../env_aware.h"
#include "java/java_objects.h"

class JavaReader final : public EnvAwareSdcReader
{
  private:
    jobject mIsOpenedCallback, mReadCallback, mGetSizeCallback;

  public:
    JavaReader(
        JNIEnv* env,
        jobject isOpenedCallback, jobject readCallback, jobject getSizeCallback
    );

    ~JavaReader();

    Int8 IsOpened() const override;

    UInt32 Read( void* aDestPtr, UInt32 aSize, UInt32 aOffset ) override;

    UInt32 GetSize() const override;
};

#endif //JAVA_READER_H
