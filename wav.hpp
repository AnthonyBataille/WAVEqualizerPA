#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief A class that allows opening and reading through a WAV file that has the following properties:
 * Number of channels: 1
 * Sample format: Signed 16 bits PCM
 * Bloc size: 16 bits
 */
class WAVHandler {
private:
	std::ifstream wavFile;
	std::wstring wavFilePath;

	uint16_t bytesPerBloc;
	uint32_t dataChunkSize;

	std::unique_ptr<std::vector<uint32_t>> dataChunk;
	std::vector<uint32_t>::const_iterator dataCursor;

public:
	void init(const std::wstring& filePath);
	bool open();
	void close();
	bool checkHeader();
	bool loadDataChunk();
	void read_next(int16_t& buffer_left, int16_t& buffer_right);

	WAVHandler();
	WAVHandler(const std::wstring& filePath);
	~WAVHandler();
};
