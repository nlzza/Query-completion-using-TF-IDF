#pragma once
#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include "HashEntry.hpp"
using BYTE = unsigned char;
using SBYTE = signed char;

class HashTable
{
    friend class Trie;

public:
    const static BYTE size = 26;

    HashTable() = default;

    HashTable(const HashTable &other)
    {
        for (BYTE i = 0; i < size; i++)
            this->entries[i].data = other.entries[i].data;
    }

    HashTable &operator=(const HashTable &other)
    {
        if (this == &other)
            return *this;

        for (BYTE i = 0; i < size; i++)
            this->entries[i].data = other.entries[i].data;
        return *this;
    }

    HashEntry *insert(const char &symbol)
    {
        HashEntry &entry = entries[symbol % size];
        entry.data = symbol;
        entries[symbol % size].empty = false;
        return &entry;
    }

    HashEntry *search(const char &symbol)
    {
        HashEntry &entry = entries[symbol % size];
        if (entry.empty)
            return nullptr;
        return &entries[symbol % size];
    }

private:
    HashEntry entries[size];
};

#endif
