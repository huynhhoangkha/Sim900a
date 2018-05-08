//Sim900A module library by Huynh Hoang Kha
//Using AT command at serial port

#include "Sim900a.h"

SIM900A::SIM900A(int baudRate) {
	this->serialMode = HARDWARE_SERIAL;
	this->baudRate = baudRate;
	while (!Serial) Serial.begin(baudRate);
}

SIM900A::SIM900A(int rxPin, int txPin, int baudRate) {
	this->serialMode = SOFTWARE_SERIAL;
	this->baudRate = baudRate;
	this->simSerial = new SoftwareSerial(rxPin, txPin);
	this->simSerial->begin(baudRate);
}

void SIM900A::sendATCommand(char * atCommand) {
	if (this->serialMode == SOFTWARE_SERIAL)
	this->simSerial->println(atCommand);
	else Serial.println(atCommand);
}

bool SIM900A::isValidCharacter(char c, int indexInBuffer) {
	if (indexInBuffer > 1) {
		if ((int)c == 13) return true;
		if ((int)c == 10) return true;
	}
	if ((int)c < 32 || (int)c > 126) return false;
	return true;
}

SIM900A::Sim900AEvent SIM900A::receiveSerialData(char* buffer, int bufferLength) {
	memset((void*)buffer, '\0' , bufferLength);
	int count = 0;
	char c = '\0';
	if (this->serialMode == SOFTWARE_SERIAL) {
		while (this->simSerial->available()>0) {
			c = this->simSerial->read();
			if (isValidCharacter(c, count)) buffer[count++] = c;
			if (count > bufferLength - 1) return USER_BUFFER_OVERFLOW;
			if (this->simSerial->overflow()) return SYSTEM_BUFFER_OVERFLOW;
			if (this->simSerial->available() < 2) delay((int)(1000.0f / ((float)this->baudRate)));
		}
	}
	else {
		while (Serial.available()>0) {
			c = Serial.read();
			if (isValidCharacter(c, count)) buffer[count++] = c;
			if (count > bufferLength - 1) return USER_BUFFER_OVERFLOW;
			if (Serial.available() < 2) delay((int)(1000.0f / ((float)this->baudRate)));
		}
	}
	if (strlen(buffer) > 0) return this->parseSerialData(buffer, bufferLength);
	else return NO_DATA;
}

SIM900A::SerialMode SIM900A::getSerialMode() {
	return this->serialMode;
}

SoftwareSerial * SIM900A::getSoftwareSerialReference() {
	if (this->serialMode == HARDWARE_SERIAL) return nullptr;
	else return this->simSerial;
}

bool SIM900A::sendMessage(char * phoneNumber, char * message) {
	char setPhoneNumberATCommand[32];
	memset((void*)setPhoneNumberATCommand, '\0', 32);
	strcpy(setPhoneNumberATCommand, "AT+CMGS=");
	setPhoneNumberATCommand[8] = '\"';
	strcpy(setPhoneNumberATCommand + 9, phoneNumber);
	setPhoneNumberATCommand[9 + strlen(phoneNumber)] = '\"';
	setPhoneNumberATCommand[10 + strlen(phoneNumber)] = '\r';
	this->sendATCommand("AT+CMGF=1");
	delay(1000);
	this->sendATCommand(setPhoneNumberATCommand);
	delay(1000);
	this->sendATCommand(message);
	delay(100);
	this->sendATCommand("\x1A");
	return false;
}

bool SIM900A::getLastMessage(char * phoneNumberContainer, int phoneNumberContainerSize, char * messageBuffer, int messageBufferSize) {
	strcpy(phoneNumberContainer, this->lastMessageSender);
	strcpy(messageBuffer, this->lastMessageContent);
	return ((phoneNumberContainerSize > strlen(this->lastMessageSender))&&(messageBufferSize > strlen(this->lastMessageContent)));
}

int SIM900A::getBaudRate() {
	return this->baudRate;
}

SIM900A::Sim900AEvent SIM900A::parseSerialData(char* buffer, int bufferLength) {
	char dataTag[7];
	memset((void*)dataTag, '\0', 7);
	for (int i = 0; i < 7; i++) dataTag[i] = buffer[i];
	dataTag[6] = '\0';
	if (strcmp(dataTag, "+CMT: ") == 0) {
		memset((void*)lastMessageSender, '\0', MAX_PHONE_NUMBER_LENGTH);
		int i;
		for (i = 7; buffer[i] != '\"' && i < MAX_PHONE_NUMBER_LENGTH + 7; i++) this->lastMessageSender[i - 7] = buffer[i];
		this->lastMessageSender[i] = '\0';
		while (((int)buffer[i] != 10) && i < bufferLength - 1) i++;
		if ((int)buffer[i] == 10) strcpy(this->lastMessageContent, buffer + i + 1);
		int msgLength = strlen(this->lastMessageContent);
		while (!this->isValidCharacter(this->lastMessageContent[msgLength - 1], 0)) {
			this->lastMessageContent[msgLength - 1] = '\0';
			msgLength--;
		}
		return INCOMING_MESSAGE;
	}
	return UNKNOWN_EVENT;
}