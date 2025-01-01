#include <iostream>
#include <fstream>
#include <string>

#include "wav.hpp"

void WAVHandler::open() {
	wavFile = std::ifstream(wavFilePath, std::ios::binary);
	if (!wavFile) {
		std::cerr << "Unable to open the file " << wavFilePath << std::endl;
		exit(1); // Add common function to exit with PATerminate
	}
}

void WAVHandler::close() {
	if (wavFile.is_open()) {
		wavFile.close();
	}
	dataChunk->clear();
	dataChunkSize = 0U;
	dataCursor = dataChunk->cbegin();
	bytesPerBloc = 0U;
}

bool WAVHandler::checkHeader() {
	if (wavFile.is_open() == false) {
		return false;
	}

	constexpr unsigned int headersize = 44U;
	unsigned int cursor = 0U;
	
	/* FileTypeBlocID */
	char fileTypeBlocID[4];
	wavFile.read(fileTypeBlocID, 4);
	if (wavFile.gcount() < 4 || std::string(fileTypeBlocID, 4) != "RIFF") {
		return false;
	}

	/* File size */
	uint32_t fileSize;
	wavFile.read(reinterpret_cast<char*>(&fileSize), 4);
	if (wavFile.gcount() < 4) {
		return false;
	}

	/* FileFormatID */
	char fileFormatID[4];
	wavFile.read(fileFormatID, 4);
	if (wavFile.gcount() < 4 || std::string(fileFormatID, 4) != "WAVE") {
		return false;
	}

	/* FormatBlocID */
	char formatBlocID[4];
	const char formatBlocIDRef[5] = { 0x66, 0x6D, 0x74, 0x20, 0x00 };
	wavFile.read(formatBlocID, 4);
	if (wavFile.gcount() < 4 || std::string(formatBlocID, 4) != formatBlocIDRef) {
		return false;
	}

	/* Bloc size */
	uint32_t blocSize;
	wavFile.read(reinterpret_cast<char*>(&blocSize), 4);
	if (wavFile.gcount() < 4) {
		return false;
	}

	/* Audio Format */
	uint16_t audioFormat;
	wavFile.read(reinterpret_cast<char*>(&audioFormat), 2);
	if (wavFile.gcount() < 2 || audioFormat != 1U) {
		return false;
	}
	
	/* Number of channels */
	uint16_t numChannels;
	wavFile.read(reinterpret_cast<char*>(&numChannels), 2);
	if (wavFile.gcount() < 2 || numChannels != 1U) {
		return false;
	}

	/* Frequency */
	uint32_t frequency;
	wavFile.read(reinterpret_cast<char*>(&frequency), 4);
	if (wavFile.gcount() < 4 || frequency != 48000U) {
		return false;
	}

	/* Bytes per second */
	uint32_t bytestPerSec;
	wavFile.read(reinterpret_cast<char*>(&bytestPerSec), 4);
	if (wavFile.gcount() < 4 || bytestPerSec != 96000U) {
		return false;
	}

	/* Bytes per bloc */
	uint16_t bPerBloc;
	wavFile.read(reinterpret_cast<char*>(&bPerBloc), 2);
	if (wavFile.gcount() < 2 || bPerBloc != 2U) {
		return false;
	}
	bytesPerBloc = bPerBloc;

	/* Bits per sample */
	uint16_t bitsPerSample;
	wavFile.read(reinterpret_cast<char*>(&bitsPerSample), 2);
	if (wavFile.gcount() < 2 || bitsPerSample != 16U) {
		return false;
	}

	/* DatablocID */
	char dataBlocID[4];
	wavFile.read(dataBlocID, 4);
	if (wavFile.gcount() < 4 || std::string(dataBlocID, 4) != "data") {
		return false;
	}

	/* Data size */
	uint32_t dataSize;
	wavFile.read(reinterpret_cast<char*>(&dataSize), 4);
	if (wavFile.gcount() < 4) {
		return false;
	}
	dataChunkSize = dataSize;

	unsigned int pos = wavFile.tellg();
	if (pos != 44U) {
		return false;
	}
	return true;
}

bool WAVHandler::loadDataChunk() {
	if (dataChunkSize > 0) {
		wavFile.seekg(44);
		size_t num_samples = dataChunkSize / bytesPerBloc;
		dataChunk->reserve(num_samples);
		for (size_t i = 0U; i < num_samples; ++i) {
			uint16_t bloc;
			wavFile.read(reinterpret_cast<char*>(&bloc), bytesPerBloc);
			if (wavFile.gcount() != 2) {
				return false;
			}
			dataChunk->push_back(bloc);
		}
		dataCursor = dataChunk->cbegin();
	}
	return true;
}

void WAVHandler::read_next(int16_t* buffer) {
	*buffer = *reinterpret_cast<const int16_t*>(&(*dataCursor));;
	++dataCursor;
	if (dataCursor == dataChunk->cend()) {
		dataCursor = dataChunk->cbegin();
	}
}

WAVHandler::WAVHandler(std::string filePath) {
	wavFilePath = filePath;
	dataChunk = new std::vector<uint16_t>();
	dataChunkSize = 0U;
	dataCursor = dataChunk->cbegin();
	bytesPerBloc = 0U;
}

WAVHandler::~WAVHandler() {
	std::cout << "~WAVHandler" << std::endl;
	close();
	delete dataChunk;
}
