#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>

typedef unsigned long long ull;

void format_size(ull bytes, wchar_t *out, size_t out_len) {
    const wchar_t *units[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
    int unit = 0;
    double size = (double)bytes;
    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        unit++;
    }
    _snwprintf(out, out_len, L"%.2f %s", size, units[unit]);
}

int should_skip(const wchar_t *name) {
    if (_wcsicmp(name, L"node_modules") == 0) return 1;
    if (_wcsicmp(name, L".git") == 0) return 1;
    return 0;
}

ull compute_direct_size(const wchar_t *folderPath) {
    WIN32_FIND_DATAW fd;
    wchar_t searchPath[MAX_PATH];
    ull totalSize = 0;

    _snwprintf(searchPath, MAX_PATH, L"%s\\*", folderPath);
    HANDLE h = FindFirstFileW(searchPath, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
            continue;

        wchar_t fullpath[MAX_PATH];
        _snwprintf(fullpath, MAX_PATH, L"%s\\%s", folderPath, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        } else {
            ull fileSize = ((ull)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
            totalSize += fileSize;
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    return totalSize;
}

void set_console_color(WORD attr) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, attr);
}

int wmain(int argc, wchar_t *argv[]) {
    if (argc != 2) {
        fwprintf(stderr, L"Usage: %s <folder path>\nExample: %s E:\\my-project\n", argv[0], argv[0]);
        return 1;
    }

    // Simpan atribut awal agar bisa dikembalikan
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    WORD defaultAttr = csbi.wAttributes;

    wchar_t base[MAX_PATH];
    if (!_wfullpath(base, argv[1], MAX_PATH)) {
        fwprintf(stderr, L"Failed normalisasi path.\n");
        return 1;
    }

    DWORD attr = GetFileAttributesW(base);
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        fwprintf(stderr, L"Path is not valid folder : %s\n", base);
        return 1;
    }

    // Header
    set_console_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    wprintf(L"List Folder in: ");
    set_console_color((FOREGROUND_GREEN | FOREGROUND_BLUE) | FOREGROUND_INTENSITY); // Cyan
    wprintf(L"%s\n\n", base);
    set_console_color(defaultAttr);

    WIN32_FIND_DATAW fd;
    wchar_t search[MAX_PATH];
    _snwprintf(search, MAX_PATH, L"%s\\*", base);
    HANDLE h = FindFirstFileW(search, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        fwprintf(stderr, L"Failed open directory: %s\n", base);
        return 1;
    }

    while (1) {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) {
            // skip
        } else {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (should_skip(fd.cFileName)) {
                    // tampilkan yang di-skip dengan warna abu-abu
                    set_console_color(FOREGROUND_INTENSITY); // light gray-ish
                    wchar_t subpath[MAX_PATH];
                    _snwprintf(subpath, MAX_PATH, L"%s\\%s", base, fd.cFileName);
                    wprintf(L"[SKIPPED] %s\n", subpath);
                    set_console_color(defaultAttr);
                } else {
                    wchar_t subpath[MAX_PATH];
                    _snwprintf(subpath, MAX_PATH, L"%s\\%s", base, fd.cFileName);
                    ull directSize = compute_direct_size(subpath);
                    wchar_t human[64];
                    format_size(directSize, human, sizeof(human)/sizeof(human[0]));

                    // Cetak path dengan warna cyan
                    set_console_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    wprintf(L"%s\t", subpath);

                    // Cetak bytes dengan warna kuning
                    set_console_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    wprintf(L"%llu bytes\t", directSize);

                    // Cetak human-readable size dengan warna magenta
                    set_console_color(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    wprintf(L"(%s)\n", human);

                    // kembalikan
                    set_console_color(defaultAttr);
                }
            }
        }
        if (!FindNextFileW(h, &fd)) break;
    }
    FindClose(h);
    // restore default just in case
    set_console_color(defaultAttr);
    return 0;
}
