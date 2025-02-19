//
// Created by leondietrich on 8/7/24.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <string>
#include <list>

namespace utils {
    void ltrim(std::string &s);

    void rtrim(std::string &s);

    inline std::string trim(std::string &s) {
        rtrim(s);
        ltrim(s);
        return s;
    }

    [[nodiscard]] std::list<std::string> split(const std::string &text, const char delimiter);

    [[nodiscard]] std::string toupper(std::string s);

    /**
     * This method returns the specified element from the provided list.
     *
     * Warning: If there is no such element, it will return an empty string instead of throwing an exception.
     * @param l The list to return the element from
     * @param position The position of the element inside the list
     * @return The element or an empty string
     */
    [[nodiscard]] std::string get_from_str_list(const std::list<std::string> &l, size_t position);

    [[nodiscard]] bool str_is_empty(const std::string &s);

    [[nodiscard]] bool str_contains(const std::string &s, const char c);

    [[nodiscard]] bool str_contains(const std::string &search_in, const std::string &search_for);

    /**
     * Use this method to replace substrings in the target string
     * @param s The string to perform the operation on
     * @param what The sequence to replace
     * @param with The content to replace it with
     * @return The new string
     */
    [[nodiscard]] std::string str_replace(const std::string &s, const std::string &what, const std::string &with);

    [[nodiscard]] bool is_number(const std::string &s);

    [[nodiscard]] bool stob(const std::string& s);

}
