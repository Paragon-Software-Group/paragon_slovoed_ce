#include "Article.h"

#include "XmlTagContent.h"

/**
	”станавливает идентификатор статьи

	@param text - строчка с индентификатором
*/
void CArticle::setID(const wstring &text)
{
	m_id = text;
}

/**
	¬озвращает количество блоков в статье

	@return	количество блоков в статье
*/
UInt32 CArticle::GetBlockCount(void) const
{
	return (UInt32)m_blocks.size();
}

/**
	¬озвращает блок данных по указанному номеру
	
	@param aNumber	- номер блока

	@return	- указатель на блок данных
			- NULL, если такой блок отсутствует
*/
BlockType* CArticle::GetBlock(UInt32 aNumber)
{
	if (aNumber >= GetBlockCount())
		return NULL;
	
	return m_blocks[aNumber];
}

/**
 * Deletes a block by it's pointer
 *
 * For "normal" text blocks simply removes the blocks from the article
 * For "metadata" blocks tries to find the corresponding closing block, and deletes both
 * of them at once if it finds one.
 *
 * Some usage notes:
 * - the passed pointer *is invalid* (stale) after the successfull removal; just like
 *   any pointer to the corresponding closing block in the case of metadata blocks
 * - for metadata blocks: the removal *must* be done through the opening block
 * - for metadata blocks: the metadata reference *must* be cleared prior to calling this
 *
 * NOTE:
 *  Can't really be called from a loop that iterates over articles blocks...
 *
 * Returns if it could remove the block [in non debug builds, in debug it asserts first]
 */
bool CArticle::removeBlock(const BlockType *aBlock)
{
#define return_if_fail(_COND, _MSG) do { assert((_COND) && (_MSG)); if (!(_COND)) return false; } while(0)

	return_if_fail(!aBlock->isClosing, "Can't remove through a closing meta tag.");

	auto open = std::find(m_blocks.begin(), m_blocks.end(), aBlock);
	return_if_fail(open != m_blocks.end(), "The block does not belong to this article.");

	// erase non metadata blocks right away
	if (!isMetadataType(aBlock->metaType))
	{
		m_blocks.erase(open);
		return true;
	}

	// for metadata blocks we *have to* find the closing block
	// and we have to handle nested blocks of the same type (in general)
	auto close = std::next(open);
	unsigned depth = 1;
	for (; close != m_blocks.end(); ++close)
	{
		if ((*close)->metaType != aBlock->metaType)
			continue;

		if ((*close)->isClosing)
			depth--;
		else
			depth++;

		if (depth == 0)
			break;
	}
	return_if_fail(close != m_blocks.end(), "Could not find the closing metadata block.");

	// remove the blocks (in "reverse" order to avoid iterator invalidation)
	m_blocks.erase(close);
	m_blocks.erase(open);

	return true;
#undef return_if_fail
}

/**
ƒобавл€ет текст определенного типа

@param aTextType	- тип текста, одно из значени€ enum'a в BlockType
@param aText		- добавл€емый текст
@param aClear		- флаг того, нужно ли нам очистить буфер перед добавлением
					  false	- текст будет добавлен к содержимому буфера
					  true	- буфер будет очищен
*/
void BlockType::AddText(int aTextType, sld::wstring_ref aText, bool aClear)
{
	if (sld2_unlikely(!metadata_)) return;
	const UInt8 index = metadata_->textIndex(aTextType);
	if (index != 0xff)
	{
		if (aClear)
			metadata_->stringData[index].assign(aText.data(), aText.size());
		else
			metadata_->stringData[index].append(aText.data(), aText.size());
	}
	else
	{
		metadata_->stringIndexes[aTextType] = static_cast<UInt8>(metadata_->stringData.size());
		metadata_->stringData.push_back(to_string(aText));
	}
}
