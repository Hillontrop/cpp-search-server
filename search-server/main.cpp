#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

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

struct Document
{
    int id;
    double relevance;
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

    void AddDocument(int document_id, const string& document) 
    {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double TF = 1.0 / (double)words.size();
        for (const auto& word : words)
        {
            word_to_document_freqs_[word][document_id] += TF;
        }
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        const Words_plus_and_minus query_words = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs)
            {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    struct Words_plus_and_minus
    {
    public:
        set<string> words_plus;
        set<string> words_minus;
    };

    map<string, map<int, double>> word_to_document_freqs_;

    int document_count_ = 0;


    set<string> stop_words_;

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

    Words_plus_and_minus ParseQuery(const string& text) const
    {
        Words_plus_and_minus query_words;
        for (string& word : SplitIntoWordsNoStop(text))
        {
            if (word[0] != '-')
            {
                query_words.words_plus.insert(word);
            }
            else
            {
                word.erase(word.begin());
                query_words.words_minus.insert(word);
            }
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(const Words_plus_and_minus& query_words) const
    {
        map <int, double> document_to_relevance;
        vector<Document> matched_documents;
        for (const auto& word : query_words.words_plus)
        {
            if (word_to_document_freqs_.count(word) != 0)
            {
                double IDF = log((document_count_ * 1.0) / (double)word_to_document_freqs_.at(word).size());
                for (const auto& id : word_to_document_freqs_.at(word))
                {
                    if (document_to_relevance.count(id.first) == 0)
                    {
                        document_to_relevance.emplace(id.first, id.second * IDF);
                    }
                    else
                    {
                        document_to_relevance.at(id.first) += (id.second * IDF);
                    }
                }
            }
        }
        for (const auto& word_minus : query_words.words_minus)
        {
            if (word_to_document_freqs_.count(word_minus) != 0)
            {
                for (const auto& id : word_to_document_freqs_.at(word_minus))
                {
                    document_to_relevance.erase(id.first);
                }
            }
        }
        for (const auto& el : document_to_relevance)
        {
            matched_documents.push_back({ el.first,el.second });
        }
        document_to_relevance.clear();
        return matched_documents;
    }
};

SearchServer CreateSearchServer()
{
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id)
    {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main()
{
    setlocale(LC_ALL, "ru");
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& el : search_server.FindTopDocuments(query))
    {
        cout << "{ document_id = "s << el.id << ", "
            << "relevance = "s << el.relevance << " }"s << endl;
    }
    return 0;
}