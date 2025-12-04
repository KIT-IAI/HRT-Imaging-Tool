/*******************************************************************************
SPDX-License-Identifier: GPL-2.0-or-later
Copyright 2010-2025 Karlsruhe Institute of Technology (KIT)
Contact: stephan.allgeier∂kit.edu,
         Institute of Automation and Applied Informatics

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, 51 Franklin Street,
Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************************/



#include "stdafx.h"
#include "InstructionSet.h"



CInstructionSet::CInstructionSet()
{
}

CInstructionSet::~CInstructionSet()
{
}

std::string CInstructionSet::Vendor(void) { return CPU_Rep.vendor_; }
std::string CInstructionSet::Brand(void) { return CPU_Rep.brand_; }
bool CInstructionSet::SSE3(void) { return CPU_Rep.f_1_ECX_[0]; }
bool CInstructionSet::PCLMULQDQ(void) { return CPU_Rep.f_1_ECX_[1]; }
bool CInstructionSet::MONITOR(void) { return CPU_Rep.f_1_ECX_[3]; }
bool CInstructionSet::SSSE3(void) { return CPU_Rep.f_1_ECX_[9]; }
bool CInstructionSet::FMA(void) { return CPU_Rep.f_1_ECX_[12]; }
bool CInstructionSet::CMPXCHG16B(void) { return CPU_Rep.f_1_ECX_[13]; }
bool CInstructionSet::SSE41(void) { return CPU_Rep.f_1_ECX_[19]; }
bool CInstructionSet::SSE42(void) { return CPU_Rep.f_1_ECX_[20]; }
bool CInstructionSet::MOVBE(void) { return CPU_Rep.f_1_ECX_[22]; }
bool CInstructionSet::POPCNT(void) { return CPU_Rep.f_1_ECX_[23]; }
bool CInstructionSet::AES(void) { return CPU_Rep.f_1_ECX_[25]; }
bool CInstructionSet::XSAVE(void) { return CPU_Rep.f_1_ECX_[26]; }
bool CInstructionSet::OSXSAVE(void) { return CPU_Rep.f_1_ECX_[27]; }
bool CInstructionSet::AVX(void) { return CPU_Rep.f_1_ECX_[28]; }
bool CInstructionSet::F16C(void) { return CPU_Rep.f_1_ECX_[29]; }
bool CInstructionSet::RDRAND(void) { return CPU_Rep.f_1_ECX_[30]; }

bool CInstructionSet::MSR(void) { return CPU_Rep.f_1_EDX_[5]; }
bool CInstructionSet::CX8(void) { return CPU_Rep.f_1_EDX_[8]; }
bool CInstructionSet::SEP(void) { return CPU_Rep.f_1_EDX_[11]; }
bool CInstructionSet::CMOV(void) { return CPU_Rep.f_1_EDX_[15]; }
bool CInstructionSet::CLFSH(void) { return CPU_Rep.f_1_EDX_[19]; }
bool CInstructionSet::MMX(void) { return CPU_Rep.f_1_EDX_[23]; }
bool CInstructionSet::FXSR(void) { return CPU_Rep.f_1_EDX_[24]; }
bool CInstructionSet::SSE(void) { return CPU_Rep.f_1_EDX_[25]; }
bool CInstructionSet::SSE2(void) { return CPU_Rep.f_1_EDX_[26]; }

bool CInstructionSet::FSGSBASE(void) { return CPU_Rep.f_7_EBX_[0]; }
bool CInstructionSet::BMI1(void) { return CPU_Rep.f_7_EBX_[3]; }
bool CInstructionSet::HLE(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[4]; }
bool CInstructionSet::AVX2(void) { return CPU_Rep.f_7_EBX_[5]; }
bool CInstructionSet::BMI2(void) { return CPU_Rep.f_7_EBX_[8]; }
bool CInstructionSet::ERMS(void) { return CPU_Rep.f_7_EBX_[9]; }
bool CInstructionSet::INVPCID(void) { return CPU_Rep.f_7_EBX_[10]; }
bool CInstructionSet::RTM(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[11]; }
bool CInstructionSet::AVX512F(void) { return CPU_Rep.f_7_EBX_[16]; }
bool CInstructionSet::RDSEED(void) { return CPU_Rep.f_7_EBX_[18]; }
bool CInstructionSet::ADX(void) { return CPU_Rep.f_7_EBX_[19]; }
bool CInstructionSet::AVX512PF(void) { return CPU_Rep.f_7_EBX_[26]; }
bool CInstructionSet::AVX512ER(void) { return CPU_Rep.f_7_EBX_[27]; }
bool CInstructionSet::AVX512CD(void) { return CPU_Rep.f_7_EBX_[28]; }
bool CInstructionSet::SHA(void) { return CPU_Rep.f_7_EBX_[29]; }

bool CInstructionSet::PREFETCHWT1(void) { return CPU_Rep.f_7_ECX_[0]; }

bool CInstructionSet::LAHF(void) { return CPU_Rep.f_81_ECX_[0]; }
bool CInstructionSet::LZCNT(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_ECX_[5]; }
bool CInstructionSet::ABM(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[5]; }
bool CInstructionSet::SSE4a(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[6]; }
bool CInstructionSet::XOP(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[11]; }
bool CInstructionSet::TBM(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[21]; }

bool CInstructionSet::SYSCALL(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[11]; }
bool CInstructionSet::MMXEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[22]; }
bool CInstructionSet::RDTSCP(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[27]; }
bool CInstructionSet::_3DNOWEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[30]; }
bool CInstructionSet::_3DNOW(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[31]; }




CInstructionSet::CInstructionSet_Internal::CInstructionSet_Internal()
	: nIds_{ 0 },
	nExIds_{ 0 },
	isIntel_{ false },
	isAMD_{ false },
	f_1_ECX_{ 0 },
	f_1_EDX_{ 0 },
	f_7_EBX_{ 0 },
	f_7_ECX_{ 0 },
	f_81_ECX_{ 0 },
	f_81_EDX_{ 0 },
	data_{},
	extdata_{}
{
	//int cpuInfo[4] = {-1};
	std::array<int, 4> cpui;

	// Calling __cpuid with 0x0 as the function_id argument
	// gets the number of the highest valid function ID.
	__cpuid(cpui.data(), 0);
	nIds_ = cpui[0];

	for (int i = 0; i <= nIds_; ++i)
	{
		__cpuidex(cpui.data(), i, 0);
		data_.push_back(cpui);
	}

	// Capture vendor string
	char vendor[0x20];
	memset(vendor, 0, sizeof(vendor));
	*reinterpret_cast<int*>(vendor) = data_[0][1];
	*reinterpret_cast<int*>(vendor + 4) = data_[0][3];
	*reinterpret_cast<int*>(vendor + 8) = data_[0][2];
	vendor_ = vendor;
	if (vendor_ == "GenuineIntel")
	{
		isIntel_ = true;
	}
	else if (vendor_ == "AuthenticAMD")
	{
		isAMD_ = true;
	}

	// load bitset with flags for function 0x00000001
	if (nIds_ >= 1)
	{
		f_1_ECX_ = data_[1][2];
		f_1_EDX_ = data_[1][3];
	}

	// load bitset with flags for function 0x00000007
	if (nIds_ >= 7)
	{
		f_7_EBX_ = data_[7][1];
		f_7_ECX_ = data_[7][2];
	}

	// Calling __cpuid with 0x80000000 as the function_id argument
	// gets the number of the highest valid extended ID.
	__cpuid(cpui.data(), 0x80000000);
	nExIds_ = cpui[0];

	char brand[0x40];
	memset(brand, 0, sizeof(brand));

	for (int i = 0x80000000; i <= nExIds_; ++i)
	{
		__cpuidex(cpui.data(), i, 0);
		extdata_.push_back(cpui);
	}

	// load bitset with flags for function 0x80000001
	if (nExIds_ >= 0x80000001)
	{
		f_81_ECX_ = extdata_[1][2];
		f_81_EDX_ = extdata_[1][3];
	}

	// Interpret CPU brand string if reported
	if (nExIds_ >= 0x80000004)
	{
		memcpy(brand, extdata_[2].data(), sizeof(cpui));
		memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
		memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
		brand_ = brand;
	}
};

const CInstructionSet::CInstructionSet_Internal CInstructionSet::CPU_Rep = CInstructionSet::CInstructionSet_Internal();
