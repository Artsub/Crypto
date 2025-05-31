#pragma once
#include<vector>
#include <algorithm>
#include <random>
#include<memory>
enum class Pudding {
    Zeros,
    ANSIX923,
    PKCS7,
    ISO10126
};

class IPadding {
public:
    virtual std::vector<uint8_t>  makePadding(std::vector<uint8_t>& block, int size) = 0;

    virtual std::vector<uint8_t>  undoPadding(std::vector<uint8_t>& block) = 0;
    virtual ~IPadding() = default;
};

class ANSIX923Padding : public IPadding {
public:
    ANSIX923Padding() = default;
    std::vector<uint8_t>  makePadding(std::vector<uint8_t>& block, int size) {
        size_t n = block.size();
        int lengthPadding = (n % size == 0) ? 0 : size - (n % size);
        if (!lengthPadding)
            return block;
        std::vector<uint8_t> result(n + lengthPadding);
        std::copy(block.begin(), block.begin() + n, result.begin());
        result[n + lengthPadding - 1] = static_cast<uint8_t>(lengthPadding);
        return result;
    }


    std::vector<uint8_t>  undoPadding(std::vector<uint8_t>& block) {
        uint8_t padding_size = block.back();
        std::vector<uint8_t> result(block.begin(), block.end() - padding_size);
        return result;
    }

};
class ZerozPadding : public IPadding {
public:
    ZerozPadding() = default;
    std::vector<uint8_t>  makePadding(std::vector<uint8_t>& block, int size) {
        size_t n = block.size();
        int lengthPadding = (n % size == 0) ? 0 : size - (n % size);
        if (!lengthPadding)
            return block;
        std::vector<uint8_t> result(n + lengthPadding);
        std::copy(block.begin(), block.begin() + n, result.begin());
        return result;
    }

    std::vector<uint8_t>  undoPadding(std::vector<uint8_t>& block) {
        size_t i = block.size() - 1;
        while (block[i] == 0) {
            i--;
        }
        return std::vector<uint8_t>(block.begin(), block.begin() + i + 1);
    }

};

class PKCS7Padding : public IPadding {
public:
    PKCS7Padding() = default;
    std::vector<uint8_t>  makePadding(std::vector<uint8_t>& block, int size) {
        size_t n = block.size();
        int lengthPadding = (n % size == 0) ? 0 : size - (n % size);
        if (!lengthPadding)
            return block;
        std::vector<uint8_t> result(n + lengthPadding);
        std::copy(block.begin(), block.begin() + n, result.begin());
        std::fill(result.begin() + n, result.end(), static_cast<uint8_t>(lengthPadding));
        return result;
    }

    std::vector<uint8_t>  undoPadding(std::vector<uint8_t>& block) {
        uint8_t padding_size = block.back();
        return std::vector<uint8_t>(block.begin(), block.end() - padding_size);
    }

};

class ISO10126Padding : public IPadding {
public:
    ISO10126Padding() = default;
    std::vector<uint8_t> makePadding(std::vector<uint8_t>& block, int size) {
        static bool seeded = false;
        if (!seeded) {
            std::srand(static_cast<unsigned>(std::time(nullptr)));
            seeded = true;
        }
        size_t n = block.size();
        int lengthPadding = (n % size == 0) ? 0 : size - (n % size);
        if (!lengthPadding)
            return block;
        std::vector<uint8_t> result(n + lengthPadding);

        std::copy(block.begin(), block.end(), result.begin());

        for (size_t i = n; i < result.size() - 1; ++i) {
            result[i] = static_cast<uint8_t>(std::rand() % 256);
        }

        result.back() = static_cast<uint8_t>(lengthPadding);

        return result;
    }

    std::vector<uint8_t>  undoPadding(std::vector<uint8_t>& block) {
        uint8_t padding_size = block.back();
        return std::vector<uint8_t>(block.begin(), block.end() - padding_size);
    }

};

std::unique_ptr<IPadding> getPadding(Pudding pudding) {
    switch (pudding) {
    case(Pudding::Zeros):
        return  std::make_unique<ZerozPadding>();
        break;

    case(Pudding::ANSIX923):
        return std::make_unique<ANSIX923Padding>();
        break;

    case(Pudding::PKCS7):
        return std::make_unique<PKCS7Padding>();

        break;
    case(Pudding::ISO10126):
        return std::make_unique<ISO10126Padding>();
        break;

    default:
        throw std::exception("Padding doesnt exist");
    }

}