#pragma once

#include <iostream>
#include <vector>


template <typename Iterator>
class IteratorRange
{
public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end), size_(distance(begin_, end_))
    {
    }
    Iterator begin() const
    {
        return begin_;
    }
    Iterator end() const
    {
        return end_;
    }
    size_t size() const
    {
        return size_;
    }
private:
    Iterator begin_;
    Iterator end_;
    size_t size_;
};

template<typename Iterator>
std::ostream& operator<<(std::ostream& os, const IteratorRange<Iterator>& range)
{
    for (auto it = range.begin(); it < range.end(); ++it)
    {
        os << *it;
    }
    return os;
}

template <typename Iterator>
class Paginator
{
public:
    Paginator(Iterator begin, Iterator end, size_t page_size)
    {
        size_t dis = distance(begin, end);
        auto page_end = begin;

        for (auto page_begin = begin; page_begin < end; advance(page_begin, page_size))
        {
            if (dis >= page_size)
            {
                advance(page_end, page_size);
                pages_.push_back(IteratorRange<Iterator>(page_begin, page_end));
                dis -= page_size;
            }
            else
            {
                pages_.push_back(IteratorRange<Iterator>(page_begin, end));
                break;
            }
        }
    }
    auto size() const
    {
        return pages_.size();
    }
    auto begin() const
    {
        return pages_.begin();
    }
    auto end() const
    {
        return pages_.end();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template<typename Iterator>
std::ostream& operator<<(std::ostream& os, const Paginator<Iterator>& page)
{
    for (auto it = page.begin(); std::distance(it, page.end()) > 0; std::advance(page))
    {
        os << *it;
    }
    return os;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size)
{
    return Paginator(begin(c), end(c), page_size);
}