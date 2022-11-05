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

MTFindResult readFile(const char *fileName, std::vector<std::string> &lines)
{
    std::string line;
    std::ifstream myfile;
    myfile.open(fileName);

    if (myfile.is_open() == false)
    {
        return MTFindResult::FAILURE;
    }

    while (getline(myfile, line))
    {
        lines.push_back(line);
    }

    return MTFindResult::OK;
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

std::vector<std::tuple<int, int, std::string>> findInRange(std::vector<std::string>::iterator begin, std::vector<std::string>::iterator end, const char *pattern, int patternSize, int offset = 0)
{
    std::vector<std::tuple<int, int, std::string>> results;
    int lineNumber = 0;
    for (auto& it = begin; it != end; ++it) 
    {
        auto result = bruteForceFind(it->c_str(), it->size(), pattern, patternSize);
        if (result)
        {
            int pos = result - it->c_str();
            results.push_back({lineNumber + offset, pos, {it->c_str() + pos, it->c_str() + pos + patternSize}});
        }
        ++lineNumber;
    }
    return results;
}

std::vector<std::tuple<int, int, std::string>> find(std::vector<std::string> &lines, const char *pattern, int parts = 1)
{
    std::vector<std::tuple<int, int, std::string>> results;

    int patternSize = strlen(pattern);

    parts = std::min(parts, static_cast<int>(lines.size()));
    int partSize = lines.size() / parts;

    auto it = lines.begin();
    int offset = 0;
    for(auto p = 0; p < parts - 1; ++p, it += partSize, offset += partSize)
    {
        std::vector<std::tuple<int, int, std::string>> results1 = findInRange(it, it + partSize, pattern, patternSize, offset);
        std::move(results1.begin(), results1.end(), std::back_inserter(results));
    }
    std::vector<std::tuple<int, int, std::string>> results2 = findInRange(it, lines.end(), pattern, patternSize, offset);
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

    std::vector<std::string> lines;
    auto result = readFile(argv[1], lines);

    if (result != MTFindResult::OK)
    {
        std::cerr << "Can't open file: " << argv[1] << std::endl;
        return static_cast<int>(result);
    }

    std::cerr << "\nFile contents:" << std::endl;
    for (auto &line : lines)
    {
        std::cout << line << "\n";
    }

    auto results = find(lines, argv[2], 100);

    std::cerr << "\nFind results:" << std::endl;
    if (results.size()) 
    {
        std::cout << results.size() << std::endl;
        for (auto &[line, pos, str] : results)
        {
            std::cout << line+1 << " " << pos+1 << " " << str << "\n";
        }
    }

    return static_cast<int>(MTFindResult::OK);
}
