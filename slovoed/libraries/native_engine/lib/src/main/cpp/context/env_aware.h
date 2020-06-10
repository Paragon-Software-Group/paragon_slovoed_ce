#ifndef ENV_AWARE_H
#define ENV_AWARE_H

#include <jni.h>
#include "ISDCFile.h"
#include "ISldLayerAccess.h"

class IEnvAware
{
  public:
    virtual void SetEnv( JNIEnv* env ) = 0;
};

class CEnvAware : public IEnvAware
{
  private:
    JNIEnv* mEnv;

  public:
    void SetEnv( JNIEnv* env ) override;

  protected:
    JNIEnv* GetEnv( void ) const;
};

class EnvAwareSdcReader : public CEnvAware, public ISDCFile
{

};

//class EnvAwareLayerAccess : public CEnvAware, public ISldLayerAccess
//{
//
//};

#endif //ENV_AWARE_H
