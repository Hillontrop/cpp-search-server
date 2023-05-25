#include "search_server.h"


SearchServer::SearchServer(const std::string& stop_words_text) : SearchServer(std::string_view(stop_words_text))
{
}

SearchServer::SearchServer(const std::string_view& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings) // 2
{
    if (document_id < 0)
    {
        throw std::invalid_argument("Negative_id");
    }
    if (documents_.count(document_id) != 0)
    {
        throw std::invalid_argument("id already exists");
    }
    text_documents.push_back({ document_id, std::string{ document } });

    const std::vector<std::string_view> words = SplitIntoWordsNoStop(text_documents.back().second);
    const double t_f = 1.0 / static_cast<double>(words.size());

    for (const std::string_view& word : words)
    {
        document_word_frequency[document_id][word] += t_f;
        word_to_document_freqs_[word][document_id] += t_f;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    order_of_adding_documents.insert(document_id);
}

void SearchServer::RemoveDocument(int document_id)
{
    if (find(SearchServer::begin(), SearchServer::end(), document_id) != SearchServer::end())
    {
        for (const auto& [word, freqs] : document_word_frequency.at(document_id)) 
        {
            word_to_document_freqs_.at(word).size() == 1 ? word_to_document_freqs_.erase(word) : word_to_document_freqs_.at(word).erase(document_id);
        }
        order_of_adding_documents.erase(find(order_of_adding_documents.begin(), order_of_adding_documents.end(), document_id));
        document_word_frequency.erase(document_id);
        documents_.erase(document_id);
        text_documents.erase(find_if(text_documents.begin(), text_documents.end(), [&document_id](std::pair<int, std::string> text_document) { return document_id == text_document.first; }));
    }
}

void SearchServer::RemoveDocument(std::execution::sequenced_policy seq, int document_id)
{
    if (std::find(SearchServer::begin(), SearchServer::end(), document_id) != SearchServer::end())
    {
        if (document_word_frequency.count(document_id) != 0)
        {
            std::map<std::string_view, double> words_freqs = document_word_frequency.at(document_id);
            std::vector<std::string_view> words(words_freqs.size());

            std::transform(seq, words_freqs.begin(), words_freqs.end(), words.begin(), [](const auto& word_freq) { return word_freq.first; });
            std::for_each(seq, words.begin(), words.end(), [this, &document_id](const std::string_view word) { word_to_document_freqs_.at(word).erase(document_id); });

            document_word_frequency.erase(document_id);
        }
        order_of_adding_documents.erase(document_id);
        documents_.erase(document_id);

        text_documents.erase(find_if(text_documents.begin(), text_documents.end(), [&document_id](std::pair<int, std::string> text_document) { return document_id == text_document.first; }));
    }
}

void SearchServer::RemoveDocument(std::execution::parallel_policy par, int document_id)
{
    if (std::find(SearchServer::begin(), SearchServer::end(), document_id) != SearchServer::end())
    {
        if (document_word_frequency.count(document_id) != 0)
        {
            std::map<std::string_view, double> words_freqs = document_word_frequency.at(document_id);
            std::vector<std::string_view> words(words_freqs.size());

            std::transform(par, words_freqs.begin(), words_freqs.end(), words.begin(), [](const auto& word_freq) { return word_freq.first; });
            std::for_each(par, words.begin(), words.end(), [this, &document_id](const std::string_view word) { word_to_document_freqs_.at(word).erase(document_id); });

            document_word_frequency.erase(document_id);
        }
        order_of_adding_documents.erase(document_id);
        documents_.erase(document_id);
        text_documents.erase(find_if(text_documents.begin(), text_documents.end(), [&document_id](std::pair<int, std::string> text_document) { return document_id == text_document.first; }));
    }
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query, DocumentStatus status_filter) const
{
    return SearchServer::FindTopDocuments(raw_query, [&status_filter](int document_id, DocumentStatus status, int rating) { return status == status_filter; });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query) const
{
    return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const
{
    return static_cast <int> (documents_.size());
}

std::set<int>::const_iterator SearchServer::begin()
{
    return order_of_adding_documents.cbegin();
}

std::set<int>::const_iterator SearchServer::end()
{
    return order_of_adding_documents.cend();
}

static const std::map<std::string_view, double> NO_DOCUMENT;
const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    return document_word_frequency.at(document_id).empty() ? NO_DOCUMENT : document_word_frequency.at(document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view& raw_query, int document_id) const
{
    return SearchServer::MatchDocument(std::execution::seq, raw_query, document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::execution::sequenced_policy seq, const std::string_view& raw_query, int document_id) const
{
    auto query_words = SearchServer::ParseQuery(raw_query);

    if (!std::any_of(seq, query_words.words_minus.begin(), query_words.words_minus.end(), [this, &document_id](const std::string_view& word)
        {
            return word_to_document_freqs_.at(word).count(document_id);
        }))
    {
        std::vector<std::string_view> words(query_words.words_plus.size());

        auto it = std::copy_if(seq, query_words.words_plus.begin(), query_words.words_plus.end(), words.begin(), [this, &document_id](const std::string_view& word)
            {
                return word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id);
            });
        words.resize(it - words.begin());

        std::sort(seq, words.begin(), words.end(), [](const std::string_view& str_1, const std::string_view& str_2) {return str_1 < str_2;});

        auto last = std::unique(seq, words.begin(), words.end());
        words.erase(last, words.end());

        return std::tuple(words, documents_.at(document_id).status);
    }
        return std::tuple(std::vector<std::string_view>(), documents_.at(document_id).status);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::execution::parallel_policy par, const std::string_view& raw_query, int document_id) const
{
    auto query_words = SearchServer::ParseQuery(par, raw_query);

    if (!std::any_of(par, query_words.words_minus.begin(), query_words.words_minus.end(), [this, &document_id](const std::string_view& word)
        {
            return word_to_document_freqs_.at(word).count(document_id);
        }))
    {
        std::vector<std::string_view> words(query_words.words_plus.size());

        auto it = std::copy_if(par, query_words.words_plus.begin(), query_words.words_plus.end(), words.begin(), [this, &document_id](const std::string_view& word)
            {
                return word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id);
            });
        words.resize(it - words.begin());

        std::sort(par, words.begin(), words.end(), [](const std::string_view& str_1, const std::string_view& str_2) {return str_1 < str_2;});

        auto last = std::unique(par, words.begin(), words.end());
        words.erase(last, words.end());

        return std::tuple(words, documents_.at(document_id).status);
    }
        return std::tuple(std::vector<std::string_view>(), documents_.at(document_id).status);
}

bool SearchServer::IsStopWord(const std::string_view& word) const
{
    return stop_words_.count(word) > 0;
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view& text) const
{
    std::vector<std::string_view> words;
    for (const std::string_view& word : SplitIntoWords(text))
    {
        if (!SearchServer::IsValidWord(word))
        {
            throw std::invalid_argument("invalid symbols in word -> " + std::string(word));
        }
        if (!SearchServer::IsStopWord(word))
        {
            words.push_back(word);
        }
    }
    return words;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view word) const
{
    if (!SearchServer::IsValidWord(word))
    {
        throw std::invalid_argument("invalid characters in query word -> " + std::string(word));
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

SearchServer::Query SearchServer::ParseQuery(const std::string_view& text) const
{
    SearchServer::Query word_plus_minus;
    for (const std::string_view& word : SplitIntoWords(text))
    {
        const SearchServer::QueryWord query_word = SearchServer::ParseQueryWord(word);
        if (!query_word.is_stop)
        {
            if (query_word.is_minus)
            {
                word_plus_minus.words_minus.push_back(query_word.data);
            }
            else
            {
                word_plus_minus.words_plus.push_back(query_word.data);
            }
        }
    }
    std::sort(word_plus_minus.words_plus.begin(), word_plus_minus.words_plus.end(), [](const std::string_view& str_1, const std::string_view& str_2) {return str_1 < str_2;});   // Сортирует слова words
    auto last_pluse = std::unique(word_plus_minus.words_plus.begin(), word_plus_minus.words_plus.end());
    word_plus_minus.words_plus.erase(last_pluse, word_plus_minus.words_plus.end());

    std::sort(word_plus_minus.words_minus.begin(), word_plus_minus.words_minus.end(), [](const std::string_view& str_1, const std::string_view& str_2) {return str_1 < str_2;});   // Сортирует слова words
    auto last_minuse = std::unique(word_plus_minus.words_minus.begin(), word_plus_minus.words_minus.end());
    word_plus_minus.words_minus.erase(last_minuse, word_plus_minus.words_minus.end());

    return word_plus_minus;
}

SearchServer::Query SearchServer::ParseQuery(std::execution::parallel_policy par, const std::string_view& text) const
{
    SearchServer::Query word_plus_minus;
    for (const std::string_view& word : SplitIntoWords(text))
    {
        const auto query_word = SearchServer::ParseQueryWord(word);
        if (!query_word.is_stop)
        {
            if (query_word.is_minus)
            {
                word_plus_minus.words_minus.push_back(query_word.data);
            }
            else
            {
                word_plus_minus.words_plus.push_back(query_word.data);
            }
        }
    }
    return word_plus_minus;
}

double SearchServer::InverseDocumentFrequency(const std::string_view& word) const
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

bool SearchServer::IsValidWord(const std::string_view& word)
{
    return std::none_of(word.begin(), word.end(), [](char c) { return c >= '\0' && c < ' '; });
}