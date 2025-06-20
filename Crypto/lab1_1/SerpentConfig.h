#pragma once
#include<vector>
#include <stdexcept>
inline std::vector<uint16_t> IP_TABLE = {
          0,  32, 64,  96,  1, 33, 65,  97,  2, 34, 66,  98,  3, 35, 67,  99,
          4,  36, 68, 100,  5, 37, 69, 101,  6, 38, 70, 102,  7, 39, 71, 103,
          8,  40, 72, 104,  9, 41, 73, 105, 10, 42, 74, 106, 11, 43, 75, 107,
          12, 44, 76, 108, 13, 45, 77, 109, 14, 46, 78, 110, 15, 47, 79, 111,
          16, 48, 80, 112, 17, 49, 81, 113, 18, 50, 82, 114, 19, 51, 83, 115,
          20, 52, 84, 116, 21, 53, 85, 117, 22, 54, 86, 118, 23, 55, 87, 119,
          24, 56, 88, 120, 25, 57, 89, 121, 26, 58, 90, 122, 27, 59, 91, 123,
          28, 60, 92, 124, 29, 61, 93, 125, 30, 62, 94, 126, 31, 63, 95, 127
};

inline std::vector<uint16_t> FP_TABLE = {
          0,  4,  8, 12, 16, 20, 24, 28, 32,  36,  40,  44,  48,  52,  56,  60,
          64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124,
          1,  5,  9, 13, 17, 21, 25, 29, 33,  37,  41,  45,  49,  53,  57,  61,
          65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125,
          2,  6, 10, 14, 18, 22, 26, 30, 34,  38,  42,  46,  50,  54,  58,  62,
          66, 70, 74, 78, 82, 86, 90, 94, 98, 102, 106, 110, 114, 118, 122, 126,
          3,  7, 11, 15, 19, 23, 27, 31, 35,  39,  43,  47,  51,  55,  59,  63,
          67, 71, 75, 79, 83, 87, 91, 95, 99, 103, 107, 111, 115, 119, 123, 127
};

inline std::vector<std::vector<uint8_t>> S_BOX = {
          { 3, 8, 15, 1, 10, 6, 5, 11, 14, 13, 4, 2, 7, 0, 9, 12 },
          { 15, 12, 2, 7, 9, 0, 5, 10, 1, 11, 14, 8, 6, 13, 3, 4 },
          { 8, 6, 7, 9, 3, 12, 10, 15, 13, 1, 14, 4, 0, 11, 5, 2 },
          { 0, 15, 11, 8, 12, 9, 6, 3, 13, 1, 2, 4, 10, 7, 5, 14 },
          { 1, 15, 8, 3, 12, 0, 11, 6, 2, 5, 4, 10, 9, 14, 7, 13 },
          { 15, 5, 2, 11, 4, 10, 9, 12, 0, 3, 14, 8, 13, 6, 7, 1 },
          { 7, 2, 12, 5, 8, 4, 6, 11, 14, 9, 1, 15, 13, 3, 10, 0 },
          { 1, 13, 15, 0, 14, 8, 2, 11, 7, 4, 12, 10, 9, 3, 5, 6 }
};

inline std::vector<std::vector<uint8_t>> INVERSE_S_BOX = {
          { 13, 3, 11, 0, 10, 6, 5, 12, 1, 14, 4, 7, 15, 9, 8, 2 },
          { 5, 8, 2, 14, 15, 6, 12, 3, 11, 4, 7, 9, 1, 13, 10, 0 },
          { 12, 9, 15, 4, 11, 14, 1, 2, 0, 3, 6, 13, 5, 8, 10, 7 },
          { 0, 9, 10, 7, 11, 14, 6, 13, 3, 5, 12, 2, 4, 8, 15, 1 },
          { 5, 0, 8, 3, 10, 9, 7, 14, 2, 12, 11, 6, 4, 15, 13, 1 },
          { 8, 15, 2, 9, 4, 1, 13, 14, 11, 6, 5, 3, 7, 12, 10, 0 },
          { 15, 10, 1, 13, 5, 3, 6, 0, 4, 9, 14, 7, 2, 12, 8, 11 },
          { 3, 0, 6, 13, 9, 14, 15, 8, 5, 12, 11, 7, 10, 1, 4, 2 }
};

inline void checkIndex(int index, int bound) {
    if (index < 0 || index >= bound) {
        throw std::out_of_range("Index out of bounds");
    }
}
    inline uint8_t  getSBoxValue(int sBoxIndex, int input) {
    checkIndex(sBoxIndex, 8);
    checkIndex(input, 16);
    return S_BOX[sBoxIndex][input];
}

    inline uint8_t getInvSBoxValue(int invSBoxIndex, int input) {
    checkIndex(invSBoxIndex, 8);
    checkIndex(input, 16);
    return INVERSE_S_BOX[invSBoxIndex][input];
}

    inline std::vector<uint8_t> getSBox(int index) {
    checkIndex(index, 8);
    return S_BOX[index];
}

    inline std::vector<uint8_t> getInvSBox(int index) {
    checkIndex(index, 8);
    return INVERSE_S_BOX[index];
}


inline uint32_t applySBox(uint32_t word, int index) {
    uint32_t res = 0;
    for (int i = 0; i < 8; ++i) {
        uint32_t part = (word >> (i * 4)) & 0x0F;
        res |= static_cast<uint32_t>(getSBoxValue(index, part)) << (i * 4);
    }
    return res;
}