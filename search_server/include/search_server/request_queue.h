#pragma once

#include "document.h"
#include "search_server.h"

#include <deque>
#include <string>
#include <vector>



class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& searchServer);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& rawQuery, DocumentPredicate documentPredicate);

    std::vector<Document> AddFindRequest(const std::string& rawQuery, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& rawQuery);
    int GetNoResultRequests() const;

private:
    bool IsEmpty(const std::vector<Document>& documents);

    struct QueryResult {
        std::string RawQuery;
        bool Empty;
    };

    std::deque<QueryResult> m_requests;
    const static int m_minInDay = 1440;
    const SearchServer& m_searchServer;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& rawQuery, DocumentPredicate documentPredicate) {
    m_requests.push_back({ rawQuery , RequestQueue::IsEmpty(m_searchServer.FindTopDocuments(rawQuery,documentPredicate)) });

    if (static_cast<size_t>(m_requests.size()) > m_minInDay) {
        m_requests.pop_front();
    }
    return m_searchServer.FindTopDocuments(rawQuery, documentPredicate);
}