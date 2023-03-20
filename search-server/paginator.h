#pragma once

#include <vector>

#include"iterator_range.h"

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