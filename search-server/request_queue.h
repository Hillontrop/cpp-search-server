#pragma once

#include <deque>
#include <string>
#include <vector>

#include "document.h"
#include "search_server.h"

class RequestQueue
{
public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate)
    {
        requests_.push_back({ raw_query , IsEmpty(search_server_.FindTopDocuments(raw_query,document_predicate)) });

        if (static_cast<size_t>(requests_.size()) > min_in_day_)
        {
            requests_.pop_front();
        }
        return search_server_.FindTopDocuments(raw_query, document_predicate);
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;

private:
    struct QueryResult
    {
        std::string raw_query;
        bool empty;
    };

    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    bool IsEmpty(const std::vector<Document>& documents);
    const SearchServer& search_server_;
};