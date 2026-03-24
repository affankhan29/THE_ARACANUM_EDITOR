
#include "ManualStringHelpers.h"

int str_len(const wchar_t* str) {
    int count = 0;
    while (str[count] != L'\0') {
        count = count + 1;
    }
    return count;
}

int str_cmp(const wchar_t* s1, const wchar_t* s2) {
    int index = 0;
    while (s1[index] != L'\0' && s2[index] != L'\0') {
        if (s1[index] != s2[index]) {
            return s1[index] - s2[index];
        }
        index = index + 1;
    }
    return s1[index] - s2[index];
}

void str_cpy(wchar_t* out, const wchar_t* inp) {
    int index = 0;
    while (inp[index] != L'\0') {
        out[index] = inp[index];
        index = index + 1;
    }
    out[index] = L'\0';
}

void str_to_lower(wchar_t* str) {
    int index = 0;
    while (str[index] != L'\0') {
        if (str[index] >= L'A' && str[index] <= L'Z') {
            str[index] = str[index] + (32);
        }
        index = index + 1;
    }
}

bool is_break(wchar_t ch) {
    if (ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r') {
        return true;
    }
    if (ch == L'.' || ch == L',' || ch == L'!' || ch == L'?' ||
        ch == L';' || ch == L':' || ch == L'-' || ch == L'(' ||
        ch == L')' || ch == L'"' || ch == L'\'' || ch == L'[' ||
        ch == L']' || ch == L'{' || ch == L'}') {
        return true;
    }
    return false;
}

bool is_upper(wchar_t ch) {
    return (ch >= L'A' && ch <= L'Z');
}

bool is_lower(wchar_t ch) {
    return (ch >= L'a' && ch <= L'z');
}

wchar_t toggle_case(wchar_t ch) {
    if (is_upper(ch)) {
        return ch + 32;
    }
    else if (is_lower(ch)) {
        return ch - 32;
    }
    return ch;
}