#ifndef UTILS_H
#define UTILS_H
#include <string>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>
#include "sha1.h"

class Utils {
public:

    /*!
     * \brief hexToBin
     * \param input
     * \return
     */
    static std::string hexToBin(std::string input);

    /*!
     * \brief hexToString
     * \param input
     * \return
     */
    static std::string hexToString(const std::string& input);

    /*!
     * \brief stringToHex
     * \param input
     * \return
     */
    static std::string stringToHex(const std::string& input);

    /*!
     * \brief Increment the hex value contained in a string.
     *        To increase the speed of the algorithm,
     *        I do not check wheter the input contains only hex chars or not.
     *
     * \param input The string to increment
     */
    static void incrementString(std::string &input);

    /*!
     * \brief Initialize a string filled with the given argument
     *        (0 by default) and with a size of len.
     * \param len The length of the string
     * \return The string
     */
    static std::string initializeHexString(int len, char fillChar = '0');

    /*!
     * \brief Utils::split
     * \param s
     * \param delim
     * \param elems
     * \author Evan Teran (from Stackoverflow)
     * \see http://stackoverflow.com/questions/236129/split-a-string-in-c
     */
    static void split(const std::string &s, char delim, std::vector<std::string> &elems);

    /*!
     * \brief Utils::split
     * \param s
     * \param delim
     * \return
     * \author Evan Teran (from Stackoverflow)
     * \see http://stackoverflow.com/questions/236129/split-a-string-in-c
     */
    static std::vector<std::string> split(const std::string &s, char delim);
    /*!
     * \brief exportToCSV
     * \param tree
     */
    static void exportToCSV(std::map<std::string, std::vector<HashWord>> &tree, int LSB, int lenInput);
};

#endif // UTILS_H
