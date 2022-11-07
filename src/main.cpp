#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <future>
#include <tuple>

#define THREADS_NUMBER 6

using MTMathResult = std::tuple<long, long, std::string_view>;
using MTMathResults = std::vector<MTMathResult>;
using MTMathInPartResults = std::tuple<long, MTMathResults>;

inline
void usage(const char *programName)
{
    std::cerr << "Usage:\n\t"
              << programName
              << " "
              << "<text_file_name>"
              << " "
              << "<pattern>"
              << std::endl;
}

inline
long readFileToBuffer(const char *fileName, char **buffer)
{
    FILE *fp;
    long lSize;

    fp = fopen(fileName, "rb");
    if (fp == nullptr)
    {
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    *buffer = (char *)malloc(lSize);
    if (*buffer == nullptr)
    {
        fclose(fp);
        return -2;
    }

    if (fread(*buffer, lSize, 1, fp) != 1)
    {
        fclose(fp);
        return -3;
    }
    
    fclose(fp);

    return lSize;
}

inline
MTMathInPartResults
findInPart(const char* text, const char* textEnd, const char* pattern, size_t patternSize)
{
    MTMathInPartResults result(0, {});
    const char* t = text;
    const char* p = pattern;
    const char* pe = pattern + patternSize;
    char c = 0;
    while(text < textEnd) 
    {
        c = *(text++); 

        if (c == '\n')
        {
            ++std::get<0>(result);
            t = text;
            p = pattern;
        }
        else
        {
            if (p < pe)
            {
                if (*p == c || *p == '?')
                {
                    ++p;
                }
                else
                {
                    text -= (p - pattern);
                    p = pattern;
                }
            }
            else
            {
                --text;
                std::get<1>(result).emplace_back(
                    static_cast<int>(std::get<0>(result)), 
                    static_cast<int>((text - patternSize) - t), 
                    std::string_view{(text - patternSize), static_cast<std::string_view::size_type>(patternSize)}
                );
                p = pattern;
            }
        }
    }
    return result;
}

inline
MTMathResults 
find(const char* text, size_t textSize, const char* pattern, size_t patternSize, int parts) 
{
    std::vector<std::future<MTMathInPartResults>> futures;

    size_t partSize = textSize / parts;
    const char* textEnd = text + textSize;

    const char* part = text;
    const char* partEnd = part + partSize;
    while (part < textEnd) 
    {
        while (partEnd < textEnd && *partEnd++ != '\n')
        {            
        }

        futures.push_back(std::async(std::launch::async, findInPart, part, partEnd, pattern, patternSize));

        part = partEnd;
        partEnd = part + partSize;
    }

    MTMathResults results;
    long offset = 0;
    for (auto&& future : futures)
    {
        auto&& [n, r] = future.get();

        for (auto& [s, p, str] : r)
        { 
            s += offset;
        }
        
        offset += n;

        results.reserve(results.size() + r.size());
        std::move(std::begin(r), std::end(r), std::back_inserter(results));
    }
    return results;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        usage(argv[0]);
        return 1;
    }

    char *text = nullptr;
    long textSize = readFileToBuffer(argv[1], &text);

    if (textSize < 0)
    {
        return 2;
    }

    MTMathResults mathResults = find(text, textSize, argv[2], strlen(argv[2]), THREADS_NUMBER);
    std::cout << mathResults.size() << std::endl;
    for (auto &[line, pos, str] : mathResults)
    {
        std::cout << (line + 1) << " " << (pos + 1) << " " << str << "\n";
    }

    free(text);

    return 0;
}
