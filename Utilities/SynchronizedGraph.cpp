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
#include "SynchronizedGraph.h"

CSynchronizedGraph::CSynchronizedGraph(size_t nSize)
	:m_Graph(CGraph(nSize))
{
}

CSynchronizedGraph::~CSynchronizedGraph()
{
}

void CSynchronizedGraph::AddEdge(size_t node1, size_t node2)
{
	m_Lock.LockWrite();
	m_Graph.AddEdge(node1, node2);
	m_Lock.UnlockWrite();
}

void CSynchronizedGraph::AddBiDirectionalEdge(size_t node1, size_t node2)
{
	m_Lock.LockWrite();
	m_Graph.AddBiDirectionalEdge(node1, node2);
	m_Lock.UnlockWrite();
}

void CSynchronizedGraph::RemoveEdge(size_t node1, size_t node2)
{
	m_Lock.LockWrite();
	m_Graph.RemoveEdge(node1, node2);
	m_Lock.UnlockWrite();
}

void CSynchronizedGraph::RemoveBiDirectionalEdge(size_t node1, size_t node2)
{
	m_Lock.LockWrite();
	m_Graph.RemoveBiDirectionalEdge(node1, node2);
	m_Lock.UnlockWrite();
}

bool CSynchronizedGraph::IsEdge(size_t node1, size_t node2) const
{
	m_Lock.LockRead();
	auto bResult = m_Graph.IsEdge(node1, node2);
	m_Lock.UnlockRead();
	return bResult;
}

size_t CSynchronizedGraph::GetEdgeCount() const
{
	m_Lock.LockRead();
	auto bResult = m_Graph.GetEdgeCount();
	m_Lock.UnlockRead();
	return bResult;
}

size_t CSynchronizedGraph::GetMinimalDistance(size_t node1, size_t node2, size_t nMaximumDistance) const
{
	m_Lock.LockRead();
	auto bResult = m_Graph.GetMinimalDistance(node1, node2, nMaximumDistance);
	m_Lock.UnlockRead();
	return bResult;
}

const std::set<size_t>& CSynchronizedGraph::GetDirectNeighbours(size_t node) const
{
	m_Lock.LockRead();
	auto& bResult = m_Graph.GetDirectNeighbours(node);
	m_Lock.UnlockRead();
	return bResult;
}

std::set<size_t> CSynchronizedGraph::BreathFirstSearch(size_t startNode, size_t nMaximumDistance) const
{
	m_Lock.LockRead();
	auto bResult = m_Graph.BreathFirstSearch(startNode, nMaximumDistance);
	m_Lock.UnlockRead();
	return bResult;
}

std::set<size_t> CSynchronizedGraph::GetIndirectNeighbors(size_t image) const
{
	m_Lock.LockRead();
	auto bResult = m_Graph.GetIndirectNeighbors(image);
	m_Lock.UnlockRead();
	return bResult;
}

std::vector<std::list<size_t>> CSynchronizedGraph::ListSubGraphs() const
{
	m_Lock.LockRead();
	auto bResult = m_Graph.ListSubGraphs();
	m_Lock.UnlockRead();
	return bResult;
}

void CSynchronizedGraph::ListSubGraphs(std::vector<std::list<size_t>>& thisList) const
{
	m_Lock.LockRead();
	m_Graph.ListSubGraphs(thisList);
	m_Lock.UnlockRead();
}

void CSynchronizedGraph::WriteToFile(std::wstring fileName, double bPrintDuplicates) const
{
	m_Graph.WriteToFile(fileName, bPrintDuplicates);
}