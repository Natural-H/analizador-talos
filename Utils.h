//
// Created by Marco on 6/9/2025.
//

#ifndef UTILS_H
#define UTILS_H
#include <map>

template <typename K, typename V>
K* getKeyFromValue(const std::map<K, V>& m, const V& value)
{
    for (const auto& pair : m)
    {
        if (pair.second == value)
        {
            K item = pair.first;
            return &item;
        }
    }
    return nullptr;
}

inline long long findLineIndex(const std::string& s, const char target, int startIndex = 0)
{
    startIndex = s.length() - startIndex;
    const auto offset = s.crbegin() + startIndex;

    for (auto it = offset; it != s.crend(); ++it)
        if (*it == target)
            return std::distance(offset, it);

    return std::distance(offset, s.crend());
}

#endif //UTILS_H
