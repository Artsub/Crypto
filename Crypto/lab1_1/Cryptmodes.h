#pragma once
#include"CryptoInterfaces.h"
#include"Operations.h"
#include<memory>
#include<vector>
enum class CryptoMode {
    ECB,
    CBC,
    PCBC,
    CFB,
    OFB,
    CTR,
    RandomDelta
};

class AEncryptMode {
protected:
    ICrypt* encryptor;
    int lengthBlock;
    std::vector<uint8_t> IV;
public:
    AEncryptMode(ICrypt* enc, int blockLen, const std::vector<uint8_t>& iv)
        : encryptor(enc), lengthBlock(blockLen), IV(iv) {
    }
    virtual std::vector<uint8_t> encrypt(std::vector<uint8_t> data) = 0;
    virtual std::vector<uint8_t> decrypt(std::vector<uint8_t> data) = 0;
    ~AEncryptMode() = default;
};

class CFBEncryptMode : public AEncryptMode {
public:
    CFBEncryptMode(ICrypt* enc, int blockLen, const std::vector<uint8_t>& iv)
        : AEncryptMode(enc, blockLen, iv) {}

    std::vector<uint8_t> encrypt(std::vector<uint8_t> data) override{
        std::vector<uint8_t> result(data.size());
        std::vector<uint8_t> prev = IV;
        for (int i = 0; i < data.size() / lengthBlock; ++i) {
            size_t offset = i * lengthBlock;
            std::vector<uint8_t> block(data.begin() + offset, data.begin() + offset + lengthBlock);
            std::vector<uint8_t> encryptedBlock;
            try {
                 encryptedBlock = xorBits(block, encryptor->encrypt(prev));
            }
            catch (std::exception err) {
                std::cerr << err.what();
            }
            std::copy(encryptedBlock.begin(), encryptedBlock.end(), result.begin() + offset);
            prev = encryptedBlock;
        }
        return result;
    }

    std::vector<uint8_t> decrypt(std::vector<uint8_t> data) override {
        std::vector<uint8_t> result(data.size());
        std::vector<uint8_t> prev = IV;
        for (size_t i = 0; i < data.size() / lengthBlock; ++i) {
            size_t offset = i * lengthBlock;
            std::vector<uint8_t> block(data.begin() + offset, data.begin() + offset + lengthBlock);
            std::vector<uint8_t> tmp = block;
            std::vector<uint8_t> decryptedBlock = xorBits(block, encryptor->encrypt(prev));
            std::copy(decryptedBlock.begin(), decryptedBlock.end(), result.begin() + offset);
            prev = tmp;
        }
        return result;
    }

};

class ECBEncryptMode : public AEncryptMode {
public:
    ECBEncryptMode(ICrypt* enc, int blockLen, std::vector<uint8_t>&)
        : AEncryptMode(enc, blockLen, {}) {
    }

    std::vector<uint8_t> encrypt(std::vector<uint8_t> data) override {
        std::vector<uint8_t> result(data.size());
        for (int i = 0; i < data.size() / lengthBlock; ++i) {
            size_t offset = i * lengthBlock;
            std::vector<uint8_t> block(data.begin() + offset, data.begin() + offset + lengthBlock);
            std::vector<uint8_t> encryptedBlock;
            
            encryptedBlock = encryptor->encrypt(block);
            
            std::copy(encryptedBlock.begin(), encryptedBlock.end(), result.begin() + offset);
        }
        return result;
    }

    std::vector<uint8_t> decrypt(std::vector<uint8_t> data)override {
        std::vector<uint8_t> result(data.size());
        for (size_t i = 0; i < data.size() / lengthBlock; ++i) {
            size_t offset = i * lengthBlock;
            std::vector<uint8_t> block(data.begin() + offset, data.begin() + offset + lengthBlock);
            std::vector<uint8_t> decryptedBlock = encryptor->decrypt(block);
            std::copy(decryptedBlock.begin(), decryptedBlock.end(), result.begin() + offset);
        }
        return result;
    }

};

class CBCEncryptMode : public AEncryptMode {
public:
    CBCEncryptMode(ICrypt* enc, int blockLen, const std::vector<uint8_t>& iv)
        : AEncryptMode(enc, blockLen, iv) {
    }


private:
    void decryptProcess(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, size_t i) {
        std::vector<uint8_t> prev;
        if (i == 0) {
            prev = IV;
        }
        else {
            size_t prevOffset = (i - 1) * lengthBlock;
            prev.assign(input.begin() + prevOffset, input.begin() + prevOffset + lengthBlock);
        }

        size_t offset = i * lengthBlock;
        std::vector<uint8_t> block(input.begin() + offset, input.begin() + offset + lengthBlock);
        std::vector<uint8_t> decrypted = encryptor->decrypt(block);
        std::vector<uint8_t> xored = xorBits(decrypted, prev);
        std::copy(xored.begin(), xored.end(), output.begin() + offset);
    }

public:

    std::vector<uint8_t> encrypt(std::vector<uint8_t> data) override {
        std::vector<uint8_t> result(data.size());
        std::vector<uint8_t> prev = IV;
        size_t blocksCount = data.size() / lengthBlock;

        for (size_t i = 0; i < blocksCount; ++i) {
            size_t offset = i * lengthBlock;
            std::vector<uint8_t> block(data.begin() + offset, data.begin() + offset + lengthBlock);
            std::vector<uint8_t> xored = xorBits(block, prev);
            std::vector<uint8_t> encryptedBlock = encryptor->encrypt(xored);
            std::copy(encryptedBlock.begin(), encryptedBlock.end(), result.begin() + offset);
            prev = encryptedBlock;
        }

        return result;
    }
    std::vector<uint8_t> decrypt(std::vector<uint8_t> data) override {
        std::vector<uint8_t> result(data.size());
        size_t blocksCount = data.size() / lengthBlock;

        for (size_t i = 0; i < blocksCount; ++i) {
            decryptProcess(data, result, i);
        }

        return result;
    }
};

class PCBCEncryptMode : public AEncryptMode {
public:
    PCBCEncryptMode(ICrypt* enc, int blockLen, const std::vector<uint8_t>& iv)
        : AEncryptMode(enc, blockLen, iv) {
    }

    std::vector<uint8_t> encrypt(std::vector<uint8_t> data) override {
        std::vector<uint8_t> result(data.size());
        std::vector<uint8_t> xorBlock = IV;

        for (size_t i = 0; i < data.size() / lengthBlock; ++i) {
            size_t offset = i * lengthBlock;

            std::vector<uint8_t> block(data.begin() + offset, data.begin() + offset + lengthBlock);
            std::vector<uint8_t> xorInput = xorBits(block, xorBlock);
            std::vector<uint8_t> encryptedBlock;

            try {
                encryptedBlock = encryptor->encrypt(xorInput);
            }
            catch (const std::exception& err) {
                std::cerr << err.what() << std::endl;
                continue;
            }

            std::copy(encryptedBlock.begin(), encryptedBlock.end(), result.begin() + offset);
            xorBlock = xorBits(encryptedBlock, block);
        }

        return result;
    }

    std::vector<uint8_t> decrypt(std::vector<uint8_t> data) override {
        std::vector<uint8_t> result(data.size());
        std::vector<uint8_t> xorBlock = IV;

        for (size_t i = 0; i < data.size() / lengthBlock; ++i) {
            size_t offset = i * lengthBlock;

            std::vector<uint8_t> block(data.begin() + offset, data.begin() + offset + lengthBlock);
            std::vector<uint8_t> decryptedBlock;

            try {
                decryptedBlock = encryptor->decrypt(block);
            }
            catch (const std::exception& err) {
                std::cerr << err.what() << std::endl;
                continue;
            }

            std::vector<uint8_t> plainBlock = xorBits(decryptedBlock, xorBlock);
            std::copy(plainBlock.begin(), plainBlock.end(), result.begin() + offset);
            xorBlock = xorBits(block, plainBlock);
        }

        return result;
    }
};

class CTREncryptMode : public AEncryptMode {
public:
    CTREncryptMode(ICrypt* enc, int blockLen, const std::vector<uint8_t>& iv)
        : AEncryptMode(enc, blockLen, iv) {
    }

private:
    void process(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int i) {
        size_t offset = i * lengthBlock;
        std::vector<uint8_t> block(input.begin() + offset, input.begin() + offset + lengthBlock);

        std::vector<uint8_t> processBlock(lengthBlock);
        int lengthHalf = lengthBlock / 2;

        // Копируем первую половину IV
        std::copy(IV.begin(), IV.begin() + lengthHalf, processBlock.begin());

        // Формируем счетчик в байтах (big-endian)
        for (int j = 0; j < lengthHalf; ++j) {
            processBlock[lengthHalf + j] = static_cast<uint8_t>((i >> ((lengthHalf - 1 - j) * 8)) & 0xFF);
        }

        // Шифруем блок с IV и счетчиком
        std::vector<uint8_t> encryptedCounter;
        try {
            encryptedCounter = encryptor->encrypt(processBlock);
        }
        catch (const std::exception& err) {
            std::cerr << err.what() << std::endl;
            return;
        }

        // XOR с блоком данных
        std::vector<uint8_t> processedBlock = xorBits(block, encryptedCounter);

        // Копируем результат в выходной буфер
        std::copy(processedBlock.begin(), processedBlock.end(), output.begin() + offset);
    }

public:
    std::vector<uint8_t> encrypt(std::vector<uint8_t> data) override {
        std::vector<uint8_t> result(data.size());
        size_t blocksCount = data.size() / lengthBlock;

        for (size_t i = 0; i < blocksCount; ++i) {
            process(data, result, static_cast<int>(i));
        }

        return result;
    }

    std::vector<uint8_t> decrypt( std::vector<uint8_t> data) override {
        // Для CTR режим шифрование и расшифровка идентичны
        return encrypt(data);
    }
};

class RandomDeltaEncryptMode : public AEncryptMode {
private:
    uint64_t init;
    uint64_t delta = 1;

    void processEncrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int i) {
        uint64_t initCurr = init + delta * i;
        size_t offset = i * lengthBlock;

        std::vector<uint8_t> block(input.begin() + offset, input.begin() + offset + lengthBlock);

        std::vector<uint8_t> initCurrBytes = uint64ToBytes(initCurr);

        // XOR первых 8 байт блока с initCurrBytes
        for (size_t j = 0; j < 8 && j < block.size(); ++j) {
            block[j] ^= initCurrBytes[j];
        }

        std::vector<uint8_t> encryptedBlock = encryptor->encrypt(block);

        std::copy(encryptedBlock.begin(), encryptedBlock.end(), output.begin() + offset);
    }

    void processDecrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int i) {
        uint64_t initCurr = init + delta * i;
        size_t offset = i * lengthBlock;

        std::vector<uint8_t> block(input.begin() + offset, input.begin() + offset + lengthBlock);

        std::vector<uint8_t> decryptedBlock = encryptor->decrypt(block);

        std::vector<uint8_t> initCurrBytes = uint64ToBytes(initCurr);

        // XOR первых 8 байт с initCurrBytes
        for (size_t j = 0; j < 8 && j < decryptedBlock.size(); ++j) {
            decryptedBlock[j] ^= initCurrBytes[j];
        }

        std::copy(decryptedBlock.begin(), decryptedBlock.end(), output.begin() + offset);
    }

public:
    RandomDeltaEncryptMode(ICrypt* enc, int blockLen, const std::vector<uint8_t>& iv)
        : AEncryptMode(enc, blockLen, iv)
    {
        init = bytesToUint64({ iv.begin(), iv.begin() + 8 });
    }

    std::vector<uint8_t> encrypt(const std::vector<uint8_t> data) override {
        if (data.size() % lengthBlock != 0) {
            throw std::invalid_argument("Data size must be multiple of block length");
        }
        std::vector<uint8_t> result(data.size());

        size_t blocksCount = data.size() / lengthBlock;
        for (int i = 0; i < static_cast<int>(blocksCount); ++i) {
            processEncrypt(data, result, i);
        }
        return result;
    }

    std::vector<uint8_t> decrypt(const std::vector<uint8_t> data) override {
        if (data.size() % lengthBlock != 0) {
            throw std::invalid_argument("Data size must be multiple of block length");
        }
        std::vector<uint8_t> result(data.size());

        size_t blocksCount = data.size() / lengthBlock;
        for (int i = 0; i < static_cast<int>(blocksCount); ++i) {
            processDecrypt(data, result, i);
        }
        return result;
    }
};


class OFBEncryptMode : public AEncryptMode {
public:
    OFBEncryptMode(ICrypt* enc, int blockLen, const std::vector<uint8_t>& iv)
        : AEncryptMode(enc, blockLen, iv) {
    }

    std::vector<uint8_t> encrypt(std::vector<uint8_t> data) override {
        std::vector<uint8_t> result(data.size());
        std::vector<uint8_t> prev = IV;
        for (size_t i = 0; i < data.size() / lengthBlock; ++i) {
            size_t startIndex = i * lengthBlock;
            std::vector<uint8_t> block(data.begin() + startIndex, data.begin() + startIndex + lengthBlock);
            std::vector<uint8_t> encryptedPart = encryptor->encrypt(prev);
            std::vector<uint8_t> encryptedBlock = xorBits(block, encryptedPart);
            std::copy(encryptedBlock.begin(), encryptedBlock.end(), result.begin() + startIndex);
            prev = encryptedPart;
        }
        return result;
    }

    std::vector<uint8_t> decrypt(std::vector<uint8_t> data) override {
        // В OFB режиме шифрование и дешифрование одинаковы
        return encrypt(data);
    }
};


std::unique_ptr<AEncryptMode> getMode(CryptoMode mode,
                                       ICrypt* encryptor,
                                      std::vector<uint8_t>& InitializationVector)
{
    int size = encryptor->getBlockLength();
    switch (mode) {
    case CryptoMode::ECB:
        return std::make_unique<ECBEncryptMode>(encryptor, size, InitializationVector);
    
    case CryptoMode::CBC:
        return std::make_unique<CBCEncryptMode>(encryptor, size, InitializationVector);
    case CryptoMode::PCBC:
        return std::make_unique<PCBCEncryptMode>(encryptor, size, InitializationVector);
    case CryptoMode::CFB:
        return std::make_unique<CFBEncryptMode>( encryptor, size, InitializationVector);
    case CryptoMode::OFB:
        return std::make_unique<OFBEncryptMode>(encryptor, size, InitializationVector);
    case CryptoMode::CTR:
        return std::make_unique<CTREncryptMode>(encryptor, size, InitializationVector);
    case CryptoMode::RandomDelta:
        return std::make_unique<RandomDeltaEncryptMode>(encryptor, size, InitializationVector);
    default:
        throw std::exception("cryptmode doesn't exist");
    }
}