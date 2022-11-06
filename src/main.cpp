#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <future>
#include <tuple>

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

MTFindResult readFileToBuffer(const char *fileName, char *&buffer)
{
    FILE *fp;
    long lSize;

    fp = fopen(fileName, "rb");
    if (!fp)
        return MTFindResult::FAILURE;

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    /* allocate memory for entire content */
    buffer = (char *)malloc(lSize + 1);
    if (!buffer)
    {
        fclose(fp);
        return MTFindResult::FAILURE;
    }

    /* copy the file into the buffer */
    if (1 != fread(buffer, lSize, 1, fp))
    {
        fclose(fp);
        free(buffer);
        return MTFindResult::FAILURE;
    }
    buffer[lSize] = 0;
    
    /* do your work here, buffer is a string contains the whole text */
    fclose(fp);

    return MTFindResult::OK;
}

void splitBufferToLines(const char *buffer, char ld, std::vector<std::string_view> &lines)
{
    const char *pb = buffer;
    const char *pe = pb;
    while (*pe)
    {
        if (*pe == ld)
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
    for (auto &it = begin; it != end; ++it)
    {
        auto result = bruteForceFind(it->data(), static_cast<int>(it->size()), pattern, patternSize);
        if (result)
        {
            int pos = static_cast<int>(result - it->data());
            results.push_back({lineNumber + offset, pos, {it->data() + pos, static_cast<std::string_view::size_type>(patternSize)}});
        }
        ++lineNumber;
    }
    return results;
}

std::vector<std::tuple<int, int, std::string_view>> find(std::vector<std::string_view> &lines, const char *pattern, int parts = 1)
{
    std::vector<std::tuple<int, int, std::string_view>> results;

    auto patternSize = strlen(pattern);

    parts = std::min(parts, static_cast<int>(lines.size()));
    int partSize = static_cast<int>(lines.size() / parts);

    auto it = lines.begin();
    int offset = 0;

    std::vector<std::future<std::vector<std::tuple<int, int, std::string_view>>>> futures;
    for (auto p = 0; p < parts - 1; ++p, it += partSize, offset += partSize)
    {
        futures.push_back(std::async(std::launch::async, findInRange, it, it + partSize, pattern, patternSize, offset));
    }
    futures.push_back(std::async(std::launch::async, findInRange, it, lines.end(), pattern, patternSize, offset));

    for (auto &&future : futures)
    {
        auto r = future.get();
        std::move(r.begin(), r.end(), std::back_inserter(results));
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

    char *buffer = 0;
    auto result = readFileToBuffer(argv[1], buffer);

    if (result != MTFindResult::OK)
    {
        std::cerr << "Can't open file: " << argv[1] << std::endl;
        return static_cast<int>(result);
    }

    std::vector<std::string_view> lines;
    splitBufferToLines(buffer, '\n', lines);

    std::vector<std::tuple<int, int, std::string_view>> results;
    if (lines.size() > 0)
    {
        results = find(lines, argv[2], 8);
    }

    // std::cerr << "\nFind results:" << std::endl;
    {
        std::cout << results.size() << std::endl;
        for (auto &[line, pos, str] : results)
        {
            std::cout << line + 1 << " " << pos + 1 << " " << str << "\n";
        }
    }

    free(buffer);

    return static_cast<int>(MTFindResult::OK);
}
