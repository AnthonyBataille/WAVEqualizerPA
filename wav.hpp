#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <iterator>

/**
 * @brief A class that allows opening and reading through a WAV file that has the following properties:
 * Numbr of channels: 1
 * Sample format: Signed 16 bits PCM
 * Bloc size: 16 bits
 */
class WAVHandler {
protected:
	std::ifstream wavFile;
	std::string wavFilePath;

	uint16_t bytesPerBloc;
	uint32_t dataChunkSize;

	std::vector<uint16_t>* dataChunk;
	std::vector<uint16_t>::const_iterator dataCursor;

public:
	void open();
	void close();
	bool checkHeader();
	bool loadDataChunk();
	void read_next(int16_t* buffer);

	WAVHandler(std::string filePath);
	~WAVHandler();
};
