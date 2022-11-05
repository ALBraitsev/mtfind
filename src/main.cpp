#include <iostream>
#include <fstream>
#include <string>
#include <vector>

enum class MTFindResult
{
    OK,
    FAILURE
};

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

MTFindResult readFileToString(const char *fileName, std::string &string)
{
    std::string line;
    std::ifstream myfile;
    myfile.open(fileName);

    if (myfile.is_open() == false)
    {
        return MTFindResult::FAILURE;
    }

    string.assign((std::istreambuf_iterator<char>(myfile)),
                    std::istreambuf_iterator<char>());

    return MTFindResult::OK;
}

void splitBufferToLines(const std::string& buffer, std::vector<std::string_view>& lines)
{
    const char* pb = buffer.data();
    const char* pe = pb;
    while(*pe) 
    {
        if (*pe == '\n') 
        {
            lines.emplace_back(pb, pe - pb);
            pb = pe + 1;
        }
        ++pe;
    }
    lines.emplace_back(pb, pe - pb);
} 

const char *bruteForceFind(const char *string, int stringSize,
                           const char *pattern, int patternSize)
{
    if (patternSize > stringSize)
        return nullptr;

    int i, j;
    for (j = 0; j <= stringSize - patternSize; ++j)
    {
        for (i = 0; i < patternSize && (pattern[i] == '?' || pattern[i] == string[i + j]); ++i)
            ;
        if (i >= patternSize)
            return string + j;
    }

    return nullptr;
}

std::vector<std::tuple<int, int, std::string_view>> findInRange(std::vector<std::string_view>::iterator begin, std::vector<std::string_view>::iterator end, const char *pattern, int patternSize, int offset = 0)
{
    std::vector<std::tuple<int, int, std::string_view>> results;
    int lineNumber = 0;
    for (auto& it = begin; it != end; ++it) 
    {
        auto result = bruteForceFind(it->data(), it->size(), pattern, patternSize);
        if (result)
        {
            int pos = result - it->data();
            results.push_back({lineNumber + offset, pos, {it->data() + pos, static_cast<std::string_view::size_type>(patternSize)}});
        }
        ++lineNumber;
    }
    return results;
}

std::vector<std::tuple<int, int, std::string_view>> find(std::vector<std::string_view> &lines, const char *pattern, int parts = 1)
{
    std::vector<std::tuple<int, int, std::string_view>> results;

    int patternSize = strlen(pattern);

    parts = std::min(parts, static_cast<int>(lines.size()));
    int partSize = lines.size() / parts;

    auto it = lines.begin();
    int offset = 0;
    for(auto p = 0; p < parts - 1; ++p, it += partSize, offset += partSize)
    {
        std::vector<std::tuple<int, int, std::string_view>> results1 = findInRange(it, it + partSize, pattern, patternSize, offset);
        std::move(results1.begin(), results1.end(), std::back_inserter(results));
    }
    std::vector<std::tuple<int, int, std::string_view>> results2 = findInRange(it, lines.end(), pattern, patternSize, offset);
    std::move(results2.begin(), results2.end(), std::back_inserter(results));

    return results;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        usage(argv[0]);
        return static_cast<int>(MTFindResult::FAILURE);
    }

    std::cerr << "Find \"" << argv[2] << "\" in file " << argv[1] << std::endl;

    std::string buffer;
    auto result = readFileToString(argv[1], buffer);

    if (result != MTFindResult::OK)
    {
        std::cerr << "Can't open file: " << argv[1] << std::endl;
        return static_cast<int>(result);
    }

    std::vector<std::string_view> lines;
    splitBufferToLines(buffer, lines);

    // std::cerr << "\nFile contents:" << std::endl;
    // for (auto &line : lines)
    // {
    //     std::cerr << line << "\n";
    // }

    std::vector<std::tuple<int, int, std::string_view>> results;
    if (lines.size() > 0) 
    {
        results = find(lines, argv[2], 100);
    }
    
    // std::cerr << "\nFind results:" << std::endl;
    {
        std::cout << results.size() << std::endl;
        for (auto &[line, pos, str] : results)
        {
            std::cout << line+1 << " " << pos+1 << " " << str << "\n";
        }
    }

    return static_cast<int>(MTFindResult::OK);
}
