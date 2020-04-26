#include "CpuInterface.h"

#include <array>

CpuInterface::CpuInterface(int processorClock)
	: processorClock(processorClock)
	, registers(processor.cpuRegisters)
{
}

const uint8_t& CpuInterface::getRegister(Registers r) const
{
	return registers.readByte(r);
}


const uint8_t& CpuInterface::getRegister(int address) const
{
	return registers.readByte(address);
}

const uint16_t& CpuInterface::getProgramCounter() const
{
	return registers.getPcl();
}

const uint16_t& CpuInterface::getLineAtProgramCounter() const
{
	return parser.getLineInFile(registers.getPcl());
}

std::array<bool, 8> CpuInterface::getRegisterBits(Registers r) const
{
	std::array<bool, 8> result;
	for (int i = 0; i < 8; i++)
	{
		result[i] = registers.readBit(r, i);
	}
	return result;
}

std::array<bool, 8> CpuInterface::getRegisterBits(uint8_t address) const
{
	std::array<bool, 8> result;
	for (int i = 0; i < 8; i++)
	{
		result[i] = registers.readBit(address, i);
	}
	return result;
}

const int& CpuInterface::getCpuTime() const
{
	return processor.timeActive;
}

int CpuInterface::getW() const
{
	return processor.cpuRegisters.w;
}

const std::vector<std::string>& CpuInterface::getLoadedFile() const
{
	return parser.getFile();
}

void CpuInterface::setRegister(Registers r, uint8_t value) const
{
	registers.writeByte(r, value);
}

void CpuInterface::setRegister(uint8_t address, uint8_t value) const
{
	registers.writeByte(address, value);
}

void CpuInterface::setRegisterBit(Registers r, uint8_t bit, bool value) const
{
	registers.writeBit(r, bit, value);
}

void CpuInterface::setRegisterBit(uint8_t address, uint8_t bit, bool value) const
{
	registers.writeBit(address, bit, value);
}

void CpuInterface::resetCpuTime()
{
	processor.timeActive = 0;
}

void CpuInterface::setProcessorClock(int frequency)
{
	processorClock = frequency;
}

void CpuInterface::loadFile(const std::string& path)
{
	parser.readFile(path);
	parser.parseLstFile();
}

void CpuInterface::resetProcessor()
{
	registers.resetPowerOn();
	processor.cpuRegisters.w = 0;
	processor.timeActive = 0;
}

void CpuInterface::runProcessor()
{
	if (!processorActive) {
		processorActive = true;
		processorThread = std::thread([this]() {
			while (processorActive) {
				executeSingleInstruction();
				std::this_thread::sleep_for(std::chrono::milliseconds(1000 / processorClock));
			}
			});
	}
}

void CpuInterface::stopProcessor()
{
	if (processorActive) {
		processorActive = false;
		processorThread.join();
	}
}

void CpuInterface::executeSingleInstruction()
{
	if (parser.getLineInFile(registers.getPcl()) != -1) {
		processor.singleStep(registers, parser.getOpcode(registers.getPcl()));
	}
}