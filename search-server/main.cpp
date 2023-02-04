#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine()
{
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text)
{
    vector<string> words;
    string word;
    for (const char c : text)
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
    {
        words.push_back(word);
    }

    return words;
}

vector<int> ReadLineWithRatings()
{
    vector<int> ratings;
    string str;
    getline(cin, str);

    for (size_t i = 1; i < SplitIntoWords(str).size(); i++)
    {
        ratings.push_back(stoi(SplitIntoWords(str)[i]));
    }

    return ratings;
}

struct Document
{
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer
{
public:
    void SetStopWords(const string& text)
    {
        for (const string& word : SplitIntoWords(text))
        {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings)
    {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double TF = 1.0 / static_cast<double>(words.size());

        for (const string& word : words)
        {
            word_to_document_freqs_[word][document_id] += TF;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    }

    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const
    {
        const WordsPlusMinus query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs)
            {
                if (abs(lhs.relevance - rhs.relevance) < 1e-6)
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

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status_filter) const
    {
        return FindTopDocuments(raw_query, [&status_filter](int document_id, DocumentStatus status, int rating) { return status == status_filter; });
    }

    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    int GetDocumentCount() const
    {
        return static_cast <int> (documents_.size());
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const
    {
        vector<string> words;

        const WordsPlusMinus query_words = ParseQuery(raw_query);

        for (const string& word : query_words.words_plus)
        {
            if (word_to_document_freqs_.count(word) != 0)
            {
                if (word_to_document_freqs_.at(word).count(document_id))
                {
                    words.push_back(word);
                }
            }
        }
        for (const string& word : query_words.words_minus)
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

        sort(begin(words), end(words), [](const string& str_1, const string& str_2) {return str_1 < str_2;});

        return make_tuple(words, documents_.at(document_id).status);
    }

private:

    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    map<string, map<int, double>> word_to_document_freqs_;

    set<string> stop_words_;

    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const
    {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const
    {
        vector<string> words;
        for (const string& word : SplitIntoWords(text))
        {
            if (!IsStopWord(word))
            {
                words.push_back(word);
            }
        }
        return words;
    }

    struct QueryWord
    {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const
    {
        bool is_minus = false;

        if (text[0] == '-')
        {
            is_minus = true;
            text = text.substr(1);
        }
        return
        {
            text,
            is_minus,
            IsStopWord(text)
        };
    }

    struct WordsPlusMinus
    {
    public:
        set<string> words_plus;
        set<string> words_minus;
    };

    WordsPlusMinus ParseQuery(const string& text) const
    {
        WordsPlusMinus word_plus_minus;
        for (const string& word : SplitIntoWords(text))
        {
            const QueryWord query_word = ParseQueryWord(word);
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

    double InverseDocumentFrequency(const string& word) const
    {
        return log(static_cast<double>(documents_.size()) / static_cast <double> (word_to_document_freqs_.at(word).size()));
    }

    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const WordsPlusMinus& query, DocumentPredicate document_predicate) const
    {
        map<int, double> document_to_relevance;
        for (const string& word : query.words_plus)
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

        for (const string& word : query.words_minus)
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

        vector<Document> matched_documents;
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

    static int ComputeAverageRating(const vector<int>& ratings)
    {
        if (ratings.empty())
        {
            return 0;
        }
        int number_of_ratings = ratings.size();
        return (accumulate(begin(ratings), end(ratings), 0) / number_of_ratings);
    }
};

SearchServer CreateSearchServer()
{
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id)
    {
        string text = ReadLine();
        vector<int> rating = ReadLineWithRatings();
        search_server.AddDocument(document_id, text,DocumentStatus::ACTUAL, rating);
    }

    return search_server;
}

void PrintDocument(const Document& document)
{
    cout << "{ "s << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}

void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status)
{
    cout << "{ "s
        << "document_id = "s << document_id << ", "s
        << "status = "s << static_cast<int>(status) << ", "s
        << "words ="s;
    for (const string& word : words)
    {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}

int main()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s))
    {
        PrintDocument(document);
    }
    cout << "ACTUAL:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
        [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; }))
    {
        PrintDocument(document);
    }
    cout << "BANNED:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED))
    {
        PrintDocument(document);
    }
    cout << "Even ids:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
        [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; }))
    {
        PrintDocument(document);
    }
    return 0;
}