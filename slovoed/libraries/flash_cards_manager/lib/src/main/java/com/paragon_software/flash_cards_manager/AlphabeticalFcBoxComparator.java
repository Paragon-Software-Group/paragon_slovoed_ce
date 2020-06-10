package com.paragon_software.flash_cards_manager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;

import java.util.Comparator;

class AlphabeticalFcBoxComparator implements Comparator<FlashcardBox> {
    @Override
    public int compare(FlashcardBox o1, FlashcardBox o2) {
        ArticleItem item1 = o1.getParent(), item2 = o2.getParent();
        Dictionary.DictionaryId dictionaryId1 = item1.getDictId(), dictionaryId2 = item2.getDictId();
        int res;
        if(!dictionaryId1.equals(dictionaryId2))
            res = dictionaryId1.toString().compareTo(dictionaryId2.toString());
        else {
            int listId1 = item1.getListId(), listId2 = item2.getListId();
            if(listId1 != listId2)
                res = listId1 - listId2;
            else
                res = item1.getGlobalIndex() - item2.getGlobalIndex();
        }
        return res;
    }
}
