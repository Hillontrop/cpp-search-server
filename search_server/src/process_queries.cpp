#include "search_server/process_queries.h"


std::vector<std::vector<Document>> ProcessQueries(const SearchServer& searchServer, const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> documentsLists(queries.size());
    std::transform(
        std::execution::par,
        queries.begin(),
        queries.end(),
        documentsLists.begin(),
        [&searchServer](const std::string query) {
            return searchServer.FindTopDocuments(query);
        }
    );
    return documentsLists;
}

std::list<Document> ProcessQueriesJoined(const SearchServer& searchServer, const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> documentsLists = ProcessQueries(searchServer, queries);
    std::list<Document> documentChain;
    for (const auto doc : documentsLists) {
        documentChain.insert(documentChain.end(), doc.begin(), doc.end());
    }
    return documentChain;
}