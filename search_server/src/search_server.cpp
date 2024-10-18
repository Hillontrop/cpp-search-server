#include "search_server/search_server.h"


SearchServer::SearchServer(const std::string& stopWordsText)
    : SearchServer(std::string_view(stopWordsText))
{}

SearchServer::SearchServer(const std::string_view stopWordsText)
    : SearchServer(SplitIntoWords(stopWordsText))
{}

void SearchServer::AddDocument(
    const int documentId,
    const std::string_view document,
    const DocumentStatus status,
    const std::vector<int>& ratings)
{
    if (documentId < 0) {
        throw std::invalid_argument("Negative_id");
    }

    if (m_documents.count(documentId) != 0) {
        throw std::invalid_argument("id already exists");
    }

    m_textDocuments.emplace_back(documentId, std::string{document});

    const auto words = SplitIntoWordsNoStop(std::get<std::string>(m_textDocuments.back()));
    const double TF = 1.0 / static_cast<double>(words.size());

    for (const std::string_view word : words) {
        m_documentWordFrequency[documentId][word] += TF;
        m_wordToDocumentFreqs[word][documentId] += TF;
    }

    m_documents.emplace(documentId, DocumentData{ ComputeAverageRating(ratings), status });
    m_orderOfAddingDocuments.insert(documentId);
}

void SearchServer::RemoveDocument(const int documentId) {
    if (find(SearchServer::begin(), SearchServer::end(), documentId) != SearchServer::end()) {
        for (const auto& [word, freqs] : m_documentWordFrequency.at(documentId)) {
            m_wordToDocumentFreqs.at(word).size() == 1
                ? m_wordToDocumentFreqs.erase(word)
                : m_wordToDocumentFreqs.at(word).erase(documentId);
        }

        m_orderOfAddingDocuments.erase(find(m_orderOfAddingDocuments.begin(), m_orderOfAddingDocuments.end(), documentId));
        m_documentWordFrequency.erase(documentId);
        m_documents.erase(documentId);

        m_textDocuments.erase(
            find_if(
                m_textDocuments.begin(),
                m_textDocuments.end(),
                [&documentId](const std::pair<int, std::string>& text_document) {
                    return documentId == text_document.first;
                }
            )
        );
    }
}

void SearchServer::RemoveDocument(std::execution::sequenced_policy seq, const int documentId) {
    if (std::find(SearchServer::begin(), SearchServer::end(), documentId) != SearchServer::end())
    {
        if (m_documentWordFrequency.count(documentId) != 0) {
            std::map<std::string_view, double> wordsFreqs = m_documentWordFrequency.at(documentId);
            std::vector<std::string_view> words(wordsFreqs.size());

            std::transform(seq, wordsFreqs.begin(), wordsFreqs.end(), words.begin(), [](const auto& word_freq) { return word_freq.first; });
            std::for_each(
                seq,
                words.begin(),
                words.end(),
                [this, &documentId](const std::string_view word) {
                    m_wordToDocumentFreqs.at(word).erase(documentId);
                }
            );

            m_documentWordFrequency.erase(documentId);
        }
        m_orderOfAddingDocuments.erase(documentId);
        m_documents.erase(documentId);

        m_textDocuments.erase(
            find_if(
                m_textDocuments.begin(),
                m_textDocuments.end(),
                [&documentId](const std::pair<int, std::string>& textDocument) {
                    return documentId == textDocument.first;
                }
            )
        );
    }
}

void SearchServer::RemoveDocument(std::execution::parallel_policy par, const int documentId) {
    if (std::find(SearchServer::begin(), SearchServer::end(), documentId) != SearchServer::end()) {

        if (m_documentWordFrequency.count(documentId) != 0) {
            decltype(auto) wordsFreqs = m_documentWordFrequency.at(documentId);
            std::vector<std::string_view> words(wordsFreqs.size());

            std::transform(par, wordsFreqs.begin(), wordsFreqs.end(), words.begin(), [](const auto& wordFreq) { return wordFreq.first; });
            std::for_each(
                par,
                words.begin(),
                words.end(),
                [this, &documentId](const std::string_view word) {
                    m_wordToDocumentFreqs.at(word).erase(documentId);
                }
            );

            m_documentWordFrequency.erase(documentId);
        }
        m_orderOfAddingDocuments.erase(documentId);
        m_documents.erase(documentId);
        m_textDocuments.erase(find_if(m_textDocuments.begin(), m_textDocuments.end(), [&documentId](std::pair<int, std::string> text_document) { return documentId == text_document.first; }));
    }
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view rawQuery, const DocumentStatus statusFilter) const{
    return SearchServer::FindTopDocuments(
        rawQuery,
        [&statusFilter](const int documentId, const DocumentStatus status, const int rating) {
            return status == statusFilter;
        }
    );
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view rawQuery) const {
    return SearchServer::FindTopDocuments(rawQuery, DocumentStatus::ACTUAL);
}


static const std::map<std::string_view, double> NO_DOCUMENT;
const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(const int documentId) const {
    return m_documentWordFrequency.at(documentId).empty() ? NO_DOCUMENT : m_documentWordFrequency.at(documentId);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view rawQuery, const int documentId) const {
    return SearchServer::MatchDocument(std::execution::seq, rawQuery, documentId);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(
    std::execution::sequenced_policy seq,
    const std::string_view raw_query,
    const int documentId) const
{
    const auto queryWords = SearchServer::ParseQuery(raw_query);

    if (!std::any_of(seq, queryWords.WordsMinus.begin(), queryWords.WordsMinus.end(), [this, &documentId](const std::string_view word)
        {
            return m_wordToDocumentFreqs.at(word).count(documentId);
        }))
    {
        std::vector<std::string_view> words(queryWords.WordsPlus.size());

        auto it = std::copy_if(
            seq, queryWords.WordsPlus.begin(),
            queryWords.WordsPlus.end(),
            words.begin(),
            [this, &documentId](const std::string_view word) {
                return m_wordToDocumentFreqs.count(word) != 0 && m_wordToDocumentFreqs.at(word).count(documentId);
            }
        );

        words.resize(it - words.begin());

        std::sort(seq, words.begin(), words.end(), [](const std::string_view str_1, const std::string_view str_2) {return str_1 < str_2;});

        auto last = std::unique(seq, words.begin(), words.end());
        words.erase(last, words.end());

        return std::tuple(words, m_documents.at(documentId).Status);
    }
        return std::tuple(std::vector<std::string_view>(), m_documents.at(documentId).Status);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(
    std::execution::parallel_policy par,
    const std::string_view rawQuery,
    const int documentId) const
{
    const auto queryWords = SearchServer::ParseQuery(par, rawQuery);

    if (!std::any_of(
            par,
            queryWords.WordsMinus.begin(),
            queryWords.WordsMinus.end(),
            [this, &documentId](const std::string_view word) {
                return m_wordToDocumentFreqs.at(word).count(documentId);
            }
        ))
    {
        std::vector<std::string_view> words(queryWords.WordsPlus.size());

        const auto it = std::copy_if(
            par,
            queryWords.WordsPlus.begin(),
            queryWords.WordsPlus.end(),
            words.begin(),
            [this, &documentId](const std::string_view word) {
                return m_wordToDocumentFreqs.count(word) != 0 && m_wordToDocumentFreqs.at(word).count(documentId);
            }
        );
        words.resize(it - words.begin());

        std::sort(par, words.begin(), words.end(), [](const std::string_view str1, const std::string_view str2) {return str1 < str2;});

        const auto last = std::unique(par, words.begin(), words.end());
        words.erase(last, words.end());

        return std::tuple(words, m_documents.at(documentId).Status);
    }
    return std::tuple(std::vector<std::string_view>(), m_documents.at(documentId).Status);
}


std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view text) const {
    std::vector<std::string_view> words;
    for (const std::string_view word : SplitIntoWords(text))
    {
        if (!SearchServer::IsValidWord(word)) {
            throw std::invalid_argument("invalid symbols in word -> " + std::string(word));
        }

        if (!SearchServer::IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view word) const {
    if (!SearchServer::IsValidWord(word)) {
        throw std::invalid_argument("invalid characters in query word -> " + std::string(word));
    }
    bool isMinus = false;

    if (word.at(0) == '-') {
        word = word.substr(1);
        if (word.empty()) {
            throw std::invalid_argument("no text after - in the query");
        }

        if (word.at(0) == '-') {
            throw std::invalid_argument("more than one minus before minus-word in the query");
        }
        isMinus = true;
    }

    const SearchServer::QueryWord queryWord = { word, isMinus, SearchServer::IsStopWord(word) };
    return queryWord;
}

SearchServer::Query SearchServer::ParseQuery(const std::string_view text) const {
    SearchServer::Query wordPlusMinus;
    for (const std::string_view word : SplitIntoWords(text))
    {
        const SearchServer::QueryWord queryWord = SearchServer::ParseQueryWord(word);
        if (!queryWord.IsStop) {
            if (queryWord.IsMinus) {
                wordPlusMinus.WordsMinus.push_back(queryWord.Data);
                continue;
            }
            wordPlusMinus.WordsPlus.push_back(queryWord.Data);
        }
    }
    std::sort(
        wordPlusMinus.WordsPlus.begin(),
        wordPlusMinus.WordsPlus.end(),
        [](const std::string_view str1, const std::string_view str2) {
            return str1 < str2;
        }
    );
    const auto last_pluse = std::unique(wordPlusMinus.WordsPlus.begin(), wordPlusMinus.WordsPlus.end());
    wordPlusMinus.WordsPlus.erase(last_pluse, wordPlusMinus.WordsPlus.end());

    std::sort(
        wordPlusMinus.WordsMinus.begin(),
        wordPlusMinus.WordsMinus.end(),
        [](const std::string_view str1, const std::string_view str2) {
            return str1 < str2;
        }
    );
    const auto last_minuse = std::unique(wordPlusMinus.WordsMinus.begin(), wordPlusMinus.WordsMinus.end());
    wordPlusMinus.WordsMinus.erase(last_minuse, wordPlusMinus.WordsMinus.end());

    return wordPlusMinus;
}

SearchServer::Query SearchServer::ParseQuery(std::execution::parallel_policy par, const std::string_view text) const {
    SearchServer::Query wordPlusMinus;
    for (const std::string_view word : SplitIntoWords(text)) {
        const auto queryWord = SearchServer::ParseQueryWord(word);
        if (!queryWord.IsStop) {
            if (queryWord.IsMinus) {
                wordPlusMinus.WordsMinus.push_back(queryWord.Data);
                continue;
            }
            wordPlusMinus.WordsPlus.push_back(queryWord.Data);
        }
    }
    return wordPlusMinus;
}

double SearchServer::InverseDocumentFrequency(const std::string_view word) const {
    return log(static_cast<double>(m_documents.size()) / static_cast <double> (m_wordToDocumentFreqs.at(word).size()));
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    const int numberOfRatings = static_cast<int> (ratings.size());
    return (accumulate(ratings.begin(), ratings.end(), 0) / numberOfRatings);
}

bool SearchServer::IsValidWord(const std::string_view word) {
    return std::none_of(word.begin(), word.end(), [](const auto& c) { return c >= '\0' && c < ' '; });
}