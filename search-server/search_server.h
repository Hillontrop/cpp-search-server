#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "const.h"
#include "document.h"
#include "string_processing.h"

class SearchServer
{
public:
    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words)
    {
        for (const auto& word : stop_words)
        {
            if (!IsValidWord(word))
            {
                throw std::invalid_argument("invalid symbols in stop-word -> " + word);
            }
            if (stop_words_.count(word) == 0)
            {
                stop_words_.insert(word);
            }
        }
    }
    explicit SearchServer(const std::string& stop_words_text);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const
    {
        const auto query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs)
            {
                if (std::abs(lhs.relevance - rhs.relevance) < EPSILON)
                {
                    return lhs.rating > rhs.rating;
                }
                else
                {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status_filter) const;
    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    int GetDocumentCount() const;
    int GetDocumentId(int index) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

private:

    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::set<std::string> stop_words_;
    std::map<int, DocumentData> documents_;
    std::vector<int> order_of_adding_documents;

    bool IsStopWord(const std::string& word) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    struct QueryWord
    {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string word) const;

    struct Query
    {
        std::set<std::string> words_plus;
        std::set<std::string> words_minus;
    };

    Query ParseQuery(const std::string& text) const;

    double InverseDocumentFrequency(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const
    {
        std::map<int, double> document_to_relevance;
        for (const std::string& word : query.words_plus)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            const double i_d_f = InverseDocumentFrequency(word);
            for (const auto [id, t_f] : word_to_document_freqs_.at(word))
            {
                const auto& document_data = documents_.at(id);
                if (document_predicate(id, document_data.status, document_data.rating))
                {
                    document_to_relevance[id] += t_f * i_d_f;
                }
            }
        }
        for (const std::string& word : query.words_minus)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word))
            {
                document_to_relevance.erase(document_id);
            }
        }
        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance)
        {
            matched_documents.push_back(
                {
                    document_id,
                    relevance,
                    documents_.at(document_id).rating
                });
        }
        return matched_documents;
    }

    static int ComputeAverageRating(const std::vector<int>& ratings);

    static bool IsValidWord(const std::string& word);
};