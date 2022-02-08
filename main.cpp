#include <iostream>
#include <time.h>
#include <set>
#include <windows.h>
#include "Indexer/Indexer.hpp"
using namespace std;

bool readingCompleted{false};

Indexer indexer;

DWORD WINAPI readIndex(LPVOID lpParam)
{
    CONSOLE_CURSOR_INFO info;

    info.dwSize = 100;
    info.bVisible = FALSE;
    COORD origin = {0, 0};
    HANDLE stdH = GetStdHandle(STD_OUTPUT_HANDLE);

    indexer.read("index.txt");
    readingCompleted = true;
    ExitThread(0);
}

int main()
{
    char c = 'a';
    char in;

    HANDLE hThread = CreateThread(0, 0, readIndex, 0, 0, 0);

    while (c != 'e')
    {
        system("cls");

        string ctx;
        string query;

        while ((in = getchar()) != '\n')
        {
            if (in == ' ')
            {
                ctx = query;
                query.clear();
            }
            else
                query.push_back(in);
        }

        while (readingCompleted == false)
        {
        }

        set<string> results;

        if (!ctx.empty())
            indexer.complete_line(ctx, query, results);

        for (auto &result : results)
        {
            puts(result.c_str());
        }

        puts("\n\nPress 'e' to exit");
        fflush(stdin);
        c = getchar();
    }
    CloseHandle(hThread);

    return 0;
}