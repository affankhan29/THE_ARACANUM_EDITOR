#pragma once

int str_len(const wchar_t* str);

int str_cmp(const wchar_t* s1, const wchar_t* s2);

void str_cpy(wchar_t* dest, const wchar_t* src);

void str_to_lower(wchar_t* str);

bool is_break(wchar_t ch);


bool is_upper(wchar_t ch);

bool is_lower(wchar_t ch);

wchar_t toggle_case(wchar_t ch);