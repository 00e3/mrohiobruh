#pragma once
#include <vector>
#include <algorithm>

class WhitelistSystem
{
private:
	std::vector<int> m_IDs;

public:
	void Add(int id)
	{
		if (std::find(m_IDs.begin(), m_IDs.end(), id) == m_IDs.end())
			m_IDs.push_back(id);
	}

	void Remove(int id)
	{
		m_IDs.erase(std::remove(m_IDs.begin(), m_IDs.end(), id), m_IDs.end());
	}

	void Clear()
	{
		m_IDs.clear();
	}

	const std::vector<int>& GetIDs() const
	{
		return m_IDs;
	}
};

inline WhitelistSystem g_WhitelistSystem; // <-- this is what the callbacks will use
