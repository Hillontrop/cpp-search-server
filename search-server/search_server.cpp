#include "search_server.h"


SearchServer::SearchServer(const std::string& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) // 2
{
    if (document_id < 0)
    {
        throw std::invalid_argument("Negative_id");
    }
    if (documents_.count(document_id) != 0)
    {
        throw std::invalid_argument("id already exists");
    }
    const std::vector<std::string> words = SplitIntoWordsNoStop(document);
    const double t_f = 1.0 / static_cast<double>(words.size());

    for (const std::string& word : words)
    {
        document_word_frequency[document_id][word] += t_f;
        word_to_document_freqs_[word][document_id] += t_f;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    order_of_adding_documents.insert(document_id);       // + Переработано
}

void SearchServer::RemoveDocument(int document_id)
{
    if (find(SearchServer::begin(), SearchServer::end(), document_id) != SearchServer::end())
    {
        for (const auto& [word, freqs] : document_word_frequency.at(document_id))   // + Добавлено удаление из контейнера слова которого нет в документах 
        {
            word_to_document_freqs_.at(word).size() == 1 ? word_to_document_freqs_.erase(word) : word_to_document_freqs_.at(word).erase(document_id);
        }
        order_of_adding_documents.erase(document_id);   // + Изменено удаление
        document_word_frequency.erase(document_id);
        documents_.erase(document_id);
    }
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status_filter) const
{
    return SearchServer::FindTopDocuments(raw_query, [&status_filter](int document_id, DocumentStatus status, int rating) { return status == status_filter; });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const
{
    return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const
{
    return static_cast <int> (documents_.size());
}

std::set<int>::const_iterator SearchServer::begin()     // + Переработано
{
    return order_of_adding_documents.cbegin();
}

std::set<int>::const_iterator SearchServer::end()       // + Переработано
{
    return order_of_adding_documents.cend();
}

static const std::map<std::string, double> NO_DOCUMENT;     // + Добавлена статическая переменная
const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    return document_word_frequency.at(document_id).empty() ? NO_DOCUMENT : document_word_frequency.at(document_id);     // + Переработано
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const
{
    std::vector<std::string> words;

    auto query_words = SearchServer::ParseQuery(raw_query);

    for (const std::string& word : query_words.words_plus)
    {
        if (word_to_document_freqs_.count(word) != 0)
        {
            if (word_to_document_freqs_.at(word).count(document_id))
            {
                words.push_back(word);
            }
        }
    }
    for (const std::string& word : query_words.words_minus)
    {
        if (word_to_document_freqs_.count(word) != 0)
        {
            if (word_to_document_freqs_.at(word).count(document_id))
            {
                words.clear();
                break;
            }
        }
    }
    sort(words.begin(), words.end(), [](const std::string& str_1, const std::string& str_2) {return str_1 < str_2;});
    return std::tuple(words, documents_.at(document_id).status);
}

bool SearchServer::IsStopWord(const std::string& word) const
{
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const
{
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text))
    {
        if (!SearchServer::IsValidWord(word))
        {
            throw std::invalid_argument("invalid symbols in word -> " + word);
        }
        if (!SearchServer::IsStopWord(word))
        {
            words.push_back(word);
        }
    }
    return words;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string word) const
{
    if (!SearchServer::IsValidWord(word))
    {
        throw std::invalid_argument("invalid characters in query word -> " + word);
    }
    bool is_minus = false;

    if (word.at(0) == '-')
    {
        word = word.substr(1);
        if (word.empty())
        {
            throw std::invalid_argument("no text after - in the query");
        }
        if (word[0] == '-')
        {
            throw std::invalid_argument("more than one minus before minus-word in the query");
        }
        is_minus = true;
    }
    const SearchServer::QueryWord query_word = { word, is_minus, SearchServer::IsStopWord(word) };
    return query_word;
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const
{
    SearchServer::Query word_plus_minus;
    for (const std::string& word : SplitIntoWords(text))
    {
        const auto query_word = SearchServer::ParseQueryWord(word);
        if (!query_word.is_stop)
        {
            if (query_word.is_minus)
            {
                word_plus_minus.words_minus.insert(query_word.data);
            }
            else
            {
                word_plus_minus.words_plus.insert(query_word.data);
            }
        }
    }
    return word_plus_minus;
}

double SearchServer::InverseDocumentFrequency(const std::string& word) const
{
    return log(static_cast<double>(documents_.size()) / static_cast <double> (word_to_document_freqs_.at(word).size()));
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings)
{
    if (ratings.empty())
    {
        return 0;
    }
    int number_of_ratings = static_cast<int> (ratings.size());
    return (accumulate(ratings.begin(), ratings.end(), 0) / number_of_ratings);
}

bool SearchServer::IsValidWord(const std::string& word)
{
    return none_of(word.begin(), word.end(), [](char c) { return c >= '\0' && c < ' '; });
}