#include <execution>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <string_view>

#include "document.h"
#include "log_duration.h"
#include "paginator.h"
#include "process_queries.h"
#include "request_queue.h"
#include "search_server.h"
#include "test_example_functions.h"

using namespace std;


void RemoveDuplicates(SearchServer& search_server)
{
    std::set<int> erase_id;
    std::set<std::set<std::string_view>> words_sets;
    std::set<std::string_view> set_words;

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

void PrintDocument(const Document& document)
{
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}

int main()
{
    setlocale(LC_ALL, "ru");

    SearchServer search_server("and with"s);
    int id = 0;
    for (const string& text :
                                {
                                    "white cat and yellow hat"s,
                                    "curly cat curly tail"s,
                                    "nasty dog with big eyes"s,
                                    "nasty pigeon john"s,
                                })
    {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
    }
    cout << "ACTUAL by default:"s << endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s))
    {
        PrintDocument(document);
    }
    cout << "BANNED:"s << endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED))
    {
        PrintDocument(document);
    }
    cout << "Even ids:"s << endl;
    // параллельная версия
    for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; }))
    {
        PrintDocument(document);
    }
    return 0;
} 

// ACTUAL by default:
// { document_id = 2, relevance = 0.866434, rating = 1 }
// { document_id = 4, relevance = 0.231049, rating = 1 }
// { document_id = 1, relevance = 0.173287, rating = 1 }
// { document_id = 3, relevance = 0.173287, rating = 1 }
// BANNED:
// Even ids :
// { document_id = 2, relevance = 0.866434, rating = 1 }
// { document_id = 4, relevance = 0.231049, rating = 1 }