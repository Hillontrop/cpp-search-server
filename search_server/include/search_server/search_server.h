#pragma once

#include "concurrent_map.h"
#include "document.h"
#include "read_input_functions.h"
#include "string_processing.h"

#include <algorithm>
#include <cmath>
#include <execution>
#include <functional>
#include <map>
#include <list>
#include <numeric>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>



const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer {
public:
    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stopWords);

    explicit SearchServer(const std::string& stopWordsText);

    explicit SearchServer(const std::string_view stopWordsText);

    void AddDocument(int documentId, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

    void RemoveDocument(int documentId);
    void RemoveDocument(std::execution::sequenced_policy seq, const int documentId);
    void RemoveDocument(std::execution::parallel_policy par, const int documentId);

    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(
        const ExecutionPolicy& policy,
        const std::string_view rawQuery,
        DocumentPredicate documentPredicate) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view rawQuery, DocumentPredicate documentPredicate) const;

    std::vector<Document> FindTopDocuments(const std::string_view rawQuery, DocumentStatus statusFilter) const;
    std::vector<Document> FindTopDocuments(const std::string_view rawQuery) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, const std::string_view rawQuery, DocumentStatus statusFilter) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, const std::string_view rawQuery) const;

    inline int GetDocumentCount() const {
        return static_cast<int>(m_documents.size());
    }

    inline auto begin() {
        return m_orderOfAddingDocuments.cbegin();
    }

    inline auto end() {
        return m_orderOfAddingDocuments.cend();
    }

    const std::map<std::string_view, double>& GetWordFrequencies(const int documentId) const;
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view rawQuery, const int documentId) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(
        std::execution::sequenced_policy seq,
        const std::string_view rawQuery,
        const int documentId) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(
        std::execution::parallel_policy par,
        const std::string_view rawQuery,
        const int documentId) const;

private:
    struct DocumentData {
        int Rating;
        DocumentStatus Status;
    };

    struct QueryWord {
        std::string_view Data;
        bool IsMinus;
        bool IsStop;
    };

    struct Query {
        std::vector<std::string_view> WordsPlus;
        std::vector<std::string_view> WordsMinus;
    };

private:
    inline bool IsStopWord(const std::string_view word) const noexcept {
        return m_stopWords.count(word) > 0;
    }

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const;

    QueryWord ParseQueryWord(std::string_view word) const;

    Query ParseQuery(const std::string_view text) const;
    Query ParseQuery(std::execution::parallel_policy par, const std::string_view text) const;

    double InverseDocumentFrequency(const std::string_view word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::sequenced_policy seq, const Query& query, DocumentPredicate documentPredicate) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::parallel_policy par, const Query& query, DocumentPredicate documentPredicate) const;

    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const ExecutionPolicy& policy, const Query& query, DocumentPredicate documentPredicate) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    static bool IsValidWord(const std::string_view word);

private:
    std::map<std::string_view, std::map<int, double>> m_wordToDocumentFreqs;
    std::set<std::string, std::less<>> m_stopWords = {};
    std::list<std::pair<int,std::string>> m_textDocuments = {};
    std::map<int, DocumentData> m_documents;
    std::set<int> m_orderOfAddingDocuments;
    std::map<int, std::map<std::string_view, double>> m_documentWordFrequency;
};

template <typename StringCollection>
SearchServer::SearchServer(const StringCollection& stopWords) {
    for (const std::string_view& word : stopWords) {
        if (!SearchServer::IsValidWord(word)) {
            throw std::invalid_argument("invalid symbols in stop-word -> " + std::string(word));
        }
        if (m_stopWords.count(word) == 0) {
            m_stopWords.insert(std::string(word));
        }
    }
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(
    const ExecutionPolicy& policy,
    const std::string_view rawQuery,
    DocumentPredicate documentPredicate) const
{
    const auto query = SearchServer::ParseQuery(rawQuery);
    auto matchedDocuments = SearchServer::FindAllDocuments(policy, query, documentPredicate);

    sort(matchedDocuments.begin(), matchedDocuments.end(),
        [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.Relevance - rhs.Relevance) < EPSILON) {
                return lhs.Rating > rhs.Rating;
            }
            return lhs.Relevance > rhs.Relevance;
        }
    );

    if (matchedDocuments.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matchedDocuments.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matchedDocuments;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(
    const std::string_view rawQuery,
    DocumentPredicate documentPredicate) const
{
    return SearchServer::FindTopDocuments(std::execution::seq, rawQuery, documentPredicate);
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(
    const ExecutionPolicy& policy,
    const std::string_view rawQuery,
    DocumentStatus statusFilter) const
{
    return SearchServer::FindTopDocuments(
        policy,
        rawQuery,
        [&statusFilter](const int documentId, DocumentStatus status, const int rating) {
                return status == statusFilter;
            }
    );
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(
    const ExecutionPolicy& policy,
    const std::string_view rawQuery) const
{
    return SearchServer::FindTopDocuments(policy, rawQuery, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(
    std::execution::sequenced_policy seq,
    const SearchServer::Query& query,
    DocumentPredicate documentPredicate) const
{
    std::map<int, double> documentToRelevance;
    for (const std::string_view word : query.WordsPlus) {
        if (m_wordToDocumentFreqs.count(word) == 0){
            continue;
        }

        const double IDF = SearchServer::InverseDocumentFrequency(word);
        for (const auto& [id, TF] : m_wordToDocumentFreqs.at(word)) {
            const auto& documentData = m_documents.at(id);
            if (documentPredicate(id, documentData.Status, documentData.Rating)) {
                documentToRelevance[id] += TF * IDF;
            }
        }
    }

    for (const std::string_view word : query.WordsMinus) {
        if (m_wordToDocumentFreqs.count(word) == 0) {
            continue;
        }

        for (const auto& [documentId, _] : m_wordToDocumentFreqs.at(word)) {
            documentToRelevance.erase(documentId);
        }
    }

    std::vector<Document> matchedDocuments;

    for (const auto& [documentId, relevance] : documentToRelevance) {
        matchedDocuments.push_back(
            {
                documentId,
                relevance,
                m_documents.at(documentId).Rating
            }
        );
    }
    return matchedDocuments;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(
    std::execution::parallel_policy par,
    const SearchServer::Query& query,
    DocumentPredicate documentPredicate) const
{
    ConcurrentMap<int, double> documentToRelevance(4);

    for_each(par, query.WordsPlus.begin(), query.WordsPlus.end(), [this, &documentPredicate, &documentToRelevance](const std::string_view word)
        {
            if (m_wordToDocumentFreqs.count(word) != 0) {
                const double IDF = SearchServer::InverseDocumentFrequency(word);
                for (const auto& [id, TF] : m_wordToDocumentFreqs.at(word)) {
                    decltype(auto) documentData = m_documents.at(id);

                    if (documentPredicate(id, documentData.Status, documentData.Rating)) {
                        documentToRelevance[id].RefToValue += TF * IDF;
                    }
                }
            }
        }
    );

    for_each(par, query.WordsMinus.begin(), query.WordsMinus.end(), [this, &documentToRelevance](const std::string_view word)
        {
            if (m_wordToDocumentFreqs.count(word) != 0) {
                for (const auto& [documentId, _] : m_wordToDocumentFreqs.at(word)){
                    documentToRelevance.erase(documentId);
                }
            }
        }
    );

    documentToRelevance.BuildOrdinaryMap();

    std::vector<Document> matchedDocuments;

    for (const auto& document : documentToRelevance) {
        for (const auto& [documentId, relevance] : document.Map) {
            matchedDocuments.emplace_back(documentId, relevance, m_documents.at(documentId).Rating);
        }
    }
    return matchedDocuments;
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(
    const ExecutionPolicy& policy,
    const SearchServer::Query& query,
    DocumentPredicate document_predicate) const
{
    return SearchServer::FindAllDocuments(policy, query, document_predicate);
}