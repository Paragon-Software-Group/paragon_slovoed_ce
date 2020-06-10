#include "SldPhraseUtility.h"

TFindBestSubphraseResult FindBestSubphrase(
    const SldU16String& aSource, const SldU16String& aSubstring, 
    CSldControlledTransformGenerator& aTransformGenerator, 
    CSldSubphraseSearchInstance& aSearchInstance)
{
  TFindBestSubphraseResult result;
  aTransformGenerator.Init(aSource);
  while (!aTransformGenerator.End())
  {
    TControlledTransformResult transformResult = aTransformGenerator.Transform();
    if (!transformResult.m_RangePullback)
      continue;

    TPhraseSubrange candidate;
    if (aSearchInstance.TryForBestMatch(transformResult.m_String, aSubstring, candidate))
    {
      result.m_BestRange = transformResult.m_RangePullback->Act(candidate);
      result.m_TransformResult = sld2::move(transformResult);
    }
  }

  return result;
}
