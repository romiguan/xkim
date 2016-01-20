#ifndef __STRING_ALGORITHMS_H__
#define __STRING_ALGORITHMS_H__

#include <string>
#include <vector>

namespace util
{

class StringAlgorithms
{
    private:

        static int _Min(int a, int b);
        static int _Min(int a, int b, int c);
        static int _Max(int a, int b);
        static int _Max(int a, int b, int c);

        template <typename T>
            static int _EditDistance(const T tstr1[], size_t tstr1len, const T tstr2[],
                    size_t tstr2len);

        template <typename T>
            static int _LCSS(const T tstr1[], size_t tstr1len, const T tstr2[],
                    size_t tstr2len);

    public:

        /*
         * 强力的空格清除, 字符串前导和后继空格全部抛弃, 连续的空白符号, 用一个指定的替换符号代替
         * 算法将对原始字符串内容进行变更
         */
        static int Trim(char in[], char replace = ' ');
        static int Trim(std::string& in, char replace = ' ');

        /*
         * Trim方法的拷贝版本, 保存原始字符串
         */
        static int TrimCopy(const char in[], std::string& out, char replace = ' ');
        static int TrimCopy(const std::string& in, std::string& out, char replace = ' ');

        /*
         * 求两字符串间编辑距离, 该距离实际上是Damerau–Levenshtein距离, 所支持的操作除了包括
         * Levenshtein距离定义的添加, 删除, 替换之外, 还包括相邻字符的对调操作
         */
        static int EditDistance(const char str1[], const char str2[]);
        static int EditDistance(const std::string& str1, const std::string& str2);

        /*
         * 编辑距离的宽字符集版本
         */
        static int EditDistance(const wchar_t wstr1[], const wchar_t wstr2[]);
        static int EditDistance(const std::wstring& wstr1, const std::wstring& wstr2);


        /*
         * 求两字符串的最长公共子序列长度
         */
        static int LCSS(const std::string& str1, const std::string& str2);
        static int LCSS(const char str1[], const char str2[]);

        /*
         * 最长公共子序列的宽字符集版本
         */
        static int LCSS(const std::wstring& wstr1, const std::wstring& wstr2);
        static int LCSS(const wchar_t wstr1[], const wchar_t wstr2[]);

        /*
         * 将输入字符串, 按照指定的分隔符进行分割
         */
        static int Split(const std::string& str, char delim, std::vector<std::string>& results);
        static int Split(const char str[], char delim, std::vector<std::string>& results);

        /*
         * 扩展版本，分隔符可以是字符串，且如果分隔符之间为空，则分割结果也为空
         */
        static int Split(const std::string& str, const char *delim, std::vector<std::string>& results);
        static int Split(const char str[], const char *delim, std::vector<std::string>& results);
};

}


#endif /* STRINGALGORITHMS_H_ */
