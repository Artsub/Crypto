#pragma once
#include<vector>
class IExpandKey {
public:
    virtual std::vector<std::vector<uint8_t>> expand(const std::vector<uint8_t>& key) = 0;
    virtual ~IExpandKey() = default;
};


class IEncryptConversion {
public:
    virtual std::vector<uint8_t> encode(const std::vector<uint8_t>& data, std::vector<uint8_t>& rkey) = 0;
    virtual ~IEncryptConversion() = default;
};

class ICrypt {
public:
    virtual std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) = 0;
    virtual ICrypt* setKey(std::vector<uint8_t>& key) = 0;
    virtual int getBlockLength() = 0;
    virtual ~ICrypt() = default;
};