#include "gpio.h"
#include "rcc.h"
#include "spi.h"
#include "usb_vcp.h"

#include "ax25.h"
#include "morse.h"
#include "radio.h"

#include "radioStateMachine.h"

#include "atCmdInter.h"
#include "commandHandler.h"

int main(void) {
	//Configure the system clock
	SystemClockInit();
	SystemTickInit();

	//Initialize all configured peripherals
	GPIOInit();
	USBVCPInit();
	ADC1Init();
	SPI1Init();
	SPI2Init();
//	PWM1Init();
//	PWM2Init();

	//Initialize Global Variables
	GlobalVariablesInit();

	//Initialize the VHF and UHF Radio Interfaces, set the SPI and CS
	RadioInterfacesInit();

	//Init VHF Radio
	RadioVHFInit();

	//Config VHF Radio
	RadioConfigStruct radioVHFConfig;
	radioVHFConfig.frequency = 145895000 + 1500;
	radioVHFConfig.datarate = 1200;
	radioVHFConfig.afcRange = 2000;
	radioVHFConfig.fskDeviation = 3000;
	radioVHFConfig.modulation = AFSK;
	RadioVHFModConfig(radioVHFConfig);

	//Set VHF Radio to RX Mode
	AX5043PwrSetPowerMode(RADIO_VHF, PwrMode_RXEN);

	//Init UHF Radio
	RadioUHFInit();

	//Config UHF Radio
	RadioConfigStruct radioUHFConfig;
	radioUHFConfig.frequency = 436450000 + 4600;
	radioUHFConfig.datarate = 1200;
	radioUHFConfig.afcRange = 2000;
	radioUHFConfig.fskDeviation = 3000;
	radioUHFConfig.modulation = AFSK;
	RadioUHFModConfig(radioUHFConfig);

	//Set UHF Radio to RX Mode
	AX5043PwrSetPowerMode(RADIO_UHF, PwrMode_RXEN);

	//Initializations done, VUHFRadio Powered Up
	GPIOWrite(GPIO_OUT_LED5, 1);

	//Variables for USB/AT Commands
	uint8_t isVCPConnected = 0;
	uint8_t rxUSBData[512];
	uint16_t rxLength;
	uint8_t* framed;
	uint16_t framedLength;

	while(1) {
		//USB/AT Command Interpreter
		if(USBVCPRead(rxUSBData, &rxLength) == 1) {
			if(ATCmdFraming(rxUSBData, rxLength, &framed, &framedLength) == 0) {
				ATCmdStruct atCmdstr;
				if(ATCmdParse(framed, framedLength, &atCmdstr) == 0) {
					CommandHandler(atCmdstr.cmd, atCmdstr.args, (CommandTypeEnum)atCmdstr.type);
				}
			}
		}

		//This Sets the LED1 to the Virtual COM Connection state, LED is ON if connected
		if(USBVCPIsConnected()) {
			if(isVCPConnected == 0) {
				//First Connection, write welcome message
				USBVCPWrite("Welcome to VUHFRadio V1! \n", 26);
				USBVCPWrite("Hardware Version: 1.3, May 2020 \n", 33);
				USBVCPWrite("Software Version: 0.9, May 2020 \n", 33);
				USBVCPWrite("This Module uses AT Commands, for more info write AT+LIST \n", 59);

				isVCPConnected = 1;
			}
		}
		else {
			isVCPConnected = 0;
		}
		GPIOWrite(GPIO_OUT_LED4, USBVCPIsConnected());

		//Check for UHF Activity
		if(GPIORead(GPIO_IN_IRQ_U) == 0x01) {
			RadioStateMachine(RADIO_UHF);
		}
		else {
			GPIOWrite(GPIO_OUT_LED1, 0);
		}

		//Check for VHF Activity
		if(GPIORead(GPIO_IN_IRQ_V) == 0x01) {
			RadioStateMachine(RADIO_VHF);
		}
		else {
			GPIOWrite(GPIO_OUT_LED3, 0);
		}

		//Morse Decoder Testing
		MorseStateMachine();

//		if(time + 1000 < GetSysTick()) {
//			time = GetSysTick();
//
//			int8_t uhfRSSI = AX5043GeneralGetRSSI(RADIO_UHF);
//			int8_t vhfRSSI = AX5043GeneralGetRSSI(RADIO_VHF);
//			uint8_t vhfGain = AX5043GeneralGetAGCCurrentGain(RADIO_VHF);
//			char str[200];
//			uint8_t len = sprintf(str, "RSSI VHF: %4d; UHF: %4d; AGC: %3d \n", vhfRSSI, uhfRSSI, vhfGain);
//			USBVCPWrite(str, len);
//
//			int32_t vhfFreqRF = AX5043RXTrackingGetRFFrequency(RADIO_VHF);
//			int32_t uhfFreqRF = AX5043RXTrackingGetRFFrequency(RADIO_UHF);
//			len = sprintf(str, "RF Deviation VHF: %d; UHF: %d \n", vhfFreqRF, uhfFreqRF);
//			USBVCPWrite(str, len);
//		}
	}

	//Send Test Frame
	uint8_t testData[200];
	uint8_t testDataLen = 0;
	testData[testDataLen++] = 'I' << 1;
	testData[testDataLen++] = 'S' << 1;
	testData[testDataLen++] = 'T' << 1;
	testData[testDataLen++] = 'S' << 1;
	testData[testDataLen++] = 'A' << 1;
	testData[testDataLen++] = 'T' << 1;
	testData[testDataLen++] = '1' << 1;
	testData[testDataLen++] = 'C' << 1;
	testData[testDataLen++] = 'S' << 1;
	testData[testDataLen++] = '5' << 1;
	testData[testDataLen++] = 'C' << 1;
	testData[testDataLen++] = 'E' << 1;
	testData[testDataLen++] = 'P' << 1;
	testData[testDataLen++] = ('1' << 1) | 0x01;
	testData[testDataLen++] = 0x03;
	testData[testDataLen++] = 0x00;
	testData[testDataLen++] = '0';
	testData[testDataLen++] = '1';
	testData[testDataLen++] = '2';
	testData[testDataLen++] = '3';
	testData[testDataLen++] = '4';
	testData[testDataLen++] = '5';
	testData[testDataLen++] = '6';
	testData[testDataLen++] = '7';
	testData[testDataLen++] = '8';
	testData[testDataLen++] = '9';
	testData[testDataLen++] = '\n';

	//Random generator
	uint8_t i;
	for(i = 0; i < 200; i++) {
		testData[i] = rand();
	}
	testDataLen = i;

	RadioState radioState = AX5043GeneralRadioState(RADIO_UHF);
	PwrModeSelection pwrMode = AX5043PwrGetPowerMode(RADIO_UHF);
	RadioUHFEnterTX();
	while(1) {
		RadioUHFEnterTX();
		RadioUHFWritePreamble(0x55, 20);
		RadioUHFWriteFrame(testData, testDataLen);
		AX5043FIFOSetFIFOStatCommand(RADIO_UHF, FIFOStat_Commit);

		do {
			radioState = AX5043GeneralRadioState(RADIO_UHF);

			if(radioState == RadioState_TX || radioState == RadioState_TXTail) {
				GPIOWrite(GPIO_OUT_LED0, 1);
			}
			else {
				GPIOWrite(GPIO_OUT_LED0, 0);
			}

		} while(radioState != RadioState_Idle);

		AX5043PwrSetPowerMode(RADIO_UHF, PwrMode_Powerdown);

//		Delay(1000);
	}
}

void Error_Handler(void) {

}
