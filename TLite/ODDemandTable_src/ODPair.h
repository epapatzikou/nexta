#pragma once


class ODPair
{
public:
	ODPair(int origin,int dest):m_Origin(origin),m_Dest(dest)
	{
	}

	bool operator==(ODPair& other) const
	{
		return (m_Origin == other.m_Origin && m_Dest == other.m_Dest);
	}

	bool operator!=(ODPair& other) const
	{
		return !(m_Origin == other.m_Origin && m_Dest == other.m_Dest);
	}

	int GetOrigin() const
	{
		return m_Origin;
	}

	int GetDestination() const
	{
		return m_Dest;
	}

	friend bool operator<(const ODPair& first, const ODPair& second);

private:
	int m_Origin;
	int m_Dest;
};