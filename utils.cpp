#include "utils.h"


std::string Utils::hexToBin(std::string input) {
    std::string res = "";
    for(char &c : input) {
        switch(toupper(c)) {
            case '0': res += "0000"; break;
            case '1': res += "0001"; break;
            case '2': res += "0010"; break;
            case '3': res += "0011"; break;
            case '4': res += "0100"; break;
            case '5': res += "0101"; break;
            case '6': res += "0110"; break;
            case '7': res += "0111"; break;
            case '8': res += "1000"; break;
            case '9': res += "1001"; break;
            case 'A': res += "1010"; break;
            case 'B': res += "1011"; break;
            case 'C': res += "1100"; break;
            case 'D': res += "1101"; break;
            case 'E': res += "1110"; break;
            case 'F': res += "1111"; break;
            default:
                throw std::string("Not an hex value");
        }
    }
    return res;
}

std::string Utils::stringToHex(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

void Utils::incrementString(std::string &input)
{
    bool increment_done = false;
    int  index = input.size() -1;
    while(!increment_done && index >= 0) {
        if(input[index] == '9') {
            input[index] = 'A';
            increment_done = true;
        } else if(input[index] == 'F') {
            input[index] = '0';
            index--;
        } else {
            input[index]++;
            increment_done = true;
        }
    }
}

std::string Utils::initializeHexString(int len, char fillChar)
{
    std::string s;
    s.resize(len);
    for(char &c : s) c = fillChar;
    return s;
}

void Utils::exportToCSV(std::map<std::string, std::vector<HashWord> > &tree, int LSB, int lenInput)
{
    std::ofstream myfile;
    std::stringstream s;
    s << "collisions_" << lenInput << "_bits_on_" << LSB << "_bytes_msg_.csv";
    myfile.open (s.str());
    myfile << "Input in hexadecimal,Hashed value\n";
    for(auto &entry : tree) {
        if(entry.second.size() > 1) // Collisions if size > 1
            for(HashWord &hw : entry.second)
                myfile << "\"0x"<< hw.word << "\",\"0x" << hw.hash << "\"\n";

        myfile << "\n";
    }
    myfile.close();
}

std::string Utils::hexToString(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();
    if (len & 1) throw std::invalid_argument("odd length");

    std::string output;
    output.reserve(len / 2);
    for (size_t i = 0; i < len; i += 2)
    {
        char a = input[i];
        const char* p = std::lower_bound(lut, lut + 16, a);
        if (*p != a) throw std::invalid_argument("not a hex digit");

        char b = input[i + 1];
        const char* q = std::lower_bound(lut, lut + 16, b);
        if (*q != b) throw std::invalid_argument("not a hex digit");

        output.push_back(((p - lut) << 4) | (q - lut));
    }
    return output;
}


void Utils::split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}


std::vector<std::string> Utils::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

