#include<iostream>
#include <cstdint>
#include "Operations.h"
#include "DES.h"
#include "Paddings.h"
#include "EncryptorManager.h"
#include "Cryptmodes.h"
#include "Mars.h"
namespace tests {
    /*std::vector<uint8_t> data{ 0b00000001 };
    std::vector<uint16_t> pBlock = { 7,6,5,4,3,2,1,0 };
    auto result = permuteBits(data, pBlock);
    for (auto i : result) {
        int q = (int)i;
        std::cout << q << " ";
    }

    std::vector<uint8_t> data = { 8};
    auto padding = getPadding(Pudding::Zeros);
    auto padded = padding->makePadding(data, 8);
    for (auto& t : padded) {
        std::cout << static_cast<int>(t) << ' ';
    }
    std::cout << '\n';
    padding = getPadding(Pudding::ANSIX923);
    padded = padding->makePadding(data, 8);
    for (auto& t : padded) {
        std::cout << static_cast<int>(t) << ' ';
    }
    std::cout << '\n';
    padding = getPadding(Pudding::PKCS7);
    padded = padding->makePadding(data, 8);
    for (auto& t : padded) {
        std::cout << static_cast<int>(t) << ' ';
    }
    std::cout << '\n';
    padding = getPadding(Pudding::ISO10126);
    padded = padding->makePadding(data, 8);
    for (auto& t : padded) {
        std::cout << static_cast<int>(t) << ' ';
    }

    std::cout << '\n';
    return 0;
    */

    /*  EncryptorManager CryptoManager2(KeyDes,
        EncryptionAlgorithm::DES,
        CryptoMode::ECB,
        Pudding::ISO10126,
        DES_IV);
    std::string message2{ "I am message" };
    std::vector<uint8_t> data_bytes;
    for (auto i : message2) {
        data_bytes.push_back(i);

    }
    for (auto& t : message2) {
        std::cout << uint8_t(t);
    }
    std::cout << std::endl;
    auto cryptoText = CryptoManager2.encrypt(data_bytes);

    for (auto& t : cryptoText) {
        std::cout << uint8_t(t);
    }
    std::cout << std::endl;
    auto result = CryptoManager2.decrypt(cryptoText);

    for (auto& t : result) {
        std::cout << uint8_t(t);
    }
    std::cout << std::endl;*/
};

std::vector<uint8_t> key = {
    0x01, 0x23, 0x45, 0x67, 0x82, 0xAB, 0xCD, 0xEF,
    0x01, 0x23, 0x45, 0x67, 0x82, 0xAB, 0xCD, 0xEF
};

std::vector<uint8_t> IV = { 0x13, 0x34, 0x57, 0x79, 0x9B, 0xBC, 0xDF, 0xF1, 0x13, 0x34, 0x57, 0x79, 0x9B, 0xBC, 0xDF, 0xF1 };


int main() {
    /*вхожные данные
    std::string data;
    std::string key;
    std::string Mode;
    std::string Padding;
    std::string IV;
    */





    for (int i = 0; i < 3; i++){
        for (int j = 3; j <= 5; j++) {
        EncryptorManager CryptoManager1(key,
            EncryptionAlgorithm::MARS,
            static_cast<CryptoMode>(j),
            static_cast<Pudding>(i),
            IV);
    std::string message1{ "I am message goida goida goida" };
    std::vector<uint8_t> data_bytes;
    for (auto i : message1) {
        data_bytes.push_back(i);

    }
    for (auto& t : message1) {
        std::cout << uint8_t(t);
    }
    std::cout << std::endl;
    auto cryptoText = CryptoManager1.encrypt(data_bytes);

    for (auto& t : cryptoText) {
        std::cout << uint8_t(t);
    }
    std::cout << std::endl;
    auto result = CryptoManager1.decrypt(cryptoText);

    for (auto& t : result) {
        std::cout << uint8_t(t);
    }
    std::cout << std::endl;
    if (data_bytes == result)
        std::cout << 1;
    else
        std::cout << 0;

    std::cout << std::endl;

    std::cout << std::endl;
}}

   
    return 0;
}