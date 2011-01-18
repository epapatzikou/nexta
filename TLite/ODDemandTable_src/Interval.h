#pragma once

class Interval
{
public:
	Interval(const Interval& other)
	{
		m_StartTime=other.m_StartTime;
		m_EndTime=other.m_EndTime;
	}

	Interval(int start,int end):m_StartTime(start),m_EndTime(end)
	{
	}

	bool operator==(Interval& other) const
	{
		return (m_StartTime == other.m_StartTime && m_EndTime == other.m_EndTime);
	}

	bool operator!=(Interval& other) const
	{
		return !(m_StartTime == other.m_StartTime && m_EndTime == other.m_EndTime);
	}

	friend bool operator<(const Interval& first, const Interval& second);

	Interval& operator=(const Interval& other)
	{
		m_StartTime = other.m_StartTime;
		m_EndTime = other.m_EndTime;
		return *this;
	}

	int GetStartTime() const
	{
		return m_StartTime;
	}

	int GetEndTime() const
	{
		return m_EndTime;
	}

private:
	int m_StartTime;
	int m_EndTime;
};