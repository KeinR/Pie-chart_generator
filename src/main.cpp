#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <vector>
#include <cmath>
#include <functional>

#include <stb/stb_image_write.h>
#include <stb/stb_truetype.h>

#include "main.h"

#define DEBUG_LOCATION "chartgen.log"
#define TTF_FONT "resources/Arial.ttf"
#define PI 3.141592653589793
#define CHANNELS 4
#define CIRCLE_FINENESS 1000 // 0 may break something

int margin = 10;
int radius = 100;
std::string out = "piechart.jpg";
std::string title;
int titleSize = 10;
int fontSize = 10;
bool printPercent = true;

std::vector<data> dat;

int main(int argc, char **argv) {
    // Generate prophile
    if (argc == 1) {
        std::cerr << "Must provide at least one config file" << std::endl;
        return 1;
    }

    std::set_terminate([]() -> void {
        std::cerr << "Terminate called after an uncaught exception was thrown" << std::endl;
    });

    for (int i = 1; i < argc; i++) {
        std::ifstream file;
        try {
            file.exceptions(std::ifstream::badbit); // Fucking failbit getting set by eofbit
            file.open(argv[i], std::ios::binary);
            if (!file.good()) {
                std::cout << "ERROR: Failed to open file \"" << argv[i] << "\"" << std::endl;
                file.close();
                return 1;
            }

            int lineNumber = 1;
            char buffer;
            while (true) {
                // Each line
                file.get(buffer);
                if (file.eof()) {
                    break;
                }
                for (; buffer == ' '; file.get(buffer));
                switch (buffer) {
                    case '\r': // Fallthrough
                        file.get(buffer);
                    case '\n':
                        lineNumber++;
                        break;
                    case '#': // Comments, just like this one!
                        for (; !file.eof(); file.get(buffer)) {
                            if (buffer == '\n') {
                                lineNumber++;
                                break;
                            }
                        }
                        break;
                    case '@': {
                        std::string command = readWord(file, lineNumber, buffer);
                        // Convert to lowercase
                        for (std::string::size_type i = 0; i < command.length(); i++) {
                            if (command[i] >= 'A' && command[i] <= 'Z') {
                                command[i] += 0x20; // Or 32 in decimal- I find it easier to remember "20" though... space ' ' works too
                            }
                        }
                        if (command == "out") {
                            out = readWord(file, lineNumber, buffer);
                        } else if (command == "title") {
                            for (; buffer == ' '; file.get(buffer));
                            title.clear();
                            for (; buffer != '\n' && buffer != '\r'; file.get(buffer)) {
                                title += buffer;
                            }
                        } else if (command == "percent") {
                            printPercent = true;
                        } else if (command == "value") {
                            printPercent = false;
                        } else {
                            try {
                                if (command == "radius") {
                                    command = readWord(file, lineNumber, buffer);
                                    radius = std::stoi(command);
                                } else if (command == "margin") {
                                    command = readWord(file, lineNumber, buffer);
                                    margin = std::stoi(command);
                                } else if (command == "titlesize") {
                                    command = readWord(file, lineNumber, buffer);
                                    titleSize = std::stoi(command);
                                } else if (command == "fontsize") {
                                    command = readWord(file, lineNumber, buffer);
                                    fontSize = std::stoi(command);
                                } else {
                                    std::cerr << formatLF(argv[i], lineNumber) <<
                                    " Unrecognized setting \"" << command << "\"" << std::endl;
                                }
                            } catch (std::invalid_argument &e) {
                                std::cerr << formatLF(argv[i], lineNumber) <<
                                    " Expected \"" << command << "\" to be an integer" << std::endl;
                            }
                        }
                        toNewline(file, buffer);
                        lineNumber++;
                        break;
                    }
                    default: {
                        file.putback(buffer);
                        data d;
                        std::string sbuff = readWord(file, lineNumber, buffer);
                        d.value = std::stod(sbuff);

                        sbuff.clear();
                        for (; buffer == ' '; file.get(buffer));
                        for (bool cont = true;; file.get(buffer)) {
                            if (!file.eof()) {
                                switch (buffer) {
                                    case '\r': // Fallthrough
                                        file.get(buffer);
                                    case '\n':
                                        lineNumber++;
                                        d.name = sbuff;
                                        cont = false;
                                        break;
                                    case '#': {
                                        // Remove last few whitespace chars, if there are any
                                        for (int i = sbuff.length()-1; i >= 0; i--) {
                                            if (sbuff[i] != ' ') {
                                                sbuff.erase(i+1, sbuff.length()-1-i);
                                                break;
                                            }
                                        }
                                        d.name = sbuff;

                                        sbuff = readWord(file, lineNumber, buffer);

                                        bool setUp = false;
                                        for (int i = sbuff.length()-1, current = 2; i >= 0; i--) {
                                            int val;
                                            if (sbuff[i] >= '0' && sbuff[i] <= '9') {
                                                val = sbuff[i] - '0';
                                            } else if (sbuff[i] >= 'A' && sbuff[i] <= 'F') {
                                                val = sbuff[i] - 0x37;
                                            } else if (sbuff[i] >= 'a' && sbuff[i] <= 'f') {
                                                val = sbuff[i] - 0x57;
                                            } else {
                                                std::cerr << formatLF(argv[i], lineNumber) << " Invalid hex string \"" << sbuff <<
                                                "\", can only contain chars 0-9, a-f, A-F" << std::endl;
                                                break;
                                            }
                                            if (current < 0) {
                                                std::cerr << formatLF(argv[i], lineNumber) << " Invalid hex string \"" << sbuff <<
                                                "\", must be at or below 6 characters" << std::endl;
                                                break;
                                            }
                                            d.RBG[current] += val * std::pow(16, (int)setUp);
                                            if (setUp) {
                                                setUp = false;
                                                current--;
                                            } else {
                                                setUp = true;
                                            }
                                        }
                                        toNewline(file, buffer);
                                        cont = false;
                                        break;
                                    }
                                    default:
                                        sbuff += buffer;
                                }
                            } else {
                                d.name = sbuff;
                                cont = false; // break works also
                            }
                            if (!cont) {
                                break;
                            }
                        }

                        dat.push_back(d);
                        break;
                    }
                }
            }

            file.close();
        } catch (std::ifstream::failure &e) {
            std::cerr << "ERROR: Failed to read from config file \"" << argv[i] << "\": " << e.what() << std::endl;
            return 1;
        }
    }

    std::ofstream debug(DEBUG_LOCATION);

    // ------------- debug -------------

    debug << "margin: " << margin << std::endl;
    debug << "radius: " << radius << std::endl;
    debug << "out: " << out << std::endl;
    debug << "title: " << title << std::endl;
    debug << "titleSize: " << titleSize << std::endl;
    debug << "printPercent: " << printPercent << std::endl;

    debug << "-----------" << std::endl;

    for (std::vector<data>::size_type i = 0; i < dat.size(); i++) {
        debug << ">>>>>>>>>>>> name: " << dat[i].name << std::endl;
        debug << "value: " << dat[i].value << std::endl;
        debug << "red: " << (int)dat[i].RBG[0] << std::endl;
        debug << "blue: " << (int)dat[i].RBG[1] << std::endl;
        debug << "green: " << (int)dat[i].RBG[2] << std::endl;
    }

    // ------------- end debug -------------

    const int diameter = radius * 2;

    const int width = diameter + margin; //+ std::round(diameter * 0.07) + 
    const int height = diameter + margin + titleSize;

    const int chartCenterX = radius;
    const int chartCenterY = radius + margin + titleSize;

    const int bitmapSize = width * height * CHANNELS;
    uint8_t *bitmap = new uint8_t[bitmapSize]{0};

    debug << "Total size in MB: " << ((double)diameter * diameter * CHANNELS / 1000000) << std::endl;

    const int movX = CHANNELS;
    const int movY = width * movX;

    // ------------ font loading -----------

    std::ifstream fontFile(TTF_FONT);
    if (!fontFile.good()) {
        std::cerr << "ERROR: Failed to open font file at \"" TTF_FONT "\"" << std::endl;
        return 1;
    }

    fontFile.seekg(0, fontFile.end);
    const int fontBufferSize = fontFile.tellg();
    fontFile.seekg(0, fontFile.beg);

    uint8_t *fontBuffer = new uint8_t[fontBufferSize];

    fontFile.read((char *)fontBuffer, fontBufferSize);
    fontFile.close();
    if (!fontFile.good()) {
        std::cerr << "ERROR: Failed to read from font file at \"" TTF_FONT "\"" << std::endl;
        delete[] fontBuffer;
        return 1;
    }

    stbtt_fontinfo finfo;
    if (!stbtt_InitFont(&finfo, fontBuffer, 0)) {
        std::cerr << "ERROR: Failed to parse font from \"" TTF_FONT "\"" << std::endl;
        return 1;
    }

    const float fscale = stbtt_ScaleForPixelHeight(&finfo, titleSize);

    int fascent, fdescent, flineGap;
    stbtt_GetFontVMetrics(&finfo, &fascent, &fdescent, &flineGap);
    fascent *= fscale;
    fdescent *= fscale;

    // Determine positioning to allow for center-align

    int fx = 0;
    std::string::size_type charOffset = 0;
    std::vector<int> sizeHistory;
    sizeHistory.reserve(title.length());
    for (std::string::size_type i = 0; i < title.length(); i++) {
        int aw, lsb;
        stbtt_GetCodepointHMetrics(&finfo, title[i], &aw, &lsb);
        std::cout << "aw=" << aw << ", lsb=" << lsb << std::endl;
        sizeHistory[i] = aw * fscale;

        fx += sizeHistory[i];
        if (i+1 >= title.length() || fx >= chartCenterX + radius) {
            if (fx >= chartCenterX + radius) { // Soft word wrap
                for (std::string::size_type newOffset = i;; newOffset--) {
                    if (newOffset < 0) {
                        break;
                    }
                    if (title[newOffset] == ' ') {
                        for (; title[newOffset] == ' '; newOffset--);
                        for (; i > newOffset; i--) {
                            fx -= sizeHistory[i];
                        }
                        break;
                    }
                }
            }

            const int cxOffset = (chartCenterX + radius - fx) / 2;
            const int cxWidth = fx;
            std::cout << "Offset: " << cxOffset << std::endl;

            const int charbpmSize = cxWidth * titleSize;
            uint8_t *charbpm = new uint8_t[charbpmSize];

            fx = 0;
            for (std::string::size_type c = charOffset; c <= i; c++) {
                std::cout << "Vroom, c=" << c << ", i=" << i << std::endl;
                // ty Justin Meiners very helpful
                int ax;
                int lsb;
                stbtt_GetCodepointHMetrics(&finfo, title[c], &ax, &lsb);

                /* get bounding box for character (may be offset to account for chars that dip above or below the line */
                int c_x1, c_y1, c_x2, c_y2;
                stbtt_GetCodepointBitmapBox(&finfo, title[c], fscale, fscale, &c_x1, &c_y1, &c_x2, &c_y2);

                /* compute y (different characters have different heights */
                int y = fascent + c_y1;

                /* render character (stride and offset is important here) */
                int byteOffset = fx + (lsb * fscale) + (y * cxWidth);
                stbtt_MakeCodepointBitmap(&finfo, charbpm + byteOffset, c_x2 - c_x1, c_y2 - c_y1, cxWidth, fscale, fscale, title[i]);

                /* advance x */
                fx += ax * fscale;

                if (c+1 <= i) {
                    /* add kerning */
                    fx += stbtt_GetCodepointKernAdvance(&finfo, title[i], title[i+1]) * fscale;
                } else {
                    break;
                }
            }

            // Render into output bitmap
            for (int y = 0; y < titleSize; y++) {
                for (int x = 0; x < cxWidth; x++) {
                    const int ci = x + y * cxWidth;
                    if (charbpm[ci] != 0) {
                        const int index = (x + cxOffset) * movX + y * movY;
                        bitmap[index] = charbpm[ci];
                        bitmap[index+1] = charbpm[ci];
                        bitmap[index+2] = charbpm[ci];
                        bitmap[index+3] = 0xFF;
                    }
                }
            }

            // AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAdebug
            stbi_write_bmp("DEBUG.bmp", cxWidth, titleSize, 1, charbpm);

            delete[] charbpm;

            fx = 0;
            charOffset = i+1;
        } else {
            const int kern = stbtt_GetCodepointKernAdvance(&finfo, title[i], title[i+1]) * fscale;
            sizeHistory[i] += kern;
            fx += kern;
        }
    }

    delete[] fontBuffer;

    // ------------- end font loading ----------

    debug << "Enter and pry,,," << std::endl;

    const int radiusSqr = radius * radius;

    // Draw sections

    int total = 0;
    for (std::vector<data>::size_type i = 0; i < dat.size(); i++) {
        total += dat[i].value;
    }

    double lastSlope = std::nextafter(0.0d, 1.0d);
    double lastYIntercept = chartCenterY;
    int lastDirectionX = 1;
    int lastX = chartCenterX + radius;

    double netRads = 0;
    bool cont = true;
    for (std::vector<data>::size_type i = 0; cont; i++) {
        const double rads = dat[i].value / total * 2 * PI;
        netRads += rads;
        debug << "Net rads = " << netRads << std::endl;
        // Trig paying off like oh yah
        const double ex = chartCenterX + radius * std::cos(netRads);
        const double ey = chartCenterY + radius * std::sin(netRads);

        const int exi = std::round(ex);

        double slope;
        double yIntercept;

        if (i+1 < dat.size()) {
            if (ex != chartCenterX) {
                slope = (ey - chartCenterY) / (ex - chartCenterX);
                yIntercept = ey - ex * slope;
            } else {
                slope = std::nextafter(0.0d, 1.0d);
                yIntercept = chartCenterY;
            }
        } else {
            slope = std::nextafter(0.0d, 1.0d);
            yIntercept = chartCenterY;
            cont = false;
        }

        const int directionX = (exi - chartCenterX) / std::abs(exi - chartCenterX);

        std::function<int(const int&)> leStartY;
        std::function<bool(const int&, const int&)> leEndY; // If it IS the end
        int leXEnd;
        int x;
        if (directionX != lastDirectionX) {
            if (directionX == 1) {
                x = lastX;
                leXEnd = exi;
                leStartY = [&radiusSqr, &chartCenterX, &chartCenterY](const int &x) -> int {
                    return chartCenterY - std::round(std::sqrt(radiusSqr - std::pow(x - chartCenterX, 2)));
                };
                leEndY = [&lastSlope, &lastYIntercept, &slope, &yIntercept, &chartCenterX](const int &x, const int &y) -> bool {
                    if (x > chartCenterX) {
                        return y > std::round(slope * x + yIntercept);
                    } else {
                        return y > std::round(lastSlope * x + lastYIntercept);
                    }
                };
            } else {
                x = exi;
                leXEnd = lastX;
                leStartY = [&lastSlope, &lastYIntercept, &slope, &yIntercept, &chartCenterX](const int &x) -> int {
                    if (x < chartCenterX) {
                        return std::round(slope * x + yIntercept);
                    } else {
                        return std::round(lastSlope * x + lastYIntercept);
                    }
                };
                leEndY = [&radiusSqr, &chartCenterX, &chartCenterY](const int &x, const int &y) -> bool {
                    return y > chartCenterY + std::round(std::sqrt(radiusSqr - std::pow(x - chartCenterX, 2)));
                };
            }
        } else if (directionX == 1) {
            x = chartCenterX;
            leXEnd = chartCenterY + radius;
            leStartY = [&lastSlope, &lastYIntercept, &radiusSqr, &chartCenterX, &chartCenterY](const int &x) -> int {
                const int alt = chartCenterY - std::round(std::sqrt(radiusSqr - std::pow(x - chartCenterX, 2)));
                const int prim = std::round(lastSlope * x + lastYIntercept);
                return imax(alt, prim);
            };
            leEndY = [&slope, &yIntercept, &radiusSqr, &chartCenterX, &chartCenterY](const int &x, const int &y) -> bool {
                const int alt = chartCenterY + std::round(std::sqrt(radiusSqr - std::pow(x - chartCenterX, 2)));
                const int prim = std::round(slope * x + yIntercept);
                const int tend = imin(alt, prim);
                return y > tend;
            };
        } else {
            x = 0;
            leXEnd = chartCenterX;
            leStartY = [&slope, &yIntercept, &radiusSqr, &chartCenterX, &chartCenterY](const int &x) -> int {
                const int alt = chartCenterY - std::round(std::sqrt(radiusSqr - std::pow(x - chartCenterX, 2)));
                const int prim = std::round(slope * x + yIntercept);
                return imax(alt, prim);
            };
            leEndY = [&lastSlope, &lastYIntercept, &radiusSqr, &chartCenterX, &chartCenterY](const int &x, const int &y) -> bool {
                const int alt = chartCenterY + std::round(std::sqrt(radiusSqr - std::pow(x - chartCenterX, 2)));
                const int prim = std::round(lastSlope * x + lastYIntercept);
                const int tend = imin(alt, prim);
                return y > tend;
            };
        }

        for (; x < leXEnd; x++) {
            for (int y = leStartY(x); !leEndY(x, y); y++) {
                const int index = x * movX + y * movY;
                bitmap[index] = dat[i].RBG[0];
                bitmap[index+1] = dat[i].RBG[1];
                bitmap[index+2] = dat[i].RBG[2];
                bitmap[index+3] = 0xFF;
            }
        }


        lastSlope = slope;
        lastYIntercept = yIntercept;
        lastDirectionX = directionX;
        lastX = exi;
    }

    stbi_write_png(out.c_str(), width, height, CHANNELS, bitmap, width * CHANNELS);
    // stbi_write_bmp("out.bmp", diameter, diameter, CHANNELS, bitmap);

    // Cleanup
    debug.close();
    delete[] bitmap;

}

std::string formatLF(const char *file, int lineNumber) {
    std::string result;
    result += '[';
    result += file;
    result += ':';
    result += std::to_string(lineNumber);
    result += ']';
    return result;
}

std::string readWord(std::ifstream &file, int &lineNumber, char &buffer) {
    std::string outBuffer;
    file.get(buffer);
    // Skip whitespace
    for (; buffer == ' '; file.get(buffer));
    // Read word
    for (; buffer != ' '; file.get(buffer)) {
        if (buffer == '\r') {
            file.get(buffer);
            lineNumber++;
            break;
        }
        if (buffer == '\n') {
            lineNumber++;
            break;
        }
        outBuffer += buffer;
    }
    return outBuffer;
}

void toNewline(std::ifstream &file, char &buffer) {
    for (; buffer != '\n'; file.get(buffer));
}

std::string readLine(std::ifstream &file) {
    std::string out;
    char buffer;
    file.get(buffer);
    for (; buffer == ' '; file.get(buffer));
    for (; buffer != '\n'; file.get(buffer)) {
        if (buffer == '\r') {
            file.get(buffer);
            break;
        }
        out += buffer;
    }
    return out;
}

int imin(int i1, int i2) {
    return i1 < i2 ? i1 : i2;
}

int imax(int i1, int i2) {
    return i1 > i2 ? i1 : i2;
}
