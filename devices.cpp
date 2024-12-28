#include <iostream>
#include "portaudio.h"

#include "devices.hpp"

static void PrintSupportedStandardSampleRates(const PaStreamParameters* inputParameters, const PaStreamParameters* outputParameters) {
	static double standardSampleRates[] = {
		8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
		44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
	};


	for (int i = 0; standardSampleRates[i] > 0; ++i) {
		PaError err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
		if (err == paFormatIsSupported) {
			std::cout << standardSampleRates[i];
		}
	}
	std::cout << std::endl;
}

void printPADevicesInfo() {
	std::cout << "PA version: " << Pa_GetVersion() << std::endl;
	PaDeviceIndex numDev = Pa_GetDeviceCount();
	std::cout << "Number of devices: " << numDev << std::endl;

	for (int i = 0; i < numDev; ++i) {
		const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
		const PaHostApiInfo* hostApiInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);

		std::cout << "\tDevice " << i << std::endl;
		std::cout << "\t\tName: " << deviceInfo->name << std::endl;
		std::cout << "\t\tHost API: " << hostApiInfo->name << std::endl;
		if (i == hostApiInfo->defaultInputDevice) {
			std::cout << "Default input device" << std::endl;
		}
		if (i == hostApiInfo->defaultOutputDevice) {
			std::cout << "Default output device" << std::endl;
		}
		std::cout << "\t\tMax input channels: " << deviceInfo->maxInputChannels << ", max output channels: " << deviceInfo->maxOutputChannels << std::endl;
		std::cout << "\t\tDefault input latency:" << std::endl;

		std::cout << "\t\t\tLow: " << deviceInfo->defaultLowInputLatency << ", high: " << deviceInfo->defaultHighInputLatency << std::endl;

		std::cout << "\t\tDefault output latency:" << std::endl;
		std::cout << "\t\t\tLow: " << deviceInfo->defaultLowOutputLatency << ", high: " << deviceInfo->defaultHighOutputLatency << std::endl;

		PaStreamParameters inputParameters, outputParameters;
		inputParameters.device = i;
		inputParameters.channelCount = deviceInfo->maxInputChannels;
		inputParameters.sampleFormat = paInt16;
		inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
		inputParameters.hostApiSpecificStreamInfo = nullptr;

		outputParameters.device = i;
		outputParameters.channelCount = deviceInfo->maxOutputChannels;
		outputParameters.sampleFormat = paInt16;
		outputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
		outputParameters.hostApiSpecificStreamInfo = nullptr;

		if (inputParameters.channelCount > 0) {
			std::cout << "Supported standard sample rates" << std::endl << "for half - duplex 16 bit " << inputParameters.channelCount << " channel input = " << std::endl;
			PrintSupportedStandardSampleRates(&inputParameters, nullptr);
		}
		if (outputParameters.channelCount > 0) {
			std::cout << "Supported standard sample rates" << std::endl << "for half - duplex 16 bit " << outputParameters.channelCount << " channel output = " << std::endl;
			PrintSupportedStandardSampleRates(nullptr, &outputParameters);
		}
		if (inputParameters.channelCount > 0 && outputParameters.channelCount > 0) {
			std::cout << "Supported standard sample rates" << std::endl << "for half - duplex 16 bit " << inputParameters.channelCount << " channel input, " << inputParameters.channelCount << " channel input = " << std::endl;
			PrintSupportedStandardSampleRates(&inputParameters, &outputParameters);
		}
	}
}
