#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "wav.hpp"

constexpr std::streamoff headerSize = 44;

bool WAVHandler::open() {
	wavFile = std::ifstream(wavFilePath, std::ios::binary);
	if (!wavFile) {
		std::wcerr << L"Unable to open the file " << wavFilePath << std::endl;
		return false;
	}
	return true;
}

void WAVHandler::close() {
	if (wavFile.is_open()) {
		wavFile.close();
	}
	if (dataChunk != nullptr) {
		dataChunk->clear();
		dataCursor = dataChunk->cbegin();
	}
	dataChunkSize = 0U;
	bytesPerBloc = 0U;
}

bool WAVHandler::checkHeader() {
	if (!wavFile.is_open()) {
		return false;
	}
	
	/* FileTypeBlocID */
	char fileTypeBlocID[4];
	wavFile.read(fileTypeBlocID, sizeof fileTypeBlocID);
	if (wavFile.gcount() != sizeof fileTypeBlocID || std::string(fileTypeBlocID, sizeof fileTypeBlocID) != "RIFF") {
		return false;
	}

	/* File size */
	uint32_t fileSize;
	wavFile.read(reinterpret_cast<char*>(&fileSize), sizeof fileSize);
	if (wavFile.gcount() != sizeof fileSize) {
		return false;
	}

	/* FileFormatID */
	char fileFormatID[4];
	wavFile.read(fileFormatID, sizeof fileFormatID);
	if (wavFile.gcount() != sizeof fileFormatID || std::string(fileFormatID, sizeof fileFormatID) != "WAVE") {
		return false;
	}

	/* FormatBlocID */
	char formatBlocID[4];
	wavFile.read(formatBlocID, sizeof formatBlocID);
	if (wavFile.gcount() != sizeof formatBlocID || std::string(formatBlocID, sizeof formatBlocID) != "fmt ") {
		return false;
	}

	/* Bloc size */
	uint32_t blocSize;
	wavFile.read(reinterpret_cast<char*>(&blocSize), sizeof blocSize);
	if (wavFile.gcount() != sizeof blocSize) {
		return false;
	}

	/* Audio Format */
	uint16_t audioFormat;
	wavFile.read(reinterpret_cast<char*>(&audioFormat), sizeof audioFormat);
	if (wavFile.gcount() != sizeof audioFormat || audioFormat != 1U) {
		return false;
	}
	
	/* Number of channels */
	uint16_t numChannels;
	wavFile.read(reinterpret_cast<char*>(&numChannels), sizeof numChannels);
	if (wavFile.gcount() != sizeof numChannels || numChannels != 2U) {
		return false;
	}

	/* Frequency */
	uint32_t frequency;
	wavFile.read(reinterpret_cast<char*>(&frequency), sizeof frequency);
	if (wavFile.gcount() != sizeof frequency || frequency != 48000U) {
		return false;
	}

	/* Bytes per second */
	uint32_t bytestPerSec;
	wavFile.read(reinterpret_cast<char*>(&bytestPerSec), sizeof bytestPerSec);
	if (wavFile.gcount() != sizeof bytestPerSec || bytestPerSec != 192000U) {
		return false;
	}

	/* Bytes per bloc */
	uint16_t bPerBloc;
	wavFile.read(reinterpret_cast<char*>(&bPerBloc), sizeof bPerBloc);
	if (wavFile.gcount() != sizeof bPerBloc || bPerBloc != 4U) {
		return false;
	}
	bytesPerBloc = bPerBloc;

	/* Bits per sample */
	uint16_t bitsPerSample;
	wavFile.read(reinterpret_cast<char*>(&bitsPerSample), sizeof bitsPerSample);
	if (wavFile.gcount() != sizeof bitsPerSample || bitsPerSample != 16U) {
		return false;
	}

	/* DatablocID */
	char dataBlocID[4];
	wavFile.read(dataBlocID, sizeof dataBlocID);
	if (wavFile.gcount() != sizeof dataBlocID || std::string(dataBlocID, sizeof dataBlocID) != "data") {
		return false;
	}

	/* Data size */
	uint32_t dataSize;
	wavFile.read(reinterpret_cast<char*>(&dataSize), sizeof dataSize);
	if (wavFile.gcount() != sizeof dataSize) {
		return false;
	}
	dataChunkSize = dataSize;

	std::streamoff pos = wavFile.tellg();
	return (pos == headerSize);
}

bool WAVHandler::loadDataChunk() {
	if (dataChunkSize > 0) {
		wavFile.seekg(headerSize);
		size_t num_samples = dataChunkSize / bytesPerBloc;
		dataChunk->reserve(num_samples);
		for (size_t i = 0U; i < num_samples; ++i) {
			uint32_t bloc;
			wavFile.read(reinterpret_cast<char*>(&bloc), bytesPerBloc);
			if (wavFile.gcount() != sizeof bloc)  {
				return false;
			}
			dataChunk->push_back(bloc);
		}
		dataCursor = dataChunk->cbegin();
	}
	return true;
}

void WAVHandler::read_next(int16_t& buffer_left, int16_t& buffer_right) {
	int32_t buffer_raw = *reinterpret_cast<const int32_t*>(&(*dataCursor));
	buffer_left = (buffer_raw & 0xFFFF0000) >> 16;
	buffer_right = buffer_raw & 0xFFFF;
	++dataCursor;
	if (dataCursor == dataChunk->cend()) {
		dataCursor = dataChunk->cbegin();
	}
}

void WAVHandler::init(const std::wstring& filePath) {
	wavFilePath = filePath;
	if(dataChunk != nullptr) {
		dataChunk->clear();
	}
	dataChunk = std::make_unique<std::vector<uint32_t>>();
	dataChunkSize = 0U;
	dataCursor = dataChunk->cbegin();
	bytesPerBloc = 0U;
}

WAVHandler::WAVHandler(const std::wstring& filePath) {
	init(filePath);
}

WAVHandler::~WAVHandler() {
	close();
	dataChunk.release();
}

WAVHandler::WAVHandler() : wavFile(), wavFilePath(), bytesPerBloc(0U), dataChunkSize(0U), dataChunk(nullptr), dataCursor() {}
