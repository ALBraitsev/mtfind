#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <future>
#include <tuple>

#include <string.h>

#define NUMBER_OF_TASKS 6

using MTMathResult = std::tuple<long, long, std::string_view>;
using MTMathResults = std::vector<MTMathResult>;
using MTMathInPartResults = std::tuple<long, MTMathResults>;

/// @brief 
/// @param programName 
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

/// @brief читает содержимое файла в массив символов
/// использует файлы, а не потоки, так как они кратно быстрее на всех платформах
/// @param fileName имя файла
/// @param buffer адрес буфера
/// @return возвращает количество символов в массиве 
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

/// @brief ищет маску в заданной области массива символов
/// @param text содержимое исходного файла в виде массива символов 
/// @param textSize размер массива
/// @param pattern маска
/// @param patternSize размер маски
/// @return вектор результатов поиска {<номер строки>, <позиция в строке>, <найденный текст>} и количество символов '\\n' в заданной области массива символов
MTMathInPartResults
actual_search(const char* text, const char* textEnd, const char* pattern, size_t patternSize)
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

/// @brief ищет маску в массиве символов, 
/// разделяет работу на несколько независимых задач
/// объединяет результаты поиска
/// @param text содержимое исходного файла в виде массива символов 
/// @param textSize размер массива
/// @param pattern маска
/// @param patternSize размер маски
/// @param parts количество задач
/// @return вектор результатов поиска {<номер строки>, <позиция в строке>, <найденный текст>}
MTMathResults 
search(const char* text, size_t textSize, const char* pattern, size_t patternSize, int parts)
{
    std::vector<std::future<MTMathInPartResults>> futures;

    /// формируем и запускаем независимые задачи
    /// их должно быть parts - штук 
    /// но может и не получиться
    size_t partSize = textSize / parts;
    const char* textEnd = text + textSize;

    const char* part = text;
    const char* partEnd = part + partSize;
    while (part < textEnd) 
    {
        /// расширяем очередную область до следующего символа '\n'
        /// область всегда будет состоять из целого числа строк 
        while (partEnd < textEnd && *partEnd++ != '\n')
        {            
        }
        
        futures.push_back(std::async(std::launch::async, actual_search, part, partEnd, pattern, patternSize));

        part = partEnd;
        partEnd = part + partSize;
    }

    MTMathResults results;
    long offset = 0;
    for (auto&& future : futures)
    {
        /// получаем результаты выполнения каждой задачи
        auto&& [n, r] = future.get();

        /// пересчитываем номера строк
        for (auto& [s, p, str] : r)
        { 
            s += offset;
        }
        offset += n;

        /// объединяем результаты в один вектор
        results.reserve(results.size() + r.size());
        std::move(std::begin(r), std::end(r), std::back_inserter(results));
    }
    return results;
}

int main(int argc, char *argv[])
{
    /// для правильной работы нужны второй и третий аргумент
    /// argv[1] -- имя файла
    /// argv[2] -- маска для поиска
    if (argc < 3)
    {
        usage(argv[0]);
        return 1;
    }

    /// считываем содержимое файла в переменную text    
    char *text = nullptr;
    long textSize = readFileToBuffer(argv[1], &text);

    if (textSize < 0)
    {
        return 2;
    }

    /// ищем маску в тексте
    /// делим работу на NUMBER_OF_TASKS независимых задач
    MTMathResults mathResults = search(text, textSize, argv[2], strlen(argv[2]), NUMBER_OF_TASKS);

    /// вывод результатов
    std::cout << mathResults.size() << std::endl;
    for (auto &[line, pos, str] : mathResults)
    {
        std::cout << (line + 1) << " " << (pos + 1) << " " << str << "\n";
    }

    free(text);

    return 0;
}
