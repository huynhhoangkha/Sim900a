//Sim900A module library by Huynh Hoang Kha
//Using AT command at serial port

#pragma once
#ifndef _SIM900A_H_
#define _SIM900A_H_
#include<Arduino.h>
#include<SoftwareSerial.h>

#ifndef SIM900A_MESSAGE_LENGTH
#define SIM900A_MESSAGE_LENGTH 256
#endif // !SIM900A_MESSAGE_LENGTH

#define MAX_PHONE_NUMBER_LENGTH 16
#define SERIAL_TIMEOUT_MILLISECOND 1000

#define _SS_MAX_RX_BUFF 256
#if (RAMEND < 1000)
#define SERIAL_BUFFER_SIZE 16
#else
#define SERIAL_BUFFER_SIZE 64
#endif

class SIM900A {
public:
	enum SerialMode { 
		HARDWARE_SERIAL,
		SOFTWARE_SERIAL 
	};
	enum Sim900AEvent {
		INCOMING_MESSAGE,
		INCOMING_CALL,
		ACKNOWLEDGEMENT,
		UNKNOWN_EVENT,
		RECEIVING_ERROR,
		NO_DATA,
		USER_BUFFER_OVERFLOW,
		SYSTEM_BUFFER_OVERFLOW
	};
	SIM900A(int baudRate);
	SIM900A(int rxPin, int txPin, int baudrate);
	void sendATCommand(char* atCommand);
	bool isValidCharacter(char c, int indexInBuffer);
	Sim900AEvent receiveSerialData(char* buffer, int bufferLength);
	SerialMode getSerialMode();
	SoftwareSerial* getSoftwareSerialReference();
	bool sendMessage(char* phoneNumber, char* message);
	bool getLastMessage(char* phoneNumberContainer, int phoneNumberContainerSize, char* messageBuffer, int messageBufferSize);
	int getBaudRate();
private:
	SoftwareSerial* simSerial;
	SerialMode serialMode;
	Sim900AEvent parseSerialData(char* buffer, int bufferLength);
	char lastMessageSender[MAX_PHONE_NUMBER_LENGTH];
	char lastMessageContent[SIM900A_MESSAGE_LENGTH];
	int baudRate;
};

#endif // !_SIM900A_H_
