#pragma once

#include <string>
#include <vector>
#include <regex>

namespace portaible
{
    static void stringReplaceAll(std::string& str, const std::string from, const std::string to)
    {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) 
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
    }

    static void splitStringToVector(const std::string& str, const std::string delimiterRegex, std::vector<std::string>& output)
    {
        output.clear();
        std::regex regexz(delimiterRegex);
        std::vector<std::string> list(std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
                                    std::sregex_token_iterator());
    }
}