#include "remove_duplicates.h"

bool Duplicates(std::vector<std::string> first, std::vector<std::string> second)
{
    return first == second;
}

void RemoveDuplicates(SearchServer& search_server)
{
    std::set<int> erase_id;

    for (const int document_id_begin : search_server)
    {
        for (const int document_id_end : search_server)
        {
            if (document_id_begin >= document_id_end)
            {
                continue;
            }

            if (search_server.GetWordFrequencies(document_id_begin).size() == search_server.GetWordFrequencies(document_id_end).size())
            {
                std::vector<std::string> first;
                first.reserve(search_server.GetWordFrequencies(document_id_begin).size());
                for (const auto& [word, frequency] : search_server.GetWordFrequencies(document_id_begin))
                {
                    first.push_back(word);
                }
                std::vector<std::string> second;
                second.reserve(search_server.GetWordFrequencies(document_id_end).size());
                for (const auto& [word, frequency] : search_server.GetWordFrequencies(document_id_end))
                {
                    second.push_back(word);
                }
                if (Duplicates(first, second))
                {
                    erase_id.insert(document_id_end);
                }
            }
        }
    }
    for (const auto& erase : erase_id)
    {
        std::cout << "Found duplicate document id " << erase << std::endl;
        search_server.RemoveDocument(erase);
    }
}