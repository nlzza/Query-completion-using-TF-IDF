#pragma once
#ifndef DOCUMENT_HPP
#define DOCUMENT_HPP

#include "List.hpp"
#define INVALID_DOC_ID (-1)

struct Document
{
    const unsigned ID;
    unsigned term_freq{0};
    List<unsigned> lines;

    Document()
        : ID(0) {}

    Document(const unsigned &ID)
        : ID(ID) {}

    Document(const unsigned &ID, const unsigned &line_no)
        : ID(ID)
    {
        this->update(line_no);
    }

    Document(const Document &other)
        : ID(other.ID), term_freq(other.term_freq), lines(other.lines) {}

    Document &operator=(const Document &other)
    {
        if (this == &other)
            return *this;

        this->term_freq = other.term_freq;
        this->lines = other.lines;
        return *this;
    }

    void update(const unsigned &line_no)
    {
        term_freq++;
        lines.push_back(line_no);
    }

    bool operator==(const Document &other) const
    {
        return this->ID == other.ID;
    }
};

#endif
