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
#include "Graph.h"

using namespace std;



CGraph::CGraph(size_t size)
{
	this->m_nSize = size;
	m_pNodes.resize(size);
	size_t i = 0;
	for (auto& node : m_pNodes)
	{
		node.nIndex = i++;
		node.lNeighbors = {};
	}
}

CGraph::~CGraph()
{
}

void CGraph::AddEdge(size_t i, size_t j) {
	assert(i >= 0 && i < m_nSize);
	assert(j >= 0 && j < m_nSize);

	if (!IsEdge(i, j))
	{
		m_pNodes[i].lNeighbors.insert(j);
		m_nEdgeCount++;
	}
}
void CGraph::AddBiDirectionalEdge(size_t i, size_t j) {
	AddEdge(i, j);
	AddEdge(j, i);
}

void CGraph::RemoveEdge(size_t i, size_t j) {
	assert(i >= 0 && i < m_nSize);
	assert(j >= 0 && j < m_nSize);

	m_nEdgeCount -= m_pNodes[i].lNeighbors.erase(j);

}
void CGraph::RemoveBiDirectionalEdge(size_t i, size_t j) {
	RemoveEdge(i, j);
	RemoveEdge(j, i);
}

bool CGraph::IsEdge(size_t i, size_t j) const {
	assert(i >= 0 && i < m_nSize);
	assert(j >= 0 && j < m_nSize);

	return m_pNodes[i].lNeighbors.find(j) != m_pNodes[i].lNeighbors.end();
}
bool CGraph::IsConnected(size_t node1, size_t node2) const
{
	if (IsEdge(node1, node2))
		return true;
	return GetMinimalDistance(node1, node2, m_nEdgeCount) != 0;
}

size_t CGraph::GetEdgeCount() const
{
	return m_nEdgeCount;
}

void CGraph::WriteToFile(std::wstring fileName, double bPrintDuplicates /*= false*/) const
{
	std::wofstream out;
	out.open(std::filesystem::path(fileName));

	if (out.bad())
	{
		assert(false);
		return;
	}

	std::set < std::pair<size_t, size_t>> writtenPairs;

	for (auto& node : m_pNodes)
	{
		auto& Neighbors = node.lNeighbors;
		for (auto& Neighbor : Neighbors)
		{
			if (!bPrintDuplicates && writtenPairs.find({ Neighbor, node.nIndex }) != writtenPairs.end())
				continue;

			out << node.nIndex << L"\t" << Neighbor << L"\n";

			writtenPairs.insert({ node.nIndex , Neighbor });
		}
	}
}

/**
*
* Finds the distance of two nodes by using Breadth-first search (http://en.wikipedia.org/wiki/Breadth-first_search)
* Returns the distance of the two nodes or 0 if no path shorter than nMaximumDistance is found.
* Looks like a lot of Copy&Paste from the BreathFirstSearch()-method but is actually more efficent.
*
*/
size_t CGraph::GetMinimalDistance(size_t from, size_t to, size_t nMaximumDistance) const {
	assert(from >= 0 && from < m_nSize);
	assert(to >= 0 && to < m_nSize);
	assert(from != to);

	std::queue<size_t> lReadyQueue;

	std::vector<bool> pWasVisited(m_nSize, false);			// FUN FACT: This is being optimized to a bitvector by the vector class. Wow. Such optimisation

	lReadyQueue.push(from);
	pWasVisited[from] = true;

	size_t nThisLevelChildren = 1;
	size_t nNextLevelChildren = 0;

	size_t nDistance = 1;

	while (lReadyQueue.size() > 0 && nDistance <= nMaximumDistance)
	{
		size_t nodeIndex = lReadyQueue.front();
		const sNode& node = m_pNodes[nodeIndex];

		lReadyQueue.pop();

		for (size_t child : node.lNeighbors)
		{
			if (child == to)
			{
				return nDistance;
			}
			if (!pWasVisited[child])
			{
				nNextLevelChildren++;
				lReadyQueue.push(child);
				pWasVisited[child] = true;
			}
		}

		nThisLevelChildren--;
		if (nThisLevelChildren == 0)		// this means we did one level of Breadth-first search, therefore the distance is increased by 1
		{
			nThisLevelChildren = nNextLevelChildren;
			nNextLevelChildren = 0;
			nDistance++;
		}
	}
	return 0;
}
const set<size_t>& CGraph::GetDirectNeighbours(size_t node) const
{
	return m_pNodes[node].lNeighbors;
}

/*
*
* Returns a list of all nodes that are reachable by the startnode with a maxium distance
*/
set<size_t> CGraph::BreathFirstSearch(size_t startNode, size_t nMaximumDistance) const
{
	if (startNode >= m_nSize)
		return{};

	queue<size_t> lReadyQueue;
	auto lVisitableNodes = set<size_t>();

	vector<bool> pWasVisited(m_nSize, false);

	lReadyQueue.push(startNode);
	pWasVisited[startNode] = true;
	lVisitableNodes.insert(startNode);

	size_t nThisLevelChildren = 1;
	size_t nNextLevelChildren = 0;

	size_t nDistance = 1;

	while (lReadyQueue.size() > 0 && nDistance <= nMaximumDistance)
	{
		size_t nodeIndex = lReadyQueue.front();
		const sNode& node = m_pNodes[nodeIndex];

		lReadyQueue.pop();

		for (size_t child : node.lNeighbors)
		{
			if (!pWasVisited[child])
			{
				nNextLevelChildren++;
				lReadyQueue.push(child);
				lVisitableNodes.insert(child);
				pWasVisited[child] = true;
			}
		}

		nThisLevelChildren--;
		if (nThisLevelChildren == 0)		// this means we did one level of Breadth-first search, therefore the distance is increased by 1
		{
			nThisLevelChildren = nNextLevelChildren;
			nNextLevelChildren = 0;
			nDistance++;
		}
	}
	return lVisitableNodes;
}

std::set<size_t> CGraph::GetIndirectNeighbors(size_t image) const
{
	set<size_t> indirectNeighbors;

	indirectNeighbors.insert(image);
	GetIndirectNeighbors(image, indirectNeighbors);

	return indirectNeighbors;
}

void CGraph::GetIndirectNeighbors(size_t image, set<size_t>& indirectNeighbors) const
{
	auto Nodes = BreathFirstSearch(image, m_nSize);

	indirectNeighbors.insert(Nodes.begin(), Nodes.end());
}

void CGraph::ListSubGraphs(vector<list<size_t>>& subGraphList) const
{
	vector<bool> hasAGroupAssigned(m_nSize);
	for (size_t image = 0; image < hasAGroupAssigned.size(); image++)
	{
		if (hasAGroupAssigned[image])
			continue;
		auto subGraph = GetIndirectNeighbors(image);

		for (auto imageInSubGraph : subGraph)
			hasAGroupAssigned[imageInSubGraph] = true;

		list<size_t> thisSubGraph(subGraph.size());
		move(subGraph.begin(), subGraph.end(), thisSubGraph.begin());

		subGraphList.push_back(thisSubGraph);
	}
}

vector<list<size_t>> CGraph::ListSubGraphs() const
{
	vector<list<size_t>> subGraphList;
	ListSubGraphs(subGraphList);
	return subGraphList;
}
