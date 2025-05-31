#include "FeistelNetwork.h"
#include "Operations.h"

FeistelNetwork::FeistelNetwork(std::unique_ptr<IEncryptConversion> encryptConversion,
    std::unique_ptr<IExpandKey> expandKey,
    int rounds)
    : encryptConversion(std::move(encryptConversion)),
    expandKey(std::move(expandKey)),
    rounds(rounds) {
}

ICrypt* FeistelNetwork::setKey(std::vector<uint8_t>& key) {
    rKeys = expandKey->expand(key);
    return this;
}

std::vector<uint8_t> FeistelNetwork::encrypt(const std::vector<uint8_t>& data) {
    size_t half = data.size() / 2;
    std::vector<uint8_t> left(data.begin(), data.begin() + half);
    std::vector<uint8_t> right(data.begin() + half, data.end());

    for (int i = 0; i < rounds - 1; i++) {
        auto tmp = xorBits(left, encryptConversion->encode(right, rKeys[i]));
        left = right;
        right = tmp;
    }

    left = xorBits(left, encryptConversion->encode(right, rKeys[rounds - 1]));

    std::vector<uint8_t> encryptedBlock(data.size());
    std::copy(left.begin(), left.end(), encryptedBlock.begin());
    std::copy(right.begin(), right.end(), encryptedBlock.begin() + half);
    return encryptedBlock;
}

std::vector<uint8_t> FeistelNetwork::decrypt(const std::vector<uint8_t>& data) {
    size_t half = data.size() / 2;
    std::vector<uint8_t> left(data.begin(), data.begin() + half);
    std::vector<uint8_t> right(data.begin() + half, data.end());

    left = xorBits(left, encryptConversion->encode(right, rKeys[rounds - 1]));
    for (int i = rounds - 2; i >= 0; --i) {
        auto tmp = xorBits(right, encryptConversion->encode(left, rKeys[i]));
        right = left;
        left = tmp;
    }

    std::vector<uint8_t> decryptedBlock(data.size());
    std::copy(left.begin(), left.end(), decryptedBlock.begin());
    std::copy(right.begin(), right.end(), decryptedBlock.begin() + half);
    return decryptedBlock;
}

int FeistelNetwork::getBlockLength() {
    return blockLength;
}