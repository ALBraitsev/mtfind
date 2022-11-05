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

std::vector<std::tuple<int, int, std::string>> find(std::vector<std::string> &lines, const char *pattern)
{
    int patternSize = strlen(pattern);
    std::vector<std::tuple<int, int, std::string>> results;
    int lineNumber = 0;
    for (auto &line : lines)
    {
        auto result = bruteForceFind(line.c_str(), line.size(), pattern, patternSize);
        if (result)
        {
            int pos = result - line.c_str();
            results.push_back({lineNumber, pos, {line.c_str() + pos, line.c_str() + pos + patternSize}});
        }
        ++lineNumber;
    }

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

    auto results = find(lines, argv[2]);
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
