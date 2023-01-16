#include <iostream>
#include <fstream>
#include <istream>
#include <vector>

const int NAL_UNIT_TYPE_MASK = 0x1F;

std::vector<unsigned char> readFile(const char* fileName) {
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    std::vector<unsigned char> buffer;

    if (file.is_open()) {
        size_t fileSize = file.tellg();
        buffer.resize(fileSize);

        file.seekg(0, std::ios::beg);
        file.read((char*) buffer.data(), fileSize);

        file.close();
    } else {
        std::cerr << "Error: Unable to open file '" << fileName << "'" << std::endl;
    }

    return buffer;
}

void printNalType (int nalUnitType) {

    switch (nalUnitType) {
        case 1:
            std::cout << "Coded slice a of an picture" << std::endl;
            break;
        case 6:
            std::cout << "Supplemental enhancement information" << std::endl;
            break;
        case 7:
            std::cout << "Sequence parameter set" << std::endl;
            break;
        case 8:
            std::cout << "Picture parameter set" << std::endl;
            break;
    }

}

std::vector<unsigned char> sliceBuffer(std::vector<unsigned char> buffer, uint start, uint end) {
    if (start > buffer.size() || end > buffer.size()) {
        throw std::out_of_range("start and end positions must be within the buffer size");
    }
    if (start > end) {
        throw std::invalid_argument("start position must be less than end position");
    }
    return std::vector<unsigned char>(buffer.begin() + start, buffer.begin() + end);
}

void printBufferHex(std::vector<unsigned char> buffer) {
    for (unsigned char data : buffer) {
        std::cout << std::hex << (int)data << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No input file specified" << std::endl;
        return -1;
    }

    const char* fileName = argv[1];
    std::vector<unsigned char> buffer = readFile(fileName);


    uint pos = 0;
    uint start = 0;
    bool firstNalUnit = true;

    while (pos < buffer.size()) {
        if (buffer[pos] != 0 || buffer[pos + 1] != 0 || buffer[pos + 2] != 0 || buffer[pos + 3] != 1) {
            std::cerr << "Error: Invalid NAL unit start code" << std::endl;
            return -1;
        }

        int nalUnitType = buffer[pos + 4] & NAL_UNIT_TYPE_MASK;

        printNalType(nalUnitType);

        if(nalUnitType == 7 && firstNalUnit){
        start = pos;
        firstNalUnit = false;

        }
            if(nalUnitType == 7 && !firstNalUnit){
            
            std::vector<unsigned char> frame = sliceBuffer(buffer, start, pos);
            printBufferHex(frame);
        }


        pos += 5; // skip start code and NAL unit type
        while (pos < buffer.size() && !(buffer[pos] == 0 && buffer[pos + 1] == 0 && buffer[pos + 2] == 0 && buffer[pos + 3] == 1)) {
            pos++;
        }
    }

    return 0;
}
