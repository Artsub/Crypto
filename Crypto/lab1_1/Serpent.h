#pragma once

#include "Operations.h"
#include "CryptoInterfaces.h"
#include "SerpentConfig.h"
#include "memory"

class SerpentKeyExpansion : public IExpandKey {
private:
	int32_t PHI = 0x9E3779B9;
	std::vector<int16_t> S_BOX_ORDER = { 3, 2, 1, 0, 7, 6, 5, 4 };
public:

	std::vector<std::vector<uint8_t>> expand(const std::vector<uint8_t>& key) override {
		std::vector<uint8_t> keyNew(32);
		std::copy(key.begin(), key.end(), keyNew.begin());
		if (key.size() < 32) {
			keyNew[key.size()] = 0x80;
		}

		std::vector<uint32_t> w(132);

		for (int i = 0; i < 8; ++i) {
			w[i] = static_cast<uint32_t>(keyNew[4 * i]) |
				(static_cast<uint32_t>(keyNew[4 * i + 1]) << 8) |
				(static_cast<uint32_t>(keyNew[4 * i + 2]) << 16) |
				(static_cast<uint32_t>(keyNew[4 * i + 3]) << 24);
		}

		for (int i = 8; i < 132; ++i) {
			w[i] = LeftRotate(
				w[i - 8] ^ w[i - 5] ^ w[i - 3] ^ w[i - 1] ^ PHI ^ static_cast<uint32_t>(i),
				11
			);


		}

		for (int block = 0; block < 33; ++block) {
			int sBoxIndex = S_BOX_ORDER[block % 8];
			for (int i = 0; i < 4; ++i) {
				w[block * 4 + i] = applySBox(w[block * 4 + i], sBoxIndex);
			}
		}

		std::vector<std::vector<uint8_t>> roundKeys(33, std::vector<uint8_t>(16));

		for (int i = 0; i < 33; ++i) {
			for (int j = 0; j < 4; ++j) {
				uint32_t word = w[i * 4 + j];
				roundKeys[i][j * 4 + 0] = static_cast<uint8_t>(word & 0xFF);
				roundKeys[i][j * 4 + 1] = static_cast<uint8_t>((word >> 8) & 0xFF);
				roundKeys[i][j * 4 + 2] = static_cast<uint8_t>((word >> 16) & 0xFF);
				roundKeys[i][j * 4 + 3] = static_cast<uint8_t>((word >> 24) & 0xFF);
			}
		}

		return roundKeys;

	}
};


class Serpent : public ICrypt {
private:
	std::vector<std::vector<uint8_t>> rKeys;
	std::unique_ptr<IExpandKey> expandKey;

protected:
	void applySboxes(std::vector<uint8_t>& block, int round, bool invSbox) {
		int sBoxIndex = round % 8;

		for (int group = 0; group < 32; ++group) {
			int byteIndex = group / 2;
			int pos = (group % 2) * 4;

			uint8_t part = (block[byteIndex] >> pos) & 0x0F;
			uint8_t substituted = invSbox
				? getInvSBoxValue(sBoxIndex, part)
				: getSBoxValue(sBoxIndex, part);

			block[byteIndex] = (block[byteIndex] & ~(0x0F << pos)) | (substituted << pos);
		}
	}
	std::vector<uint8_t> intToBytes(uint32_t value) {
		std::vector<uint8_t> result(4);
		result[0] = static_cast<uint8_t>(value);
		result[1] = static_cast<uint8_t>(value >> 8);
		result[2] = static_cast<uint8_t>(value >> 16);
		result[3] = static_cast<uint8_t>(value >> 24);
		return result;
	}

	std::vector<uint8_t> getBytes(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3) {
		std::vector<uint8_t> result(16);
		auto b0 = intToBytes(x0);
		auto b1 = intToBytes(x1);
		auto b2 = intToBytes(x2);
		auto b3 = intToBytes(x3);

		std::copy(b0.begin(), b0.end(), result.begin());
		std::copy(b1.begin(), b1.end(), result.begin() + 4);
		std::copy(b2.begin(), b2.end(), result.begin() + 8);
		std::copy(b3.begin(), b3.end(), result.begin() + 12);

		return result;
	}

	std::vector<uint8_t> linearTransformation(const std::vector<uint8_t>& block) {
		uint32_t x0 = toUInt32(block, 0);
		uint32_t x1 = toUInt32(block, 4);
		uint32_t x2 = toUInt32(block, 8);
		uint32_t x3 = toUInt32(block, 12);

		x0 = LeftRotate(x0, 13);
		x2 = LeftRotate(x2, 3);
		x1 ^= x0 ^ x2;
		x3 ^= x2 ^ (x0 << 3);
		x1 = LeftRotate(x1, 1);
		x3 = LeftRotate(x3, 7);
		x0 ^= x1 ^ x3;
		x2 ^= x3 ^ (x1 << 7);
		x0 = LeftRotate(x0, 5);
		x2 = LeftRotate(x2, 22);

		return getBytes(x0, x1, x2, x3);
	}

	std::vector<uint8_t> inverseLinearTransformation(const std::vector<uint8_t>& block) {
		uint32_t x0 = toUInt32(block, 0);
		uint32_t x1 = toUInt32(block, 4);
		uint32_t x2 = toUInt32(block, 8);
		uint32_t x3 = toUInt32(block, 12);

		x2 = RightRotate(x2, 22);
		x0 = RightRotate(x0, 5);
		x2 ^= x3 ^ (x1 << 7);
		x0 ^= x1 ^ x3;
		x3 = RightRotate(x3, 7);
		x1 = RightRotate(x1, 1);
		x3 ^= x2 ^ (x0 << 3);
		x1 ^= x0 ^ x2;
		x2 = RightRotate(x2, 3);
		x0 = RightRotate(x0, 13);

		return getBytes(x0, x1, x2, x3);
	}

public:
	int getBlockLength() override {
		return 16;
	}

	Serpent() {
		expandKey = std::make_unique<SerpentKeyExpansion>();
	}

	ICrypt* setKey(std::vector<uint8_t>& key) override
	{
		rKeys = expandKey->expand(key);
		return this;
	}

	std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) override {

		std::vector<uint8_t> block = permuteBits(data, IP_TABLE);

		for (int round = 0; round < 32; round++) {
			block = xorBits(block, rKeys[round]);
			applySboxes(block, round, false);
			if (round != 32 - 1) {
				block = linearTransformation(block);
			}
		}

		block = xorBits(block, rKeys[32]);
		block = permuteBits(block, FP_TABLE);

		return block;
	}

	std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext) override {
		if (ciphertext.size() != 16) {
			throw std::invalid_argument("Block length must be 16 bytes");
		}

		std::vector<uint8_t> block = permuteBits(ciphertext, IP_TABLE);

		block = xorBits(block, rKeys[32]);

		for (int round = 31; round >= 0; --round) {
			if (round != 31) {
				block = inverseLinearTransformation(block);
			}
			applySboxes(block, round, true);
			block = xorBits(block, rKeys[round]);
		}

		block = permuteBits(block, FP_TABLE);

		return block;
	}

		
};
