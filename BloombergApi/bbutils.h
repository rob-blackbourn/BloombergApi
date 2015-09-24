#pragma once

namespace Bloomberg
{
	inline std::string bb_todouble(double f)
	{
		char buf_gcvt[_CVTBUFSIZE+10]; // sadly, 10 is the magic number.
		return std::string(_gcvt(f, _CVTBUFSIZE, buf_gcvt));
	}

	inline std::string bb_toint(int4 i)
	{
		char buf_itoa[34];
		return std::string(_itoa(i, buf_itoa, 10));
	}

	inline std::string bb_tobool(BOOL i)
	{
		return std::string( i == 0 ? "N" : "Y" );
	}

	inline std::string bb_topcs(int4 pcs)
	{
		char buf_pcs[sizeof(pcs)+1];
		memcpy(buf_pcs, &pcs, sizeof(pcs));
		buf_pcs[sizeof(pcs)] = '\000';
		return std::string(buf_pcs);
	}

	struct bb_time_t
	{
		int hour, min, sec;
		bb_time_t(long t =  0) : hour(t / 3600), min((t % 3600) / 60), sec(t % 60) {}
		bb_time_t(int _hour, int _min, int _sec) : hour(_hour), min(_min), sec(_sec) {}

		bool is_valid() const { return hour >= 0 && hour < 24 && min >= 0 && min < 60 && sec >= 0 && sec < 60; }

		operator std::string()
		{
			if (is_valid())
			{
				char buf_time[9];
				sprintf(buf_time, "%02d:%02d:%02d", hour, min, sec);
				return std::string(buf_time);
			}
			else
				return std::string();
		}
	};

	struct bb_date_t
	{
		int day, month, year;
		bb_date_t(long t = 0) : day(t % 100), month((t / 100) % 100), year(t / 10000) {}
		bb_date_t(int _day, int _month, int _year) : day(_day), month(_month), year(_year) {}

		// This is clearly a bit of an approximation
		bool is_valid() const { return day > 0 && day <= 31 && month > 0 && month <= 12 && year > 1900 && year <= 3000; }

		operator std::string()
		{
			if (is_valid())
			{
				char buf_date[11];
				sprintf(buf_date, "%04d%02d%02d", year, month, day);
				return std::string(buf_date);
			}
			else
				return std::string();
		}
	};

	struct bb_timestamp_t
	{
		bb_date_t bb_date;
		bb_time_t bb_time;
		bb_timestamp_t(long d = 0, long t = 0) : bb_date(d), bb_time(t) {}
		bb_timestamp_t(int day, int month, int year, int hour, int min, int sec) : bb_date(day, month, year), bb_time(hour, min, sec) {}

		operator std::string()
		{
			return std::string(bb_date) + " " + std::string(bb_time);
		}
	};

	enum BB_FORMAT
	{
		BB_FMT_STRING = 1,
		BB_FMT_NUMERIC = 2,
		BB_FMT_PRICE = 3,
		BB_FMT_SECURITY = 4,
		BB_FMT_DATE = 5,
		BB_FMT_TIME = 6,
		BB_FMT_DATETIME = 7,
		BB_FMT_BULK = 8,
		BB_FMT_MONTHYEAR = 9,
		BB_FMT_BOOL = 10,
		BB_FMT_ISOCCY = 11
	};

	enum BB_TICKTYPE
	{
		BB_TICKTYPE_TCM				= BB_IDX_TCM,			/* Ticker-Coupon-Maturity    */
		BB_TICKTYPE_CUSIP			= BB_IDX_CUSIP,			/* Committee on Uniform      */
															/* Securities Identification */
															/* Procedures (CUSIP)        */
															/* (8 chars + 1 check digit) */
		BB_TICKTYPE_EUROCLEAR		= BB_IDX_EUROCLEAR,
		BB_TICKTYPE_ISMA			= BB_IDX_ISMA,			/*                           */
		BB_TICKTYPE_SEDOL1			= BB_IDX_SEDOL1,		/* Stock Exchange Daily      */
															/* Official List (London)    */
															/* (7 digits)                */
		BB_TICKTYPE_SEDOL2			= BB_IDX_SEDOL2,		/* Stock Exchange Daily      */
															/* Official List (London)    */
															/* (7 digits)                */
		BB_TICKTYPE_CEDEL			= BB_IDX_CEDEL,
		BB_TICKTYPE_WPK				= BB_IDX_WPK,			/* Wertpapier Kenn-Nummer,   */
															/* German Identification     */
															/* Number (6 digits )        */
		BB_TICKTYPE_RGA				= BB_IDX_RGA,
		BB_TICKTYPE_ISIN			= BB_IDX_ISIN,			/* International Securities  */
															/* Identification Number     */
		BB_TICKTYPE_DUTCH			= BB_IDX_DUTCH,
		BB_TICKTYPEVALOREN			= BB_IDX_VALOREN,		/* Telekurs assigned number, */
															/* Official in Switzerland   */
		BB_TICKTYPE_FRENCH			= BB_IDX_FRENCH,		/* SICOVAM                   */
		BB_TICKTYPE_COMMON_NUMBER	= BB_IDX_COMMON_NUMBER,	/* Common number             */
		BB_TICKTYPE_CUSIP8			= BB_IDX_CUSIP8,		/* CUSIP (8 chars)           */
		BB_TICKTYPE_JAPAN			= BB_IDX_JAPAN,
		BB_TICKTYPE_BELGIAN			= BB_IDX_BELGIAN,		/* SVM Code                  */
		BB_TICKTYPE_DANISH			= BB_IDX_DANISH,
		BB_TICKTYPE_AUSTRIAN		= BB_IDX_AUSTRIAN,
		BB_TICKTYPE_LUXEMBOURG		= BB_IDX_LUXEMBOURG,
		BB_TICKTYPE_SWEDEN			= BB_IDX_SWEDEN,
		BB_TICKTYPE_NORWAY			= BB_IDX_NORWAY,
		BB_TICKTYPE_ITALY			= BB_IDX_ITALY,
		BB_TICKTYPE_JAPAN_COMPANY	= BB_IDX_JAPAN_COMPANY,
		BB_TICKTYPE_SPAIN			= BB_IDX_SPAIN,
		BB_TICKTYPE_FIRMID			= BB_IDX_FIRMID,
		BB_TICKTYPE_MISC_DOMESTIC	= BB_IDX_MISC_DOMESTIC,
		BB_TICKTYPE_AIBD			= BB_IDX_AIBD,
		BB_TICKTYPE_CINS			= BB_IDX_CINS,			/* CUSIP International       */
															/* Numbering System          */
															/* (8 chars + 1 check digit) */
		BB_TICKTYPE_TICKERX			= BB_IDX_TICKERX,		/* Ticker, exchange          */
		BB_TICKTYPE_CUSIPX			= BB_IDX_CUSIPX,		/* CUSIP, exchange           */
		BB_TICKTYPE_SCM				= BB_IDX_SCM,			/* State-Coupon-Maturity     */
															/* (Municipal)               */
		BB_TICKTYPE_TICKER			= BB_IDX_TICKER,		/* Ticker                    */
		BB_TICKTYPE_STCM			= BB_IDX_STCM,			/* State-Ticker-Coupon-Mrty  */
															/* (Municipal)               */
		BB_TICKTYPE_TICKERDIV		= BB_IDX_TICKERDIV,		/* Ticker-Divident           */
															/* (Preferred)               */
		BB_TICKTYPE_CLIENTPORT		= BB_IDX_CLIENTPORT,	/* Client-Portfolio          */
		BB_TICKTYPE_TCA				= BB_IDX_TCA,			/* Ticker-Coupon-Age         */
															/* (Mortgage Generic)        */
		BB_TICKTYPE_SEDOL			= BB_IDX_IRISH_SEDOL,	/* Irish SEDOL Number        */
		BB_TICKTYPE_CATS			= BB_IDX_CATS,			/* Malaysian CATS Code       */

		BB_TICKTYPE_OBJECT_ID		= BB_IDX_OBJECT_ID,		/* get monid by object id    */
	};

	enum BB_MKT
	{
		BB_MKT_COMMODITY = 1,
		BB_MKT_EQUITY = 2,
		BB_MKT_MUNICIPAL_BOND = 3,
		BB_MKT_PREFERRED_STOCK = 4,
		BB_MKT_CLIENT_PORTFOLIO = 5,
		BB_MKT_MONEY_MARKET = 6,
		BB_MKT_GOVERNMENT = 7,
		BB_MKT_CORPORATE_BOND = 8,
		BB_MKT_INDEX = 9,
		BB_MKT_CURRENCY = 10,
		BB_MKT_MORTGAGE = 11
	};

	typedef std::string	secname_t;
	typedef std::pair<secname_t,BB_TICKTYPE> secdesc_t;
	typedef int4 fieldid_t;
	typedef int4 market_num_t;
	typedef int4 request_id_t;
	typedef int4 monid_t;
	typedef int4 moncat_t;
};
