#include "output_statements.h"

std::ostream& operator<<(std::ostream& os, const Document& doc)
{
    os << "{ " << "document_id = " << doc.id << ", "
        << "relevance = " << doc.relevance << ", "
        << "rating = " << doc.rating << " }";
    return os;
}