#include "search_server/document.h"

Document::Document() = default;

Document::Document(const int id, const double relevance, const int rating)
    : Id(id)
    , Relevance(relevance)
    , Rating(rating)
{}

std::ostream& operator<<(std::ostream& os, const Document& doc) {
    os << "{ " << "document_id = " << doc.Id << ", "
       << "relevance = " << doc.Relevance << ", "
       << "rating = " << doc.Rating << " }";
    return os;
}