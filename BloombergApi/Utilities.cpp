#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include "bbutils.h"
#include "stringutils.h"

#include "BloombergApi.h"

namespace Bloomberg
{
	BB_MKT getMarketFromTicker(const std::string& ticker)
	{
		std::vector<std::string> ticker_parts = split(ticker, ' ');
		if (ticker_parts.size() < 1)
		{
			std::stringstream msg;
			msg << "invalid ticker: \"" << ticker << "\"";
			throw std::runtime_error(msg.str());
		}

		// Get a list of fields that are valid for this market type
		const char* yellow_key = ticker_parts.back().c_str();
		BB_MKT bb_market = static_cast<BB_MKT>(-1);
		for (size_t i = 0; i < MAX_SecMarket; ++i)
		{
			if (stricmp(bbSecMarketArray[i].SecMarketName, yellow_key) == 0)
			{
				bb_market = static_cast<BB_MKT>(bbSecMarketArray[i].SecMarketNum);
				break;
			}
		}
		if (bb_market == -1)
		{
			std::stringstream msg;
			msg << "invalid ticker - bad yellow key: \"" << ticker << "\"";
			throw std::runtime_error(msg.str());
		}

		return bb_market;
	}
}