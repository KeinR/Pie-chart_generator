#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

struct data {
    double value;
    int enterX;
    int enterY;
    std::string name;
    // Colors
    unsigned char RBG[3]{0};
};

std::string formatLF(const char *file, int lineNumber);
std::string readWord(std::ifstream &file, int &lineNumber, char &buffer);
int hexToDec(const std::string &value);
void toNewline(std::ifstream &file, char &buffer);
std::string readLine(std::ifstream &file);

#endif
