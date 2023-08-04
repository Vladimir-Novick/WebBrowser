 
#pragma once

#include "TipData.h"

class TipDataPlain : public TipData
	{
	public:
		TipDataPlain(const CString & text, UINT bitmapID = 0)
			: m_message(text), m_bitmapID(bitmapID)
			{}
		~TipDataPlain()
			{/* For your own tip data, delete members here as needed. */}
		CString		m_message;
		UINT		m_bitmapID;
	};