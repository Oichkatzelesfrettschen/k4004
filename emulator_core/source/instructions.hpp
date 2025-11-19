#pragma once
#include <cstdint>

class RAM;
class ROM;

uint8_t getRegisterValue(const uint8_t* registers, uint8_t reg);
void setRegisterValue(uint8_t* registers, uint8_t reg, uint8_t value);

void NOP();

// Intel 4040 new instructions
void HLT(bool& halted);
void BBS(uint16_t* stack, uint8_t& SP, RAM& ram, ROM& rom, uint8_t srcBackup, bool& interruptEnabled);
void LCR(uint8_t& ACC, uint8_t commandRegister);
void OR4(uint8_t& ACC, const uint8_t* registers);
void OR5(uint8_t& ACC, const uint8_t* registers);
void AN6(uint8_t& ACC, const uint8_t* registers);
void AN7(uint8_t& ACC, const uint8_t* registers);
void DB0(uint8_t& romBank);
void DB1(uint8_t& romBank);
void SB0(uint8_t& registerBank);
void SB1(uint8_t& registerBank);
void EIN(bool& interruptEnabled);
void DIN(bool& interruptEnabled);
void RPM(uint8_t& ACC, const ROM& rom, uint16_t PC);
void JCN(uint16_t* stack, uint8_t SP, uint8_t IR, uint8_t ACC, uint8_t test, const ROM& rom);
void FIM(uint16_t* stack, uint8_t SP, uint8_t* registers, uint8_t IR, const ROM& rom);
void SRC(RAM& ram, ROM& rom, const uint8_t* registers, uint8_t IR);
void FIN(uint8_t* registers, uint16_t PC, uint8_t IR, const ROM& rom);
void JIN(uint16_t* stack, uint8_t SP, const uint8_t* registers, uint8_t IR);
void JUN(uint16_t* stack, uint8_t SP, uint8_t IR, const ROM& rom);
void JMS(uint16_t* stack, uint8_t& SP, uint8_t IR, const ROM& rom, uint8_t stackSize);
void INC(uint8_t* registers, uint8_t IR);
void ISZ(uint16_t* stack, uint8_t SP, uint8_t* registers, uint8_t IR, const ROM& rom);
void ADD(uint8_t& ACC, const uint8_t* registers, uint8_t IR);
void SUB(uint8_t& ACC, const uint8_t* registers, uint8_t IR);
void LD(uint8_t& ACC, const uint8_t* registers, uint8_t IR);
void XCH(uint8_t& ACC, uint8_t* registers, uint8_t IR);
void BBL(uint16_t* stack, uint8_t& SP, uint8_t& ACC, const uint8_t* registers, uint8_t IR);
void LDM(uint8_t& ACC, uint8_t IR);
void WRM(RAM& ram, uint8_t ACC);
void WMP(RAM& ram, uint8_t ACC);
void WRR(ROM& rom, uint8_t ACC);
void WPM();
void WR0(RAM& ram, uint8_t ACC);
void WR1(RAM& ram, uint8_t ACC);
void WR2(RAM& ram, uint8_t ACC);
void WR3(RAM& ram, uint8_t ACC);
void SBM(uint8_t& ACC, const RAM& ram);
void RDM(uint8_t& ACC, const RAM& ram);
void RDR(uint8_t& ACC, const ROM& rom);
void ADM(uint8_t& ACC, const RAM& ram);
void RD0(uint8_t& ACC, const RAM& ram);
void RD1(uint8_t& ACC, const RAM& ram);
void RD2(uint8_t& ACC, const RAM& ram);
void RD3(uint8_t& ACC, const RAM& ram);
void CLB(uint8_t& ACC);
void CLC(uint8_t& ACC);
void IAC(uint8_t& ACC);
void CMC(uint8_t& ACC);
void CMA(uint8_t& ACC);
void RAL(uint8_t& ACC);
void RAR(uint8_t& ACC);
void TCC(uint8_t& ACC);
void DAC(uint8_t& ACC);
void TCS(uint8_t& ACC);
void STC(uint8_t& ACC);
void DAA(uint8_t& ACC);
void KBP(uint8_t& ACC);
void DCL(RAM& ram, uint8_t ACC);
