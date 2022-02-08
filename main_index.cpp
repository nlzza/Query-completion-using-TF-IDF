#include "Indexer/Indexer.hpp"

#include <iostream>
#include <filesystem>
#include <time.h>
#include <windows.h>
#define TOTAL (24815)
using namespace std;

unsigned countFiles{0};
bool go{true};

DWORD WINAPI printCount(LPVOID lpParam)
{
    CONSOLE_CURSOR_INFO info;

    info.dwSize = 100;
    info.bVisible = FALSE;
    COORD origin = {0, 0};
    HANDLE stdH = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleCursorInfo(stdH, &info);

    while (go)
    {
        printf("%.1f percent", (100.0 * countFiles) / TOTAL);
        SetConsoleCursorPosition(stdH, origin);
    }
    ExitThread(0);
}

int main()
{
    countFiles = 0;
    HANDLE hThread = CreateThread(0, 0, printCount, 0, 0, 0);
    unsigned id;
    Indexer indexer;

    time_t begin, end;
    time(&begin);

    for (const auto &entry : filesystem::directory_iterator("../dataset"))
    {
        const string &filenameStr = entry.path().filename().string();
        id = stoi(filenameStr.substr(0, filenameStr.length() - 4));

        indexer.index(("../dataset/" + filenameStr).c_str(), id);

        countFiles++;
        if (countFiles == TOTAL)
            break;
    }

    time(&end);
    printf("\nTime measured: %d seconds.\n", end - begin);
    go = false;

    indexer.write_on("index_short.txt");
    CloseHandle(hThread);

    fflush(stdin);
    system("pause");

    return 0;
}
