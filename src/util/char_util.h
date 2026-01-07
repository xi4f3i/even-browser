#pragma once

namespace CharUtil {

constexpr bool is_ascii_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

constexpr bool is_ascii_digit(char c) { return c >= '0' && c <= '9'; }

constexpr bool is_ascii_alphanumeric(char c)
{
    return is_ascii_digit(c) || is_ascii_alpha(c);
}

constexpr bool is_html_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

constexpr char to_ascii_upper(char c)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

constexpr char to_ascii_lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

} // namespace CharUtil