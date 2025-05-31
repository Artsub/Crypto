#pragma once
#include<vector>
#include<memory>
#include"FeistelNetwork.h"

class DESExpandKey : public IExpandKey {
private:
    int rounds = 16;
    int halfKeySize = 28;

protected:
    int makeC(const std::vector<uint8_t>& pKey);
    int makeD(const std::vector<uint8_t>& pKey);
    std::vector<uint8_t> makeCD(int C, int D);
    std::vector<std::vector<uint8_t>> expand(const std::vector<uint8_t>& key) override;
};

class DESEncryptConversion : public IEncryptConversion {
public:
    std::vector<uint8_t> encode(const std::vector<uint8_t>& data, std::vector<uint8_t>& rkey) override;
};

class DESEncryptor : public FeistelNetwork {
public:
    DESEncryptor();

    std::vector<uint8_t> encrypt(std::vector<uint8_t> data);
    std::vector<uint8_t> decrypt(std::vector<uint8_t> data);
};
