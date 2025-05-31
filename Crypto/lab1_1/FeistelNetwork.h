#pragma once
#include"CryptoInterfaces.h"
#include <vector>
#include <memory>
#include <cstdint>

class FeistelNetwork : public ICrypt {
protected:
    std::unique_ptr<IEncryptConversion> encryptConversion;
    std::unique_ptr<IExpandKey> expandKey;
    std::vector<std::vector<uint8_t>> rKeys;
    int rounds;
    int blockLength = 4;

public:
    FeistelNetwork(std::unique_ptr<IEncryptConversion> encryptConversion,
        std::unique_ptr<IExpandKey> expandKey,
        int rounds);

    ICrypt* setKey(std::vector<uint8_t>& key) override;
    virtual std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) override;
    virtual std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) override;
    virtual int getBlockLength() override;
};