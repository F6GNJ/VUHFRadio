#include "commandHandler.h"

const CommandStruct commandArray[COMMAND_LENGTH] = {
		{0, "TEST", "This is a Test Function \n", CommandTest},
		{1, "LIST", "List all available commands \n", CommandList},
		{2, "PTT-U", "Get PTT UHF State \n", CommandBasicPTTU},
		{3, "PTT-V", "Get PTT VHF State \n", CommandBasicPTTV},
		{4, "CUR", "Get Currents \n", CommandBasicCurrent},
		{5, "SEND-U", "UHF Radio Transmission Start \n", CommandRadioUHFSend},
		{6, "SEND-V", "VHF Radio Transmission Start \n", CommandRadioVHFSend},
		{7, "MOD-U", "UHF Radio Modulation Configuration \n", CommandRadioUHFModConfig},
		{8, "MOD-V", "VHF Radio Modulation Configuration \n", CommandRadioVHFModConfig},
		{9, "ENC-U", "UHF Radio Encoding Configuration \n", CommandRadioUHFEncConfig},
		{10, "ENC-V", "VHF Radio Encoding Configuration \n", CommandRadioVHFEncConfig},
		{11, "AX-ENFLT-U", "UHF AX25 Enabled Filters \n", CommandAX25EnabledFilterUHF},
		{12, "AX-ENFLT-V", "VHF AX25 Enabled Filters \n", CommandAX25EnabledFilterVHF},
		{13, "AX-FLT-U", "UHF AX25 Filter Values \n", CommandAX25FilterValuesUHF},
		{14, "AX-FLT-V", "VHF AX25 Filter Values \n", CommandAX25FilterValuesVHF},
		{15, "AM-U", "UHF AM Demodulation Enable \n", CommandRadioUHFAMMode},
		{16, "AM-V", "VHF AM Demodulation Enable \n", CommandRadioVHFAMMode},
		{17, "FM-U", "UHF FM Demodulation Enable \n", CommandRadioUHFFMMode},
		{18, "FM-V", "VHF FM Demodulation Enable \n", CommandRadioVHFFMMode},
		{19, "FREQ-U", "UHF RF Frequency \n", CommandRadioUHFRFFrequency},
		{20, "FREQ-V", "VHF RF Frequency \n", CommandRadioVHFRFFrequency}
};

uint8_t CommandHandler(uint8_t* command, uint8_t* args, CommandTypeEnum commandType) {
	uint8_t error;

	//Run through the command array and find matching command
	uint16_t i;
	for(i = 0; i < COMMAND_LENGTH; i++) {
		if(strcmp(command, commandArray[i].commandString) == 0) {
			error = commandArray[i].commandFunction(commandType, args, strlen(args));
			if(error == 0) {
				USBVCPWrite("OK\n", 3);
			}
			else {
				USBVCPWrite("ERROR\n", 6);
			}
			return 0;
		}
	}

	return 0;
}

char testStringParam[200];
uint8_t CommandTest(CommandTypeEnum commandType, uint8_t* args, uint8_t argsLength) {
	switch(commandType) {
		case CommandType_Test: {
			USBVCPWrite("+TEST=<string>\n", 15);
			break;
		}
		case CommandType_Query: {
			USBVCPWrite(testStringParam, strlen(testStringParam));
			break;
		}
		case CommandType_Set: {
			uint8_t i;
			for(i = 0; i < argsLength; i++) {
				testStringParam[i] = args[i];
			}
			testStringParam[i++] = '\n';
			testStringParam[i++] = '\0';
			break;
		}
		case CommandType_Execute: {
			USBVCPWrite("Test Execute \n", 14);
			break;
		}
	}

	return 0;
}

uint8_t CommandList(CommandTypeEnum commandType, uint8_t* args, uint8_t argsLength) {
	uint16_t i;
	for(i = 0; i < COMMAND_LENGTH; i++) {
		char string[200];
		sprintf(string, "AT+%-10s: %s", commandArray[i].commandString, commandArray[i].commandInfo);
		USBVCPWrite(string, strlen(string));
	}

	return 0;
}
