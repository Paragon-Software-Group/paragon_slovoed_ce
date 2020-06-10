#ifndef _SLD_PHRASE_UTILITY_H_
#define _SLD_PHRASE_UTILITY_H_

#include "SldPlatform.h"
#include "SldString.h"
#include "SldUtil.h"

struct TPhraseSubrange
{
  UInt32 m_Begin = -1;
  UInt32 m_Size = 0;
};

inline bool operator==(const TPhraseSubrange& lhs, const TPhraseSubrange& rhs)
{
  return lhs.m_Begin == rhs.m_Begin && 
         lhs.m_Size  == rhs.m_Size;
}

class CSldSubphraseSearchInstance
{
public:
  virtual Int8 TryForBestMatch(
    const SldU16String& aPhrase,
    const SldU16String& aSubphrase,
    TPhraseSubrange& aRange) = 0;
};

class CSldSubrangePullback
{
public:
  virtual ~CSldSubrangePullback() {}

  virtual TPhraseSubrange Act(const TPhraseSubrange&) const = 0;
};

struct TControlledTransformResult
{
  SldU16String m_String;
  sld2::UniquePtr<CSldSubrangePullback> m_RangePullback;
};

class CSldControlledTransform
{
public:
  using Result = TControlledTransformResult;

  virtual ~CSldControlledTransform() {}
  virtual Result Act(const SldU16String& aFrom) const = 0;
};

class CSldControlledTransformGenerator
{
public:
  using Result = TControlledTransformResult;

  virtual ~CSldControlledTransformGenerator() {}

  virtual Int8 Init(const SldU16String& aPhrase) = 0;
  virtual Result Transform() = 0;
  virtual Int8 End() const = 0;
};

struct TFindBestSubphraseResult
{
  TPhraseSubrange m_BestRange;
  TControlledTransformResult m_TransformResult;
};

TFindBestSubphraseResult FindBestSubphrase(
  const SldU16String& aSource, const SldU16String& aSubstring, 
  CSldControlledTransformGenerator& aTransformGenerator, 
  CSldSubphraseSearchInstance& aSearchInstance);

#endif //_SLD_PHRASE_UTILITY_H_
