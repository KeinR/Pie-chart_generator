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

#define SIDEBAR_ELEMENT_MARGIN 5
#define DECLIP_MARGIN 3

// As you may or may not assume, this is the debug out stream
std::ofstream debug(DEBUG_LOCATION);

int main(int argc, char **argv) {
    // Generate prophile
    if (argc == 1) {
        std::cerr << "Must provide at least one config file" << std::endl;
        return 1;
    }

    std::set_terminate([]() -> void {
        std::cerr << "Terminate called after an uncaught exception was thrown" << std::endl;
    });

    // Defaults
    int margin = 10;
    int radius = 100;
    std::string out = "piechart.png";
    std::string title;
    int titleSize = 10;
    int descSize = 10;
    int descWrap = 50;
    bool printPercent = true;
    std::string suffix;
    std::string prefix;

    struct data {
        double value;
        double percent;
        std::string name;
        // Colors
        unsigned char RBG[3]{0};
    };

    std::vector<data> dat;

    for (int i = 1; i < argc; i++) {
        std::ifstream file;
        try {
            file.exceptions(std::ifstream::badbit); // Fucking failbit getting set by eofbit
            file.open(argv[i], std::ios::binary);
            if (!file.good()) {
                std::cerr << "ERROR: Failed to open file \"" << argv[i] << "\"" << std::endl;
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
                        } else if (command == "suffix") {
                            if (buffer == ' ') { // Perhaps not really necessary to check, but I like to be sure
                                file.get(buffer);
                            }
                            suffix = readWordAll(file, lineNumber, buffer);
                        } else if (command == "prefix") {
                            if (buffer == ' ') {
                                file.get(buffer);
                            }
                            prefix = readWordAll(file, lineNumber, buffer);
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
                                } else if (command == "descsize") {
                                    command = readWord(file, lineNumber, buffer);
                                    descSize = std::stoi(command);
                                } else if (command == "descwrap") {
                                    command = readWord(file, lineNumber, buffer);
                                    descWrap = std::stoi(command);
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

    std::function<std::string(const data&)> genDispName;
    if (printPercent) {
        if (suffix.length() == 0) {
            suffix = '%';
        }

        genDispName = [](const data &d) -> std::string {
            return formatValue(d.percent * 100);
        };
    } else {
        genDispName = [](const data &d) -> std::string {
            return formatValue(d.value);
        };
    }

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

    const int chartCenterX = radius;

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
        std::cerr << "ERROR: Failed to parse font file at \"" TTF_FONT "\"" << std::endl;
        delete[] fontBuffer;
        return 1;
    }

    int mFAscent, mFDescent, mFLineGap;
    stbtt_GetFontVMetrics(&finfo, &mFAscent, &mFDescent, &mFLineGap);

    // Determine positioning to allow for center-align

    // Title
    const float fscale = stbtt_ScaleForPixelHeight(&finfo, titleSize);
    const int fascent = mFAscent * fscale;
    // const int fdescent = mFDescent * fscale; // UNUSED ATM


    int fx = 0;
    std::vector<fontMetrics> sizeHistory;
    std::vector<fLine> lines;
    if (title.length()) {
        getStringMetrics(title, chartCenterX + radius, fscale, finfo, sizeHistory, lines);
    }

    // Sidebar
    int total = 0;
    for (std::vector<data>::size_type i = 0; i < dat.size(); i++) {
        total += dat[i].value;
    }

    const float oScale = stbtt_ScaleForPixelHeight(&finfo, descSize);
    const int oAscent = mFAscent * oScale;
    // const int oDescent = mFDescent * oScale; // UNUSED ATM

    struct sideSection {
        unsigned char *RBG;
        std::string displayString;
        std::vector<fontMetrics> metrics;
        std::vector<fLine> lines;
        int yOffset;
        int xOffset;
    };

    std::vector<sideSection> sideParts(dat.size());
    // I know having two variables that are this hard to tell appart
    // is a fucking taboo, but yolo swag you only live once
    int tyofs = 0; // Good luck guessing that abbreviated mess
    int txofs = 0; // or perhaps you can now
    int sbMostX = 0;
    for (std::vector<data>::size_type i = 0; i < dat.size(); i++) {
        dat[i].percent = dat[i].value / total;

        sideSection &ss = sideParts[i];

        ss.RBG = dat[i].RBG;

        ss.displayString += prefix;
        ss.displayString += genDispName(dat[i]);
        ss.displayString += suffix;
        ss.displayString += ' ';
        ss.displayString += dat[i].name;

        getStringMetrics(ss.displayString, descWrap, oScale, finfo, ss.metrics, ss.lines);
        const int inc = (ss.lines.size() - 1) * oAscent + descSize + SIDEBAR_ELEMENT_MARGIN;
        if (tyofs + inc > diameter) {
            txofs += descWrap + margin; // Move right
            ss.yOffset = 0;
            tyofs = inc; // Reset y-offset
            sbMostX = 0;
        } else {
            ss.yOffset = tyofs; // Set to foot of last
            tyofs += inc; // Move down
        }
        ss.xOffset = txofs;

        for (std::vector<fLine>::size_type l = 0; l < ss.lines.size(); l++) {
            if (ss.lines[l].width > sbMostX) {
                sbMostX = ss.lines[l].width;
            }
        }
    }

    // -------- font rendering intermission; create the main bitmap
    const int titleHeight = title.length() ? fascent * lines.size() + margin : 0;
                                          //- x taken up -   // Those little color boxes
    const int width = diameter + margin + txofs + sbMostX + descSize; //+ std::round(diameter * 0.07) + 
    const int height = diameter + titleHeight;

    const int chartCenterY = radius + titleHeight;

    const int bitmapSize = width * height * CHANNELS;
    uint8_t *bitmap = new uint8_t[bitmapSize]{0};

    debug << "Total bmp size in MB: " << ((double)diameter * diameter * CHANNELS / 1000000) << std::endl;

    const int movX = CHANNELS;
    const int movY = width * movX;

    // -------- end of intermission

    debug << "THrough" << std::endl;

    int verticalOffset = 0;
    std::string::size_type c = 0;

    // Title rendering
    if (titleHeight) {
        for (std::vector<std::string::size_type>::size_type l = 0; l < lines.size(); l++) {
            // Must create intermediary bitmap as stb_truetype renders in grayscale, and the output bitmap is in RGBA
            const int charbpmSize = lines[l].width * titleSize;
            uint8_t *charbpm = new uint8_t[charbpmSize]{0};

            fx = 0;
            for (; c < lines[l].cOffset; c++) {
                debug << "Vroom, c=" << c << ", l=" << l << ", cEnd=" << lines[l].cOffset << std::endl;
                // ty Justin Meiners very helpful
                // int ax;
                // int lsb;
                // stbtt_GetCodepointHMetrics(&finfo, title[c], &ax, &lsb);

                /* get bounding box for character (may be offset to account for chars that dip above or below the line */
                int c_x1, c_y1, c_x2, c_y2;
                stbtt_GetCodepointBitmapBox(&finfo, title[c], fscale, fscale, &c_x1, &c_y1, &c_x2, &c_y2);

                /* compute y (different characters have different heights */
                int y = fascent + c_y1;

                /* render character (stride and offset is important here) */
                int byteOffset = fx + sizeHistory[c].lsb + (y * lines[l].width);
                stbtt_MakeCodepointBitmap(&finfo, charbpm + byteOffset, c_x2 - c_x1, c_y2 - c_y1, lines[l].width, fscale, fscale, title[c]);

                /* advance x */
                fx += sizeHistory[c].aw;

                /* add kerning */
                fx += sizeHistory[c].kern;
            }

            // Render into output bitmap
            for (int y = 0; y < titleSize; y++) {
                for (int x = 0; x < lines[l].width; x++) {
                    const int ci = x + y * lines[l].width;
                    if (charbpm[ci] != 0) {
                        const int index = (x + lines[l].mapOffset) * movX + (y + verticalOffset) * movY;
                        bitmap[index] = charbpm[ci];
                        bitmap[index+1] = charbpm[ci];
                        bitmap[index+2] = charbpm[ci];
                        bitmap[index+3] = 0xFF;
                    }
                }
            }

            // AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAdebug
            std::string debug_fname = std::string("DEBUG.").append(std::to_string(l)).append(".bmp");
            stbi_write_bmp(debug_fname.c_str(), lines[l].width, titleSize, 1, charbpm);

            delete[] charbpm;

            verticalOffset += fascent;
            c = lines[l].cResume;
        }
    }

    // Sidebar rendering
    for (std::vector<sideSection>::size_type p = 0; p < sideParts.size(); p++) {
        debug << "Part " << p << std::endl;

        // Render lil' cube
        for (int x = diameter + margin + sideParts[p].xOffset, endX = x + descSize; x < endX; x++) {
            for (int y = titleHeight + sideParts[p].yOffset, endY = y + descSize; y < endY; y++) {
                const int index = x * movX + y * movY;
                bitmap[index] = sideParts[p].RBG[0];
                bitmap[index+1] = sideParts[p].RBG[1];
                bitmap[index+2] = sideParts[p].RBG[2];
                bitmap[index+3] = 0xFF;
            }
        }

        c = 0; // resused cuz' the name good
        verticalOffset = 0;
        std::vector<fLine> &oLines = sideParts[p].lines;
        for (std::vector<std::string::size_type>::size_type l = 0; l < oLines.size(); l++) {
            // Must create intermediary bitmap as stb_truetype renders in grayscale, and the output bitmap is in RGBA
            const int charbpmSize = oLines[l].width * descSize;
            uint8_t *charbpm = new uint8_t[charbpmSize]{0};

            fx = 0;
            for (; c < oLines[l].cOffset; c++) {
                debug << "VroomXX2, c=" << c << ", l=" << l << ", cEnd=" << oLines[l].cOffset << std::endl;
                // ty Justin Meiners very helpful

                int c_x1, c_y1, c_x2, c_y2;
                stbtt_GetCodepointBitmapBox(&finfo, sideParts[p].displayString[c], oScale, oScale, &c_x1, &c_y1, &c_x2, &c_y2);

                int y = oAscent + c_y1;

                int byteOffset = fx + sideParts[p].metrics[c].lsb + (y * oLines[l].width);
                stbtt_MakeCodepointBitmap(&finfo, charbpm + byteOffset, c_x2 - c_x1, c_y2 - c_y1, oLines[l].width, oScale, oScale, sideParts[p].displayString[c]);

                fx += sideParts[p].metrics[c].aw;

                fx += sideParts[p].metrics[c].kern;
            }

            // Render into output bitmap
            for (int y = 0; y < descSize; y++) {
                for (int x = 0; x < oLines[l].width; x++) {
                    const int ci = x + y * oLines[l].width;
                    if (charbpm[ci] != 0) {
                        const int index = (x + margin + sideParts[p].xOffset + descSize + diameter + SIDEBAR_ELEMENT_MARGIN) * movX + (y + titleHeight + verticalOffset + sideParts[p].yOffset) * movY;
                        bitmap[index] = charbpm[ci];
                        bitmap[index+1] = charbpm[ci];
                        bitmap[index+2] = charbpm[ci];
                        bitmap[index+3] = 0xFF;
                    }
                }
            }

            delete[] charbpm;

            verticalOffset += oAscent;
            c = oLines[l].cResume;
        }
    }

    delete[] fontBuffer;

    // ------------- end font loading ----------

    debug << "Enter and pry,,," << std::endl;

    const int radiusSqr = radius * radius;

    // Draw sections

    double lastSlope = std::nextafter(0.0d, 1.0d);
    double lastYIntercept = chartCenterY;
    int lastDirectionX = 1;
    int lastX = chartCenterX + radius;

    double netRads = 0;
    bool cont = true;
    for (std::vector<data>::size_type i = 0; cont; i++) {
        const double rads = dat[i].percent * 2 * PI;
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
    // Skip whitespace
    for (file.get(buffer); buffer == ' '; file.get(buffer));
    return readWordAll(file, lineNumber, buffer);
}

std::string readWordAll(std::ifstream &file, int &lineNumber, char &buffer) {
    std::string outBuffer;
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

// Do note that the font buffer must still be allocated (the one used to initialize the font),
// otherwise this function will inevitably seg fault
void getStringMetrics(const std::string &str, const int wrapWidth, const float &fontScale, const stbtt_fontinfo &fontInfo, // in
                        std::vector<fontMetrics> &metrics, std::vector<fLine> &lines) { // out
    int fx = DECLIP_MARGIN; // Stands for like "font-x" or something
    metrics.reserve(str.length());
    bool canHazSpace = false;
    for (std::string::size_type i = 0; i < str.length(); i++) {
        stbtt_GetCodepointHMetrics(&fontInfo, str[i], &metrics[i].aw, &metrics[i].lsb);
        metrics[i].aw *= fontScale;
        metrics[i].lsb *= fontScale;
        debug << "aw=" << metrics[i].aw << ", lsb=" << metrics[i].lsb << std::endl;

        fx += metrics[i].aw + metrics[i].lsb;
        if (i+1 < str.length()) {
            metrics[i].kern = stbtt_GetCodepointKernAdvance(&fontInfo, str[i], str[i+1]) * fontScale;
            fx += metrics[i].kern;
        } else {
            metrics[i].kern = 0;
        }

        if (fx > wrapWidth) {
            // Remove last char that caused overflow
            fx -= metrics[i].aw + metrics[i].kern;
            i--;
            std::string::size_type resume = i;
            // Mov to last word for soft wrap- if there's more than one word...
            if (canHazSpace) {
                debug << "Backtrack " << str << std::endl;
                canHazSpace = false;
                for (std::string::size_type newI = i; newI >= 0; newI--) {
                    if (str[newI] == ' ') { // Perhaps terminar el spaces en el futuro?
                        resume = newI+1;
                        for (; newI >= 0 && str[newI] == ' '; newI--);
                        newI++; // because it's exclusive
                        for (; i > newI; i--) {
                            fx -= metrics[i].aw + metrics[i].kern;
                        }
                        break;
                    }
                }
            }
            debug << "Pushback " << i << std::endl;
            lines.push_back({
                i,
                resume,
                (wrapWidth - fx) / 2,
                fx
            });
            i = resume-1;
            fx = DECLIP_MARGIN;
        } else if (str[i] == ' ') {
            canHazSpace = true;
        }
    }
    lines.push_back({
        str.length(),
        str.length(),
        (wrapWidth - fx) / 2,
        fx
    });
}

std::string formatValue(const double &value) {
    std::string out = std::to_string((int)std::round(value * 100)); // TODO: setting to configure accuracy?
    out.insert(out.length()-2, ".");
    for (int i = out.length()-1;; i--) {
        if (out[i] == '.') {
            out.erase(i, out.length() - i);
            break;
        } else if (out[i] != '0') {
            out.erase(i+1, out.length() - i);
        }
    }
    return out;
}
