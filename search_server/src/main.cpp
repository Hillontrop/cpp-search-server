#include <execution>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <string_view>

#include "search_server/document.h"
#include "search_server/log_duration.h"
#include "search_server/paginator.h"
#include "search_server/process_queries.h"
#include "search_server/request_queue.h"
#include "search_server/search_server.h"
// #include <include/search_server/test_example_functions.h>

using namespace std;


void RemoveDuplicates(SearchServer& searchServer) {
    std::set<int> erase_id;
    std::set<std::set<std::string_view>> wordsSets;
    std::set<std::string_view> setWords;

    for (const int documentId : searchServer) {
        for (const auto& [word, frequency] : searchServer.GetWordFrequencies(documentId)) {
            setWords.insert(word);
        }
        
        if (wordsSets.find(setWords) == wordsSets.end()) {
            wordsSets.insert(setWords);
            setWords.clear();
            continue;
        }

        erase_id.insert(documentId);
        setWords.clear();
    }

    for (const auto& erase : erase_id) {
        std::cout << "Found duplicate document id " << erase << std::endl;
        searchServer.RemoveDocument(erase);
    }
}

void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.Id << ", "s
        << "relevance = "s << document.Relevance << ", "s
        << "rating = "s << document.Rating << " }"s << endl;
}

int main() {
    setlocale(LC_ALL, "ru");

    SearchServer searchServer("and with"s);
    int id = 0;
    for (const string& text :  {
            "white cat and yellow hat"s,
            "curly cat curly tail"s,
            "nasty dog with big eyes"s,
            "nasty pigeon john"s,
        })
    {
        searchServer.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
    }
    cout << "ACTUAL by default:"s << endl;

    for (const Document& document : searchServer.FindTopDocuments("curly nasty cat"s)) {
        PrintDocument(document);
    }

    cout << "BANNED:"s << endl;

    for (const Document& document : searchServer.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }
    cout << "Even ids:"s << endl;

    for (const Document& document
        : searchServer.FindTopDocuments(
            execution::par,
            "curly nasty cat"s,
            [](int document_id, DocumentStatus status, int rating) {
                return document_id % 2 == 0;
            }
        ))
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