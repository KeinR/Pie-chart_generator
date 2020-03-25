#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

struct fontMetrics {
    int aw;
    int lsb;
    int kern;
};

struct fLine {
    std::string::size_type cOffset;
    std::string::size_type cResume;
    int mapOffset;
    int width;
};

std::string formatLF(const char *file, int lineNumber);
std::string readWord(std::ifstream &file, int &lineNumber, char &buffer);
std::string readWordAll(std::ifstream &file, int &lineNumber, char &buffer);
int hexToDec(const std::string &value);
void toNewline(std::ifstream &file, char &buffer);
std::string readLine(std::ifstream &file);
int imin(int i1, int i2);
int imax(int i1, int i2);

void getStringMetrics(const std::string &str, const int wrapWidth, const float &fontScale, const stbtt_fontinfo &fontInfo, // in
                        std::vector<fontMetrics> &metrics, std::vector<fLine> &lines); // out

#endif
