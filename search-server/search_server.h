#pragma once

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

#include "concurrent_map.h"
#include "document.h"
#include "read_input_functions.h"
#include "string_processing.h"


const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;   // Глобальная константа для сравнение чисел с плавающей точкой 

class SearchServer
{
public:
    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words);

    explicit SearchServer(const std::string& stop_words_text);

    explicit SearchServer(const std::string_view& stop_words_text);

    void AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings);

    void RemoveDocument(int document_id);
    void RemoveDocument(std::execution::sequenced_policy seq, int document_id);
    void RemoveDocument(std::execution::parallel_policy par, int document_id);

    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query, DocumentPredicate document_predicate) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentStatus status_filter) const;
    std::vector<Document> FindTopDocuments(const std::string_view& raw_query) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query, DocumentStatus status_filter) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query) const;

    int GetDocumentCount() const;

    std::set<int>::const_iterator begin();

    std::set<int>::const_iterator end();

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view& raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::execution::sequenced_policy seq, const std::string_view& raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::execution::parallel_policy par, const std::string_view& raw_query, int document_id) const;

private:
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;   // <Слово, <Id документов, Частота слов в документе>>
    std::set<std::string, std::less<>> stop_words_ = {};
    std::list<std::pair<int,std::string>> text_documents = {};
    std::map<int, DocumentData> documents_;      // <Id документов, (Рейтинг, Статус)>
    std::set<int> order_of_adding_documents;     // Id документов в порядке их добавления
    std::map<int, std::map<std::string_view, double>> document_word_frequency;   // <Id документов, <Слова в документе, Частота слов в документе>>

    bool IsStopWord(const std::string_view& word) const;

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view& text) const;

    struct QueryWord
    {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string_view word) const;

    struct Query
    {
        std::vector<std::string_view> words_plus;
        std::vector<std::string_view> words_minus;
    };

    Query ParseQuery(const std::string_view& text) const;

    Query ParseQuery(std::execution::parallel_policy par, const std::string_view& text) const;

    double InverseDocumentFrequency(const std::string_view& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::sequenced_policy seq, const Query& query, DocumentPredicate document_predicate) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::parallel_policy par, const Query& query, DocumentPredicate document_predicate) const;

    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const ExecutionPolicy& policy, const Query& query, DocumentPredicate document_predicate) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    static bool IsValidWord(const std::string_view& word);
};

template <typename StringCollection>
SearchServer::SearchServer(const StringCollection& stop_words)
{
    for (const std::string_view& word : stop_words)
    {
        if (!SearchServer::IsValidWord(word))
        {
            throw std::invalid_argument("invalid symbols in stop-word -> " + std::string(word));
        }
        if (stop_words_.count(word) == 0)
        {
            stop_words_.insert(std::string(word));
        }
    }
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query, DocumentPredicate document_predicate) const
{
    const auto query = SearchServer::ParseQuery(raw_query);
    auto matched_documents = SearchServer::FindAllDocuments(policy, query, document_predicate);

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

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const
{
    return SearchServer::FindTopDocuments(std::execution::seq, raw_query, document_predicate);
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query, DocumentStatus status_filter) const
{
    return SearchServer::FindTopDocuments(policy, raw_query, [&status_filter](int document_id, DocumentStatus status, int rating) { return status == status_filter; });
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query) const
{
    return SearchServer::FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(std::execution::sequenced_policy seq, const SearchServer::Query& query, DocumentPredicate document_predicate) const
{
    std::map<int, double> document_to_relevance;
    for (const std::string_view& word : query.words_plus)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }
        const double i_d_f = SearchServer::InverseDocumentFrequency(word);
        for (const auto [id, t_f] : word_to_document_freqs_.at(word))
        {
            const auto& document_data = documents_.at(id);
            if (document_predicate(id, document_data.status, document_data.rating))
            {
                document_to_relevance[id] += t_f * i_d_f;
            }
        }
    }
    for (const std::string_view& word : query.words_minus)
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

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(std::execution::parallel_policy par, const SearchServer::Query& query, DocumentPredicate document_predicate) const
{
    ConcurrentMap<int, double> document_to_relevance(4);

    for_each(par, query.words_plus.begin(), query.words_plus.end(), [this, &document_predicate, &document_to_relevance](const std::string_view& word)
        {
            if (word_to_document_freqs_.count(word) != 0)
            {
                const double i_d_f = SearchServer::InverseDocumentFrequency(word);
                for (const auto [id, t_f] : word_to_document_freqs_.at(word))
                {
                    const auto& document_data = documents_.at(id);
                    if (document_predicate(id, document_data.status, document_data.rating))
                    {
                        document_to_relevance[id].ref_to_value += t_f * i_d_f;
                    }
                }
            }
        });

    for_each(par, query.words_minus.begin(), query.words_minus.end(), [this, &document_to_relevance](const std::string_view& word)
        {
            if (word_to_document_freqs_.count(word) != 0)
            {
                for (const auto [document_id, _] : word_to_document_freqs_.at(word))
                {
                    document_to_relevance.erase(document_id);
                }
            }
        });
    document_to_relevance.BuildOrdinaryMap();
    std::vector<Document> matched_documents;
    for (const auto& document : document_to_relevance)
    {
        for (const auto& [document_id, relevance] : document.map)
        {
            matched_documents.push_back( { document_id, relevance, documents_.at(document_id).rating });
        }
    }
    return matched_documents;
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const ExecutionPolicy& policy, const SearchServer::Query& query, DocumentPredicate document_predicate) const
{
    return SearchServer::FindAllDocuments(policy, query, document_predicate);
}