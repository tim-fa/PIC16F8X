#include "RegisterData.h"
#include "CPU.h"
#include "Exception.h"
#include <cassert>
#include <FormatString.h>

RegisterData::RegisterData(CPURegisters& cpuRegisters)
	: cpuRegisters(cpuRegisters)
{
	// set up ram array
	initialize();
	// reset all registers to the initial power-on values
	resetPowerOn();
	// reset w register
	cpuRegisters.w = 0;
}

uint8_t RegisterData::readBit(uint8_t address, uint8_t index) const
{
	onRamRead(address);
	return *ram.at(address) >> index & 1;
}

void RegisterData::writeBit(uint8_t address, uint8_t index, bool value) const
{
	*ram.at(address) &= ~(1 << index);
	*ram.at(address) |= value << index;
	onRamWrite(address);
}

void RegisterData::writeByte(const uint8_t& address, unsigned char value) const
{
	*ram.at(address) = value;
	onRamWrite(address);
}

const uint8_t& RegisterData::readByte(const uint8_t& address) const
{
	onRamRead(address);
	return *ram.at(address);
}

uint8_t RegisterData::readBitS(uint8_t address, uint8_t index) const
{
	assert(address <= 0x7F);
	uint8_t adr = readBit(0x3, 5) ? address + 0x80 : address;
	return readBit(adr, index);
}

void RegisterData::writeBitS(uint8_t address, uint8_t index, bool value) const
{
	assert(address <= 0x7F);
	uint8_t adr = readBit(0x3, 5) ? address + 0x80 : address;
	writeByte(adr, readByte(adr) & ~(1 << index)); // clear bit
	writeByte(adr, readByte(adr) | value << index); // set bit to value
}

void RegisterData::writeByteS(const uint8_t& address, unsigned char value) const
{
	assert(address <= 0x7F);
	uint8_t adr = readBit(0x3, 5) ? address + 0x80 : address;
	writeByte(adr, value);
}

const uint8_t& RegisterData::readByteS(const uint8_t& address) const
{
	assert(address <= 0x7F);
	uint8_t adr = readBit(0x3, 5) ? address + 0x80 : address;
	return readByte(adr);
}

// reset all values to their power-on defaults
void RegisterData::resetPowerOn()
{
	initialize();
	writeByte(0x2, 0);
	cpuRegisters.w = 0;
	// Set default startup values
	writeByte(0x03, 0x18); // Status
	writeByte(0x81, 0xFF);
	writeByte(0x85, 0xFF);
	writeByte(0x86, 0xFF);
}

void RegisterData::initialize()
{
	ram.clear();
	for (int i = 0; i <= 0xFF; i++) {
		if (i == 0x80 || i == 0x82 || i == 0x83 || i == 0x84 || i == 0x8A || i == 0x8B || (i >= 0x8C && i <= 0xAF)) {
			ram.push_back(ram.at(i - 0x80));
		}
		else {
			ram.push_back(std::make_shared<uint8_t>(0));
		}
	}

	onRamWrite.connect([this](int address) {
		if (address == 0x2)
		{
			this->cpuRegisters.programCounter = readByte(0x2) & 0xFF;
			this->cpuRegisters.programCounter |= ((readByte(0xA) & 0x1F) << 8);
		}
		});
}

// increase the program counter by a given amount
void RegisterData::increasePCBy(uint16_t amount)
{
	cpuRegisters.programCounter = getPcl() + amount;
	writeByte(0x82, cpuRegisters.programCounter & 0xFF);
}


// returns the current program counter value
const uint16_t& RegisterData::getPcl() const
{
	return cpuRegisters.programCounter;
}