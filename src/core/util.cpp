#include "util.h"
#include <iostream>
#include <cstring>
#include <cctype>

using namespace std;

int clampInt(int val, int minVal, int maxVal) {
    if (val < minVal) {
        return minVal;
    }
    if (val > maxVal) {
        return maxVal;
    }
    return val;
}

void trimWhitespace(char* str) {
    if (str == nullptr) {
        return;
    }

    int len = static_cast<int>(strlen(str));
    if (len == 0) {
        return;
    }

    // Find first non-whitespace character
    int start = 0;
    while (start < len && isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }

    // Find last non-whitespace character
    int end = len - 1;
    while (end >= start && isspace(static_cast<unsigned char>(str[end]))) {
        end--;
    }

    // Shift trimmed substring to index 0
    int newLen = end - start + 1;
    for (int i = 0; i < newLen; i++) {
        str[i] = str[start + i];
    }
    str[newLen] = '\0';
}

bool stringEquals(const char* a, const char* b) {
    if (a == nullptr || b == nullptr) {
        return false;
    }
    return strcmp(a, b) == 0;
}

void printHeader(const char* title) {
    cout << "\n========================================\n";
    if (title != nullptr) {
        cout << "  " << title << "\n";
    }
    cout << "========================================\n";
}

void clearScreen() {
    // Print newlines for portable, deterministic console separation
    cout << "\n----------------------------------------\n";
}