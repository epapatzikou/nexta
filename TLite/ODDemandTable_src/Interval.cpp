#include "StdAfx.h"
#include "Interval.h"

bool operator<(const Interval& first, const Interval& second)
{

	if (first.m_StartTime < second.m_StartTime)
	{
		return true;
	}

	else
	{
		if (first.m_StartTime == second.m_StartTime)
		{
			if (first.m_EndTime < second.m_EndTime)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
}
