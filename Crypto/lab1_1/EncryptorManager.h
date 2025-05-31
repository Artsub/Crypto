#pragma once

#include "Cryptmodes.h"
#include "Paddings.h"
#include "DES.h"
#include "MARS.h"
#include "Serpent.h"

enum class EncryptionAlgorithm {
	DES,
	DEAL,
	MARS, 
	SERPENT
};

class EncryptorManager {
private:
	std::unique_ptr<AEncryptMode> kernelMode;
	std::unique_ptr<IPadding> padding;
	int blockLength;
	ICrypt* encryptor;
public:
	EncryptorManager(std::vector<uint8_t>& key,
					EncryptionAlgorithm algorithm,
					CryptoMode mode,
					Pudding padd,
					std::vector<uint8_t>& IV){
		
		switch (algorithm) {
			case(EncryptionAlgorithm::DES):
				encryptor = new DESEncryptor();
				break;
			case(EncryptionAlgorithm::MARS):
				encryptor = new MARS();
				break;
			case(EncryptionAlgorithm::SERPENT):
				encryptor = new Serpent();
				break;
			default:
				throw std::exception("whong algorithm");
		}

		kernelMode = getMode(mode, encryptor->setKey(key), IV);
		padding = getPadding(padd);
		blockLength = encryptor->getBlockLength();
	}

	std::vector<uint8_t> encrypt(std::vector<uint8_t>& data) {
		auto dataPadding = padding->makePadding(data, blockLength);
		return kernelMode->encrypt(dataPadding);

	}
		
	std::vector<uint8_t> decrypt(std::vector<uint8_t>& ciphertext) {
		
		auto data = kernelMode->decrypt(ciphertext);
		return padding->undoPadding(data);
		
	}
	~EncryptorManager(){
		delete encryptor;
	}
};
