//
// Created by leondietrich on 8/7/24.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "utils.h"

#include <algorithm>
#include <cctype>
#include <locale>
#include <sstream>

namespace utils {

    void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    bool is_number(const std::string &s) {
        return !s.empty() &&
               std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    }

    std::list<std::string> split(const std::string &text, const char delimiter) {
        std::list<std::string> l;
        std::stringstream ss;
        for (const auto c: text) {
            if (c == delimiter) {
                l.push_back(ss.str());
                ss.str("");
            } else {
                ss << c;
            }
        }
        l.push_back(ss.str());
        return l;
    }

    std::string toupper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
        return s;
    }

    std::string get_from_str_list(const std::list<std::string> &l, size_t position) {
        if (l.size() <= position) {
            return "";
        }
        size_t i = 0;
        for (auto &elem: l) {
            if (i == position) {
                return elem;
            } else {
                i++;
            }
        }
        return "";
    }

    bool str_is_empty(const std::string &s) {
        for (const auto &c: s) {
            if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
                return false;
            }
        }
        return true;
    }

    bool str_contains(const std::string &s, const char c) {
        for (const auto x: s) {
            if (x == c) {
                return true;
            }
        }
        return false;
    }

    bool str_contains(const std::string &search_in, const std::string &search_for) {
        const auto l = search_in.length();
        for (unsigned int i = 0, j = 0; i < l; i++) {
            if (search_in.at(i) == search_for.at(j)) {
                j++;
                if (j == search_for.length()) {
                    return true;
                }
            } else {
                j = 0;
                if (search_in.at(i) == search_for.at(j)) {
                    j++;
                }
            }
        }
        return false;
    }

    std::string str_replace(const std::string &s, const std::string &what, const std::string &with) {
        std::stringstream sb;

        if (what.empty()) {
            return with;
        }

        for (size_t i = 0; i < s.length(); i++) {
            bool found = true;

            for (size_t j = 0; j < what.length(); j++) {
                if (s.at(i + j) != what.at(j)) {
                    found = false;
                    break;
                }
            }

            if (found) {
                sb << with;
                i += what.length() - 1;
            } else {
                sb << s.at(i);
            }
        }

        return sb.str();
    }

    bool stob(const std::string& s) {
        return toupper(s) == "TRUE" || s == "1";
    }

}