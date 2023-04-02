#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server)  // + Переработан
{
    std::set<int> erase_id;
    std::set<std::set<std::string>> words_sets;
    std::set<std::string> set_words;

    for (const int document_id : search_server)
    {
        for (const auto& [word, frequency] : search_server.GetWordFrequencies(document_id))
        {
            set_words.insert(word);
        }
        if (words_sets.find(set_words) == words_sets.end())
        {
            words_sets.insert(set_words);
        }
        else
        {
            erase_id.insert(document_id);
        }
        set_words.clear();
    }
    for (const auto& erase : erase_id)
    {
        std::cout << "Found duplicate document id " << erase << std::endl;
        search_server.RemoveDocument(erase);
    }
}