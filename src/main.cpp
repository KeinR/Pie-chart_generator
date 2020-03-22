#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <vector>
#include <cmath>

#include <stb/stb_image_write.h>
#include <stb/stb_truetype.h>

#include "main.h"

int margin = 10;
int radius = 100;
std::string out = "piechart.jpg";
std::string title;
int titleSize = 10;
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
            file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
            file.open(argv[i], std::ios::binary);

            int lineNumber = 1;
            char buffer;
            while (!file.eof()) {
                // Each line
                file.get(buffer);
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
                        for (int i = 0; i < command.length(); i++) {
                            if (command[i] >= 'A' && command[i] <= 'Z') {
                                command[i] += 0x20; // Or 32 in decimal- I find it easier to remember "20" though
                            }
                        }
                        if (command == "out") {
                            out = readWord(file, lineNumber, buffer);
                        } else if (command == "title") {
                            title = readWord(file, lineNumber, buffer);
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
                        // file.get(buffer);
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
                                cont = false; // break; works also
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
            std::cerr << "Failed to read from config file \"" << argv[i] << "\": " << e.what() << std::endl;
        }
    }

    std::cout << "margin: " << margin << std::endl;
    std::cout << "radius: " << radius << std::endl;
    std::cout << "out: " << out << std::endl;
    std::cout << "title: " << title << std::endl;
    std::cout << "titleSize: " << titleSize << std::endl;
    std::cout << "printPercent: " << printPercent << std::endl;

    std::cout << "-----------" << std::endl;

    for (int i = 0; i < dat.size(); i++) {
        std::cout << ">>>>>>>>>>>> name: " << dat[i].name << std::endl;
        std::cout << "value: " << dat[i].value << std::endl;
        std::cout << "red: " << (int)dat[i].RBG[0] << std::endl;
        std::cout << "blue: " << (int)dat[i].RBG[1] << std::endl;
        std::cout << "green: " << (int)dat[i].RBG[2] << std::endl;
    }
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
