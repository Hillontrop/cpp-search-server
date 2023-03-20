#include "request_queue.h"


RequestQueue::RequestQueue(const SearchServer& search_server) : search_server_(search_server)
{
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status)
{
    return RequestQueue::AddFindRequest(raw_query, [&status](int document_id, DocumentStatus status_, int rating) { return status_ == status; });
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query)
{
    return RequestQueue::AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const
{
    int rezult = 0;
    for (const auto& [raw_query, empty] : requests_)
    {
        if (empty)
        {
            ++rezult;
        }
    }
    return rezult;
}

bool RequestQueue::IsEmpty(const std::vector<Document>& documents)
{
    return documents.empty();
}