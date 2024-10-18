#pragma once

#include <iostream>
#include <vector>


template <typename Iterator>
class IteratorRange
{
public:
    IteratorRange(Iterator begin, Iterator end)
        : m_begin(begin)
        , m_end(end)
        , m_size(distance(m_begin, m_end))
    {}

    inline Iterator begin() const noexcept {
        return m_begin;
    }

    inline Iterator end() const noexcept {
        return m_end;
    }

    inline size_t size() const noexcept {
        return m_size;
    }

private:
    Iterator m_begin;
    Iterator m_end;
    size_t m_size;
};

template<typename Iterator>
std::ostream& operator<<(std::ostream& os, const IteratorRange<Iterator>& range) {
    for (auto it = range.begin(); it < range.end(); ++it) {
        os << *it;
    }
    return os;
}

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, const size_t pageSize) {
        size_t dis = distance(begin, end);
        auto pageEnd = begin;

        for (auto pageBegin = begin; pageBegin < end; advance(pageBegin, pageSize)) {
            if (dis < pageSize) {
                m_pages.push_back(IteratorRange<Iterator>(pageBegin, end));
                break;
            }
            advance(pageEnd, pageSize);
            m_pages.push_back(IteratorRange<Iterator>(pageBegin, pageEnd));
            dis -= pageSize;
        }
    }
    inline auto size() const noexcept {
        return m_pages.size();
    }

    inline auto begin() const {
        return m_pages.begin();
    }

    inline auto end() const {
        return m_pages.end();
    }

private:
    std::vector<IteratorRange<Iterator>> m_pages;
};

template<typename Iterator>
std::ostream& operator<<(std::ostream& os, const Paginator<Iterator>& page) {
    for (auto it = page.begin(); std::distance(it, page.end()) > 0; std::advance(page)) {
        os << *it;
    }
    return os;
}

template <typename Container>
auto Paginate(const Container& c, const size_t pageSize) {
    return Paginator{begin(c), end(c), pageSize};
}