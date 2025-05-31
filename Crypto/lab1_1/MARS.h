#pragma once
#include "Operations.h"
#include "CryptoInterfaces.h"
#include "MARSConfig.h"

class KeyExpansion : public IExpandKey {


public:
	std::vector<std::vector<uint8_t>> expand(const std::vector<uint8_t>& key) override {
		size_t n = key.size() / 4;
		std::vector<uint32_t> T(15);
		std::vector<uint32_t> K(40);

		for (int i = 0; i < n; ++i)
		{
            std::memcpy(&T[i], &key[i * 4], 4);
		}

		T[n] = static_cast<uint32_t>(n);

        for (int j = 0; j < 4; ++j)
        {
            // linear Key-Word Expansion
            for (int i = 0; i < 15; i++)
            {
                T[i] ^= LeftRotate(T[(i + 8) % 15] ^ T[(i + 13 ) % 15], 3) ^ (uint32_t)(4 * i + j);
            }

            // S-box Based Stirring
            for (int round = 0; round < 4; round++)
            {
                for (int i = 0; i < 15; i++)
                {
                    uint32_t sIndex = T[(i + 14) % 15] & 0x1FF;
                    T[i] = LeftRotate(T[i] + S[sIndex], 9);
                }
            }

            // store next 10 key words into K
            for (int i = 0; i < 10; i++)
            {
                K[10 * j + i] = T[(4 * i) % 15];
            }
        }

        for (int i = 5; i <= 35; i += 2)
        {
            uint32_t j = K[i] & 0x3;
            uint32_t  w = K[i] | 0x3;
            uint32_t r = K[i - 1] & 0x1f;

            uint32_t p = LeftRotate(B[j], (int)r);
            uint32_t M = ComputeMask(w);

            K[i] = w ^ (p & M);
        }

        std::vector<std::vector<uint8_t>> roundKeys(40);

        for (int i = 0; i < 40; ++i) {
            std::vector<uint8_t> bytes(4);
            uint32_t value = K[i];
            bytes[0] = static_cast<uint8_t>(value & 0xFF);
            bytes[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
            bytes[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
            bytes[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
            roundKeys[i] = std::move(bytes);
        }

        return roundKeys;
	}



};


class MARS :public ICrypt {
private: 
    std::vector<std::vector<uint8_t>> rKeys;
    std::unique_ptr<IExpandKey> expandKey;
protected:

    std::tuple<uint32_t, uint32_t, uint32_t> EFunction(uint32_t A, uint32_t firstKey, uint32_t secondKey)
    {
        uint32_t R = LeftRotate(
            LeftRotate(A, 13) * firstKey,
            10);

        uint32_t M = LeftRotate(
            A + secondKey,
            (int)((R >> 5) & 0x1F));

        uint32_t  L = LeftRotate(
            S[M & 0x1FF] ^ (R >> 5) ^ R,
            (int)(R & 0x1F));

        return std::make_tuple(L, M, R);
    }

public:

    MARS() {
        expandKey = std::make_unique<KeyExpansion>();

    }

    ICrypt* setKey(std::vector<uint8_t>& key) override
    {
        rKeys = expandKey->expand(key);
        return this;
    }

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) override {
        std::vector<uint32_t> K(40);
        for (int i = 0; i < 40; ++i)
        {
            K[i] = toUInt32(rKeys[i], 0);
        }

        uint32_t A = toUInt32(data, 0) + K[0];
        uint32_t B = toUInt32(data, 4) + K[1];
        uint32_t C = toUInt32(data, 8) + K[2];
        uint32_t D = toUInt32(data, 12) + K[3];

        // Forward Mixing
        for (int i = 0; i < 8; i++)  
        {
            B = (B ^ S0[A & 0xff]) + S1[RightRotate(A, 8) & 0xff];
            C += S0[RightRotate(A, 16) & 0xff];
            D ^= S1[RightRotate(A, 24) & 0xff];

            A = RightRotate(A, 24);

            if (i == 1 || i == 5) A += B;
            else if (i == 0 || i == 4) A += D;

            int32_t tmp = D;
            D = C;
            C = B;
            B = A;
            A = tmp;
        }

        // Cryptographic core
        for (int i = 0; i < 16; i++)
        {
            uint32_t firstKey = toUInt32(rKeys[2 * i + 5], 0);
            uint32_t  secondKey = toUInt32(rKeys[2 * i + 4], 0);

            uint32_t L, M, R;
            std::tie(L, M, R) = EFunction(A, firstKey, secondKey);

            C += M;
            if (i < 8)
            {
                D += R;
                B += L;
            }
            else
            {
                D += L;
                B += R;
            }

            uint32_t temp = A;
            A = B;
            B = C;
            C = D;
            D = LeftRotate(temp, 13);
        }


        // Backward Mixing
        for (int i = 0; i < 8; i++)
        {
            if (i == 3 || i == 7) A -= B;
            if (i == 2 || i == 6) A -= D;

            B ^= S1[A & 0xff];
            C -= S0[LeftRotate(A, 8) & 0xff];
            D = (D - S1[LeftRotate(A, 16) & 0xff]) ^
                S0[LeftRotate(A, 24) & 0xff];

            uint32_t temp = A;
            A = B;
            B = C;
            C = D;
            D = LeftRotate(temp, 24);
        }

        A -= K[36];
        B -= K[37];
        C -= K[38];
        D -= K[39];

        std::vector<uint8_t> result(16);
        std::memcpy(result.data(), &A, 4);
        std::memcpy(result.data() + 4, &B, 4);
        std::memcpy(result.data() + 8, &C, 4);
        std::memcpy(result.data() + 12, &D, 4);

        return result;


    }

    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) override {
        
        std::vector<uint32_t> K(40);
        for (int i = 0; i < 40; ++i)
        {
            K[i] = toUInt32(rKeys[i], 0);
        }

        uint32_t A = toUInt32(data, 0) + K[36];
        uint32_t B = toUInt32(data, 4) + K[37];
        uint32_t C = toUInt32(data, 8) + K[38];
        uint32_t D = toUInt32(data, 12) + K[39];

        // Inverse Backward Mixing
        for (int i = 7; i >= 0; i--)
        {
            uint32_t tmp = RightRotate(D, 24);
            D = C;
            C = B;
            B = A;
            A = tmp;

            D = (D ^ S0[LeftRotate(A, 24) & 0xff]) + S1[LeftRotate(A, 16) & 0xff];
            C += S0[LeftRotate(A, 8) & 0xff];
            B ^= S1[A & 0xff];

            if (i == 3 || i == 7) A += B;
            else if (i == 2 || i == 6) A += D;
        }

        // Inverse Core Rounds
        for (int i = 15; i >= 0; i--)
        {
            uint32_t tmp = RightRotate(D, 13);
            D = C;
            C = B;
            B = A;
            A = tmp;

            uint32_t firstKey = toUInt32(rKeys[2 * i + 5], 0);
            uint32_t secondKey = toUInt32(rKeys[2 * i + 4], 0);

            uint32_t L, M, R;
            std::tie(L, M, R) = EFunction(A, firstKey, secondKey);

            if (i < 8) {
                B -= L;
                D -= R;
            }
            else {
                B -= R;
                D -= L;
            }
            C -= M;
        }

        // Inverse Forward Mixing
        for (int i = 7; i >= 0; i--)
        {
            uint32_t tmp = D;
            D = C;
            C = B;
            B = A;
            A = tmp;

            if (i == 1 || i == 5) A -= B;
            if (i == 0 || i == 4) A -= D;

            A = LeftRotate(A, 24);

            D ^= S1[RightRotate(A, 24) & 0xff];
            C -= S0[RightRotate(A, 16) & 0xff];
            B = (B - S1[RightRotate(A, 8) & 0xff]) ^ S0[A & 0xff];
        }

        A -= K[0];
        B -= K[1];
        C -= K[2];
        D -= K[3];

        std::vector<uint8_t> result(16);
        std::memcpy(result.data(), &A, 4);
        std::memcpy(result.data() + 4, &B, 4);
        std::memcpy(result.data() + 8, &C, 4);
        std::memcpy(result.data() + 12, &D, 4);

        return result;
    }

    int getBlockLength() override {
        return 16;
    }

};