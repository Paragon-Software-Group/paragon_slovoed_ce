#pragma once
#ifndef _C_SLD_CSS_DATA_MANAGER_H_
#define _C_SLD_CSS_DATA_MANAGER_H_

#include "SldDefines.h"
#include "SldPlatform.h"
#include "SldTypeDefs.h"
#include "SldSDCReadMy.h"

// forward decls
struct TSldCSSProperty;
struct TSldCSSPropertyBlock;
class CSldDictionary;
class CSldStringStore;

/// Структура - резолвер ссылок на внутренние ресурсы
class CSldCSSUrlResolver
{
public:
	virtual ~CSldCSSUrlResolver() {}

	/**
	 * Должен резолвить ссылки на "внутренние" картинки
	 *
	 * @param [in] aIndex  - индекс картинки
	 *
	 * Получив пару индекс картинки должен вернуть путь по которому
	 * можно обратиться [картинке] к ней через css `url()` значение.
	 * При невозможности создать валидный путь (вне зависимости от причины)
	 * обязан вернуть пустую строку.
	 *
	 * ВАЖНО (немного особенностей реализации):
	 *  Концептульно возвращенная строка добавляется в список css свойств как
	 *    printf("url('%s')", <string>);
	 *  где <string> - возвращенное значение "как есть".
	 *  т.е. символ `'` в возвращенной строке должны быть уже экранирован
	 *
	 * @return строка с путем к картинке (или пустая строка при ошибке)
	 */
	virtual SldU16String ResolveImageUrl(UInt32 aIndex) = 0;
};

class CSldCSSDataManager
{
public:
	CSldCSSDataManager(CSldDictionary &aDict, CSDCReadMy &aReader,
					   const CSDCReadMy::Resource &aHeaderResource, ESldError &aError);

	/**
	 *  Выдает строку css параметров по глобальному индексу блока
	 *
	 *  @param [in] aIndex     - индекс блока css параметров
	 *  @param [in] aString    - указатель куда будет записана строка параметров
	 *  @param [in] aResolver  - указатель на объект "резолвящий" ссылки на внутренние ресурсы в url'ы
	 *
	 *  @return код ошибки
	 *  ВАЖНО:
	 *   необходимо проверять длину полученой строки
	 *   функция может вернуть eOK и при этом выдать пустую строку, это "нормально"
	 */
	ESldError GetCSSStyleString(UInt32 aIndex, SldU16String *aString, CSldCSSUrlResolver *aResolver);

private:

	// loads a css property block
	ESldError LoadCSSPropertyBlock(UInt32 aIndex, const TSldCSSPropertyBlock **aBlock);

	// loads a single css property
	ESldError LoadCSSProperty(UInt32 aIndex, const TSldCSSProperty **aProperty);

	// backref to the dictionary for strings
	CSldDictionary &m_dictionary;

	//
	CSDCReadMy &m_reader;

	// cached data from the header
	UInt32 m_resourceHeaderSize;
	UInt32 m_propsIndexBits;

	// css resources
	CSDCReadMy::Resource m_blocksResource;
	CSDCReadMy::Resource m_propsResource;

	// resource descriptors
	sld2::DynArray<TSldCSSResourceDescriptor> m_blocksDescriptors;
	sld2::DynArray<TSldCSSResourceDescriptor> m_propsDescriptors;

	// string store
	sld2::UniquePtr<CSldStringStore> m_strings;
};

#endif // _C_SLD_CSS_DATA_MANAGER_H_
