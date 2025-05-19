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



#pragma once

#include <bitset>
#include <vector>

class CInstructionSet
{
public:
	CInstructionSet();
	~CInstructionSet();
	std::string Vendor(void);
	std::string Brand(void);
	static bool SSE3(void);
	static bool PCLMULQDQ(void);
	static bool MONITOR(void);
	static bool SSSE3(void);
	static bool FMA(void);
	static bool CMPXCHG16B(void);
	static bool SSE41(void);
	static bool SSE42(void);
	static bool MOVBE(void);
	static bool POPCNT(void);
	static bool AES(void);
	static bool XSAVE(void);
	static bool OSXSAVE(void);
	static bool AVX(void);
	static bool F16C(void);
	static bool RDRAND(void);
	static bool MSR(void);
	static bool CX8(void);
	static bool SEP(void);
	static bool CMOV(void);
	static bool CLFSH(void);
	static bool MMX(void);
	static bool FXSR(void);
	static bool SSE(void);
	static bool SSE2(void);
	static bool FSGSBASE(void);
	static bool BMI1(void);
	static bool HLE(void);
	static bool AVX2(void);
	static bool BMI2(void);
	static bool ERMS(void);
	static bool INVPCID(void);
	static bool RTM(void);
	static bool AVX512F(void);
	static bool RDSEED(void);
	static bool ADX(void);
	static bool AVX512PF(void);
	static bool AVX512ER(void);
	static bool AVX512CD(void);
	static bool SHA(void);
	static bool PREFETCHWT1(void);
	static bool LAHF(void);
	static bool LZCNT(void);
	static bool ABM(void);
	static bool SSE4a(void);
	static 	bool XOP(void);
	static 	bool TBM(void);
	static 	bool SYSCALL(void);
	static 	bool MMXEXT(void);
	static 	bool RDTSCP(void);
	static 	bool _3DNOWEXT(void);
	static 	bool _3DNOW(void);

private:
	class CInstructionSet_Internal
	{
	public:
		CInstructionSet_Internal();
		int nIds_;
		int nExIds_;
		std::string vendor_;
		std::string brand_;
		bool isIntel_;
		bool isAMD_;
		std::bitset<32> f_1_ECX_;
		std::bitset<32> f_1_EDX_;
		std::bitset<32> f_7_EBX_;
		std::bitset<32> f_7_ECX_;
		std::bitset<32> f_81_ECX_;
		std::bitset<32> f_81_EDX_;
		std::vector<std::array<int, 4>> data_;
		std::vector<std::array<int, 4>> extdata_;

	};
	static const CInstructionSet_Internal CPU_Rep;
};

