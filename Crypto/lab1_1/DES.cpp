#include "DES.h"
#include "Operations.h" 
#include "DESConfig.h"

int DESExpandKey::makeC(const std::vector<uint8_t>& pKey) {
    return ((pKey[0] & 0xFF) << 20) |
        ((pKey[1] & 0xFF) << 12) |
        ((pKey[2] & 0xFF) << 4) |
        ((pKey[3] & 0xFF) >> 4);
}

int DESExpandKey::makeD(const std::vector<uint8_t>& pKey) {
    return ((pKey[3] & 0x0F) << 24) |
        ((pKey[4] & 0xFF) << 16) |
        ((pKey[5] & 0xFF) << 8) |
        (pKey[6] & 0xFF);
}

std::vector<uint8_t> DESExpandKey::makeCD(int C, int D) {
    uint64_t CD = (static_cast<uint64_t>(C) << halfKeySize) | D;
    std::vector<uint8_t> bytes(7);
    for (int j = 0; j < 7; ++j) {
        bytes[j] = static_cast<uint8_t>((CD >> ((6 - j) * 8)) & 0xFF);
    }
    return bytes;
}

std::vector<std::vector<uint8_t>> DESExpandKey::expand(const std::vector<uint8_t>& key) {
    std::vector<std::vector<uint8_t>> keys(rounds);
    std::vector<uint8_t> permuteKey = permuteBits(key, PC_1);
    int c = makeC(permuteKey);
    int d = makeD(permuteKey);

    for (int i = 0; i < rounds; i++) {
        c = leftCycleShift(CYCLE_SHIFTS[i], c, halfKeySize);
        d = leftCycleShift(CYCLE_SHIFTS[i], d, halfKeySize);
        auto CD = makeCD(c, d);
        keys[i] = permuteBits(CD, PC_2);
    }

    return keys;
}

std::vector<uint8_t> DESEncryptConversion::encode(const std::vector<uint8_t>& data, std::vector<uint8_t>& rkey) {
    std::vector<uint8_t> result;
    result = permuteBits(data, P_BLOCK_EXPAND);
    result = xorBits(result, rkey);
    result = substitution(result);
    result = permuteBits(result, P_BLOCK_PLAIN); 
    return result;
}

DESEncryptor::DESEncryptor()
    : FeistelNetwork(std::make_unique<DESEncryptConversion>(), std::make_unique<DESExpandKey>(), 16) {
    blockLength = 8;
}

std::vector<uint8_t> DESEncryptor::encrypt(std::vector<uint8_t> data) {
    auto initialPermutedBlock = permuteBits(data, INITIAL_PERMUTATION);
    auto encryptedBlock = FeistelNetwork::encrypt(initialPermutedBlock);
    return permuteBits(encryptedBlock, FINAL_PERMUTATION);
}

std::vector<uint8_t> DESEncryptor::decrypt(std::vector<uint8_t> data) {
    auto initialPermutedBlock = permuteBits(data, INITIAL_PERMUTATION);
    auto decryptedBlock = FeistelNetwork::decrypt(initialPermutedBlock);
    return permuteBits(decryptedBlock, FINAL_PERMUTATION);
}

