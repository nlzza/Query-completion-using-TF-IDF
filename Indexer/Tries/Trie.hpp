#pragma once
#ifndef TRIE_HPP
#define TRIE_HPP

#include "HashTable.hpp"
#include <queue>
#include <vector>
#include <ostream>

class Trie
{
public:
    using Results = std::vector<std::pair<std::string, Posting *>>;

    Trie()
    {
        root = new HashTable;
    }
    ~Trie()
    {
        deleteTrie();
        delete root;
    }

    HashEntry *insert(const std::string &prefix);
    HashEntry *search(const std::string &prefix);
    HashEntry *search_incomplete(const std::string &str);

    void searchTopWords(std::string &partialStr, const unsigned &count, Results &results)
    {
        HashEntry *block = search_incomplete(partialStr);
        if (block != nullptr)
        {
            if (block->endOfWord)
                results.push_back(std::pair<std::string, Posting *>(partialStr, block->posting));
            if (block->next_table != nullptr)
                writeSearches(partialStr, block->next_table, count, results);
        }
    }

    void deleteTrie();

    void write(std::ostream &buffer)
    {
        std::string prefix;
        writeUtil(root, prefix, buffer);
    }

private:
    HashTable *root{0};

    void writeUtil(HashTable *ptr, std::string &prefix, std::ostream &buffer);

    void writeSearches(std::string &partialStr,
                       HashTable *ptr,
                       const unsigned &count,
                       Results &buffer);
};

HashEntry *Trie::insert(const std::string &prefix)
{
    HashTable *ptr = root;
    HashEntry *target;
    const auto &length = prefix.length();

    for (unsigned i = 0; i < length; i++)
    {
        // insert in current table
        target = ptr->search(prefix[i]);
        if (target == nullptr) // character not found
            target = ptr->insert(prefix[i]);

        if (i == length - 1)
        {
            target->endOfWord = true;
            break;
        }
        // move on to next table
        if (target->next_table == nullptr)
            target->next_table = new HashTable;
        ptr = target->next_table;
    }
    return target;
}

// finds the given std::string
// returns nullptr if not found
HashEntry *Trie::search(const std::string &prefix)
{
    HashTable *ptr = root;
    HashEntry *target;
    const auto &length = prefix.length();

    for (unsigned i = 0; i < length; i++)
    {
        // search in current table
        target = ptr->search(prefix[i]);
        if (target == nullptr)
            break;

        if (i == length - 1)
        {
            if (target->endOfWord == true)
                return target;
            break;
        }
        // move on to next table
        ptr = target->next_table;
        if (ptr == nullptr) // if next table does not exist
            break;
    }
    return nullptr;
}

// finds the given std::string
// returns nullptr if not found
HashEntry *Trie::search_incomplete(const std::string &str)
{
    HashTable *ptr = root;
    HashEntry *target;
    const auto &length = str.length();

    for (unsigned i = 0; i < length; i++)
    {
        // search in current table
        target = ptr->search(str[i]);
        if (target == nullptr) // if not in current table
            break;

        if (i == length - 1)
        {
            // if (target->endOfWord)
            // return nullptr;
            return target;
        }

        ptr = target->next_table;
        if (ptr == nullptr) // if next table does not exist
            break;
    }
    return nullptr;
}

void Trie::writeUtil(HashTable *ptr, std::string &prefix, std::ostream &buffer)
{
    for (HashEntry &beg : ptr->entries)
    {
        if (beg.empty == true)
            continue;

        prefix.push_back(beg.data);
        if (beg.endOfWord)
        {
            buffer << prefix
                   << " "
                   << beg.posting->doc_count
                   << " ";

            for (auto doc = beg.posting->documents.begin(); doc != nullptr; doc = doc->next)
            {
                buffer << doc->data.ID
                       << " "
                       << doc->data.term_freq;

                for (auto line = doc->data.lines.begin(); line != nullptr; line = line->next)
                    buffer << " "
                           << line->data;
                buffer << " ";
            }
            buffer << "\n";

            if (!beg.next_table)
            {
                prefix.pop_back();
                continue;
            }
        }
        if (beg.next_table)
        {
            writeUtil(beg.next_table, prefix, buffer);
            prefix.pop_back();
        }
    }
}

void Trie::writeSearches(std::string &partialStr,
                         HashTable *ptr,
                         const unsigned &count,
                         Results &buffer)
{
    for (HashEntry &beg : ptr->entries)
    {
        if (beg.empty)
            continue;

        partialStr.push_back(beg.data);
        if (beg.endOfWord)
        { // Insertion sort
            if (!buffer.empty())
            {
                BYTE i = buffer.size();
                for (; i > 0; i--)
                {
                    if (beg.posting->total_count < buffer[i - 1].second->total_count)
                        break;
                }
                buffer.insert(buffer.begin() + i, std::pair<std::string, Posting *>(partialStr, beg.posting));
                buffer.shrink_to_fit(); // memory optimisation
                if (buffer.size() > count)
                    buffer.pop_back();
            }
            else
                buffer.push_back(std::pair<std::string, Posting *>(partialStr, beg.posting));

            if (!beg.next_table)
            {
                partialStr.pop_back();
                continue;
            }
        }
        if (beg.next_table)
        {
            writeSearches(partialStr, beg.next_table, count, buffer);
            partialStr.pop_back();
        }
    }
}

void Trie::deleteTrie()
{
    std::queue<HashTable *> q;
    q.push(root);

    while (!q.empty())
    {
        HashTable *f = q.front();
        q.pop();

        for (HashEntry &beg : f->entries)
        {
            if (beg.next_table)
                q.push(beg.next_table);
        }
        delete f;
    }
    root = new HashTable;
}

#endif