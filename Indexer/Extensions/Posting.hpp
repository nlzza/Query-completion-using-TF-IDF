#pragma once
#ifndef POSTING_HPP
#define POSTING_HPP

#include "Document.hpp"

struct Posting
{
    unsigned doc_count{0};
    unsigned total_count{0};
    unsigned prev_docID = INVALID_DOC_ID;
    List<Document> documents;

    Posting() = default;

    Posting(const unsigned &doc_ID, const unsigned &line_no)
    {
        push_directly(doc_ID, line_no);
    }

    Posting(const Posting &other)
        : documents(other.documents), doc_count(other.doc_count) {}

    Posting &operator=(const Posting &other)
    {
        if (this == &other)
            return *this;

        this->doc_count = other.doc_count;
        this->documents = other.documents;
        return *this;
    }

    void push_directly(const unsigned &doc_ID, const unsigned &line_no)
    {
        total_count++;
        if (prev_docID == doc_ID)
            documents.last()->data.update(line_no);
        else
        {
            prev_docID = doc_ID;
            doc_count++;
            documents.push_back(Document(doc_ID, line_no));
        }
    }
};

#endif
