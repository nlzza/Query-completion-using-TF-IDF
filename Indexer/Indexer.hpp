#pragma once
#ifndef INDEXER_HPP
#define INDEXER_HPP
#define TOTAL_DOCS (248153)
#include <fstream>
#include <string>
#include "Tries/Trie.hpp"
#include <cmath>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>

class Indexer
{
    std::string word;

private:
    char c1, c2;
    bool go{true};
    char findChar{0};
    unsigned line_no{0};
    HashEntry *target{0};

    Trie dictionary;

public:
    Indexer() = default;

    void index(const char *filename, const unsigned &doc_ID = 0)
    {
        go = true;
        line_no = 0;

        FILE *file = fopen(filename, "r");
        while ((c1 = fgetc(file)) != EOF)
        {
            c2 = fgetc(file);
            ungetc(c2, file);

            if (c1 == '\n')
                line_no++;

            if (go == false)
            {
                if (c1 == findChar)
                {
                    if (findChar != '*')
                    {
                        go = true;
                        word.clear();
                    }
                    else if (c2 == '/')
                    {
                        go = true;
                        word.clear();
                        fgetc(file);
                    }
                }
                continue;
            }

            if (c1 == '\"' || c1 == '\'')
            {
                findChar = c1;
                go = false;
                continue;
            }

            if (c1 == '/')
            {
                if (c2 == '/')
                {
                    findChar = '\n';
                    go = false;
                    continue;
                }
                else if (c2 == '*')
                {
                    findChar = '*';
                    go = false;
                    continue;
                }
            }

            c1 |= 32;
            if (c1 >= 'a' && c1 <= 'z')
                word.push_back(c1);
            else
            {
                if (word.length() > 3)
                {
                    target = dictionary.insert(word);
                    if (target->posting)
                        target->posting->push_directly(doc_ID, line_no);
                    else
                        target->posting = new Posting(doc_ID, line_no);
                }
                word.clear();
            }
        }
        fclose(file);
    }

    void write_on(const char *filename)
    {
        std::ofstream file;
        file.open(filename, std::ios::out);
        dictionary.write(file);
        file.close();
    }

    void read(const char *filename)
    {
        std::string token;
        unsigned doc_count{0};
        unsigned doc_ID{0};
        unsigned term_freq{0};
        unsigned line_no{0};
        HashEntry *target;

        dictionary.deleteTrie();

        std::ifstream file;
        file.open(filename, std::ios::in);
        while (!file.eof())
        {
            file >> token;
            if (file.eof())
                break;

            file >> doc_count;
            if (file.eof())
                break;

            for (unsigned i = 0; i < doc_count; i++)
            {
                file >> doc_ID;
                if (file.eof())
                    break;
                file >> term_freq;
                if (file.eof())
                    break;

                for (unsigned j = 0; j < term_freq; j++)
                {
                    file >> line_no;
                    if (file.eof())
                        break;

                    target = dictionary.insert(token);
                    if (target->posting)
                        target->posting->push_directly(doc_ID, line_no);
                    else
                        target->posting = new Posting(doc_ID, line_no);
                }
            }
        }
        file.close();
    }

    HashEntry *search(const std::string &token)
    {
        return dictionary.search(token);
    }

    // Seeks to the given line of a text file
    void goToLine(std::ifstream &file, unsigned line)
    {
        file.seekg(0);

        if (line == 0)
            return;

        for (unsigned i = 0; i < line - 1; ++i)
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Returns tf-idf score
    float getTfIdf(const unsigned &term_freq, const unsigned &doc_count)
    {
        return ((float)(1.0 + log10(term_freq)) * log10(TOTAL_DOCS / doc_count));
    }

    void rankDocs(const Posting *posting, std::vector<std::pair<Document *, float>> &docs)
    {
        for (auto doc = posting->documents.begin(); doc; doc = doc->next)
        {
            float score = getTfIdf(doc->data.term_freq, posting->doc_count);
            docs.push_back(std::pair<Document *, float>(&(doc->data), score));
        }

        sort(docs.begin(), docs.end(),
             [](const std::pair<Document *, float> &lhs, const std::pair<Document *, float> &rhs)
             { return lhs.second > rhs.second; });

        if (docs.size() > 10)
            docs.resize(10);
    }

    void scoreWords(const Posting *posting, std::set<std::string> &dest)
    {
        float highest = -1;
        std::vector<std::pair<Document *, float>> highestIDs; // Holds the Documents structure for the highest ranking docs

        rankDocs(posting, highestIDs);
        // Traverse the highest ranking docs

        for (auto docID : highestIDs)
        {
            printf("%ld: \n", docID.first->ID);

            std::ifstream code;
            code.open("D:\\Data\\Fall 2021\\DS\\Project\\code_and_comments\\chunks\\dataset\\" + std::to_string(docID.first->ID) + ".txt", std::ios::in);

            for (auto line = docID.first->lines.begin(); line != nullptr; line = line->next)
            {
                goToLine(code, line->data + 1);

                std::string snippet;
                getline(code, snippet);

                auto pos = snippet.find_first_not_of(" \t"); // Trim out indents
                if (pos != std::string::npos)
                    snippet = snippet.substr(pos); // from the line of code
                if (!snippet.empty())
                    dest.insert(snippet);
            }
            code.close();
        }
    }

    void searchWords(std::string &partialStr, const unsigned &count)
    {
        Trie::Results results;
        dictionary.searchTopWords(partialStr, count, results);

        if (results.empty())
        {
            puts("No results were found");
            return;
        }

        std::set<std::string> words;
        for (auto &pair : results)
        {
            words.clear();
            printf("%s ", pair.first.c_str());
            printf("%ld ", pair.second->doc_count);
            printf("%ld\n", pair.second->total_count);
            scoreWords(pair.second, words);

            for (auto &word : words)
            {
                puts(word.c_str());
            }
            puts("\n");
        }
    }

    // void complete_line(std::set<std::string> &context, std::string &incomplete)
    void complete_line(std::string &context, std::string &incomplete, std::set<std::string> &dest)
    {
        Trie::Results query_res;
        dictionary.searchTopWords(incomplete, 5, query_res);
        if (query_res.empty())
        {
            puts("No query results were found");
            return;
        }

        Trie::Results ctx_res;
        dictionary.searchTopWords(context, 5, ctx_res);
        if (ctx_res.empty())
        {
            puts("No context results were found");
            ;
        }

        std::vector<std::tuple<Document *, Document *, float>> final_res;
        bool doc_intersected{false};
        float highest = -1;

        // go in each query result:
        for (auto &pair : query_res)
        {
            // go in each query doc:
            for (auto query_doc = pair.second->documents.begin(); query_doc; query_doc = query_doc->next)
            {
                bool go = true;
                // go in each context result:
                for (auto &ctx_pair : ctx_res)
                {
                    // go in each context doc:
                    for (auto ctx_doc = ctx_pair.second->documents.begin(); go && ctx_doc; ctx_doc = ctx_doc->next)
                    {
                        // if an intersection exits:
                        if (ctx_doc->data.ID == query_doc->data.ID)
                        {
                            doc_intersected = true;

                            // tf-idf of query in common doc:
                            float score1 = getTfIdf(query_doc->data.term_freq, pair.second->doc_count);
                            // tf-idf of ctx in common doc:
                            float score2 = getTfIdf(ctx_doc->data.term_freq, ctx_pair.second->doc_count);
                            float final_score = score1 * score2;

                            final_res.push_back(std::tuple<Document *, Document *, float>(&(query_doc->data), &(ctx_doc->data), final_score));

                            go = false;
                        }
                    }
                    if (go == false)
                        break;
                }
            }
        }

        sort(final_res.begin(), final_res.end(),
             [](const std::tuple<Document *, Document *, float> &lhs, const std::tuple<Document *, Document *, float> &rhs)
             { return std::get<2>(lhs) > std::get<2>(rhs); });

        if (final_res.size() > 5)
            final_res.resize(5);

        // Traverse the highest ranking docs

        if (doc_intersected == false)
        {
            // puts("The words are not found together in any document");
            if (ctx_res.size() > 5)
                ctx_res.resize(5);

            std::vector<std::pair<Document *, float>> results;
            for (auto &pair : ctx_res)
            {
                rankDocs(pair.second, results);
                if (results.size() > 5)
                    results.resize(5);

                for (auto &pair : results)
                {
                    for (auto ctx_line = pair.first->lines.begin(); ctx_line; ctx_line = ctx_line->next)
                    {
                        std::ifstream code;
                        code.open("D:\\Data\\Fall 2021\\DS\\Project\\code_and_comments\\chunks\\dataset\\" + std::to_string(pair.first->ID) + ".txt", std::ios::in);

                        goToLine(code, ctx_line->data + 1);

                        std::string snippet;
                        getline(code, snippet);

                        auto pos = snippet.find_first_not_of(" \t"); // Trim out indents
                        if (pos != std::string::npos)
                            snippet = snippet.substr(pos); // from line of code

                        if (!snippet.empty())
                        {
                            dest.insert(snippet);
                        }
                    }
                }
            }
            return;
        }

        bool line_intersected{false};
        for (auto final_doc : final_res)
        {
            for (auto query_line = std::get<0>(final_doc)->lines.begin(); query_line; query_line = query_line->next)
            {
                bool go = true;
                for (auto ctx_line = std::get<1>(final_doc)->lines.begin(); go && ctx_line; ctx_line = ctx_line->next)
                {
                    if (query_line->data == ctx_line->data)
                    {
                        line_intersected = true;

                        std::ifstream code;
                        code.open("D:\\Data\\Fall 2021\\DS\\Project\\code_and_comments\\chunks\\dataset\\" + std::to_string(std::get<0>(final_doc)->ID) + ".txt", std::ios::in);

                        goToLine(code, query_line->data + 1);

                        std::string snippet;
                        getline(code, snippet);

                        auto pos = snippet.find_first_not_of(" \t"); // Trim out indents
                        if (pos != std::string::npos)
                            snippet = snippet.substr(pos); // from line of code

                        if (!snippet.empty())
                        {
                            dest.insert(snippet);
                        }
                        code.close();
                        go = false;
                    }
                }
            }
        }

        if (line_intersected == false)
        {
            for (auto &final_doc : final_res)
            {
                for (auto ctx_line = std::get<1>(final_doc)->lines.begin(); ctx_line; ctx_line = ctx_line->next)
                {
                    line_intersected = true;

                    std::ifstream code;
                    code.open("D:\\Data\\Fall 2021\\DS\\Project\\code_and_comments\\chunks\\dataset\\" + std::to_string(std::get<0>(final_doc)->ID) + ".txt", std::ios::in);

                    goToLine(code, ctx_line->data + 1);

                    std::string snippet;
                    getline(code, snippet);

                    auto pos = snippet.find_first_not_of(" \t"); // Trim out indents
                    if (pos != std::string::npos)
                        snippet = snippet.substr(pos); // from line of code

                    if (!snippet.empty())
                    {
                        dest.insert(snippet);
                    }
                    code.close();
                }
            }
            return;
        }
    }
};
#endif