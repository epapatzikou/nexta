#include "StdAfx.h"
#include "ODPair.h"

bool operator<(const ODPair& first,const ODPair& second)
{
	if (first.m_Origin < second.m_Origin)
	{
		return true;
	}
	else
	{
		if (first.m_Origin == second.m_Origin)
		{
			if (first.m_Dest < second.m_Dest)
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
