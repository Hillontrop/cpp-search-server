#include "process_queries.h"


std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries)
{
    std::vector<std::vector<Document>> documents_lists(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), documents_lists.begin(), [&search_server](const std::string query) {return search_server.FindTopDocuments(query);});
    return documents_lists;
}

std::list<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries)
{
    std::vector<std::vector<Document>> documents_lists = ProcessQueries(search_server, queries);
    std::list<Document> document_chain;
    for (const auto doc : documents_lists)
    {
        document_chain.insert(document_chain.end(), doc.begin(), doc.end());
    }
    return document_chain;
}