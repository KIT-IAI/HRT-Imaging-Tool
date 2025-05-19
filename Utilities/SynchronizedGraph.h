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
#include <set>
#include <list>
#include "Graph.h"
#include "SimpleReadWriteLock.h"

class CSynchronizedGraph
{
public:
	explicit CSynchronizedGraph(size_t nSize);
	~CSynchronizedGraph();


	void AddEdge(size_t node1, size_t node2);
	void AddBiDirectionalEdge(size_t node1, size_t node2);
	void RemoveEdge(size_t node1, size_t node2);
	void RemoveBiDirectionalEdge(size_t node1, size_t node2);

	bool IsEdge(size_t node1, size_t node2) const;

	size_t GetEdgeCount() const;

	// Algorithms

	size_t GetMinimalDistance(size_t node1, size_t node2, size_t nMaximumDistance) const;
	const std::set<size_t>& GetDirectNeighbours(size_t node) const;

	std::set<size_t> BreathFirstSearch(size_t startNode, size_t nMaximumDistance) const;
	std::set<size_t> GetIndirectNeighbors(size_t image) const;
	std::vector<std::list<size_t>> ListSubGraphs() const;
	void ListSubGraphs(std::vector<std::list<size_t>>&) const;

	void WriteToFile(std::wstring fileName, double bPrintDuplicates = false) const;
private:
	CGraph m_Graph;
	CSimpleReadWriteLock m_Lock;
};

