# $Header$
""" Module for Application container for Bloomberg Adapter
"""
__author__  = "$Author: $"
__date__    = "$Date: $ "
__version__ = "$Revision$"
__comments__ = """
$History: $
"""

import datetime
from PyBloomberg import *

###############################################################################
# Given the bounds of an array and a 1D index, returns the nD indices
###############################################################################
def from_index(bounds, index):
    bounds = list(bounds)
    bounds.reverse()
    
    n = len(bounds)
    dims = [0]*n

    dims[0] = index % bounds[0]
    
    sum = bounds[0]
    for dim in range(1, n):
        dims[dim] = (index / sum) % bounds[dim]
        sum = sum * bounds[dim]
    dims.reverse()
    return tuple(dims)

###############################################################################
# Given the bounds of an array and the nD indices, returns a 1D index
###############################################################################
def to_index(bounds, indices):
    bounds, indices = list(bounds), list(indices)
    bounds.reverse()
    indices.reverse()
    n = 1
    for i in bounds:
        n = n * i

    cell = indices[0]
    if cell < 0 or cell >= n:
        raise "Initial index out of bounds"

    sum = 1
    for dim in range(1, len(bounds)):
        if bounds[dim] == 0 or indices[dim] < 0 or indices[dim] >= bounds[dim]:
            raise "Index out of bounds"
        sum = sum * bounds[dim]
        cell = cell + indices[dim] * sum

    return cell

class BbFieldInfo:
    """
    Holds the attributes of a field.
    """
    
    def __init__(self):
        self.FieldCatCode = 0   # Category code
        self.FieldCatName = ""  # Category name
        self.FieldSubCode = 0   # Subcategory code
        self.FieldSubName = ""  # Subcategory name
        self.FieldId = 0        # Field Id
        self.FieldDesc = ""     # Field name
        self.FieldMnemonic = "" # Field Mnemonic
        self.FieldMarket = 0    # See market applicability bits above
        self.FieldSource = 0    # bitfield :
                                #   0 -> use bb_getheader()
                                #   1 -> use bb_tickmntr()
                                #   2 -> use bb_getdata()
                                #   3 -> use bb_gethistory()
                                #   4 -> use bb_gettimeseries()
        self.FieldFormat = 0    # =1 -> ASCII string
                                # =2 -> Regular Numeric (use sscanf() to convert
                                # =3 -> Price (use bb_pricetodouble() to convert

class BbSecMarket:
    """
    Holds the attributes of a market.
    """
    def __init__(self):
        self.SecMarketNum = 0
        self.SecMarketName = ''
        self.MarketDesc = ''
        self.SecMarketIdType = 0

class BbSecMarketDict:
    """
    A dictionary of market attributes with some helper functions to provide mappings.
    """
    def __init__(self):
        raw_markets = (
            ( BB_SEC_COMMODITY,        "Comdty",  "Commodity",        BB_IDX_TICKER     ),
            ( BB_SEC_EQUITY,           "Equity",  "Equity",           BB_IDX_TICKERX    ),
            ( BB_SEC_MUNICIPAL_BOND,   "Muni",    "Municipal Bond",   BB_IDX_STCM       ),
            ( BB_SEC_PREFERRED_STOCK,  "Pfd",     "Preferred Stock",  BB_IDX_TICKERDIV  ),
            ( BB_SEC_CLIENT_PORTFOLIO, "Client",  "Client Portfolio", BB_IDX_CLIENTPORT ),
            ( BB_SEC_MONEY_MARKET,     "M-Mkt",   "Money Market",     BB_IDX_TICKER     ),
            ( BB_SEC_GOVERNMENT,       "Govt",    "Government Bond",  BB_IDX_TCM        ),
            ( BB_SEC_CORPORATE_BOND,   "Corp",    "Corporate Bond",   BB_IDX_TCM        ),
            ( BB_SEC_INDEX,            "Index",   "Index",            BB_IDX_TICKER     ),
            ( BB_SEC_CURRENCY,         "Curncy",  "Currency",         BB_IDX_TICKER     ),
            ( BB_SEC_MORTGAGE,         "Mtge",    "Mortgage",         BB_IDX_TCA        ) )

        self.markets = {}
        self.market_index = {}
        for raw_market in raw_markets:
            market = BbSecMarket()
            market.SecMarketNum, market.SecMarketName, market.SecMarketDesc, market.SecMarketIdType = raw_market
            self.markets[market.SecMarketNum] = market
            self.market_index[market.SecMarketName] = market.SecMarketNum
        
    def GetMarketFromNum(self, num):
        """
        Given a market-id returns the market as a BbSecMarket.
        """
        return self.markets[num]

    def GetMarketFromName(self, name):
        """
        Given a market name returns the market as a BbSecMarket.
        """
        return self.GetMarketFromNum(self.market_index[name])

class BbDataDict:
    """
    Holds the data dictionary of fields and provides helper lookup functions.
    """
    def __init__(self, fname = r"C:\blp\API\bbfields.tbl"):
        self.fields = {}
        self.field_index = {}
        self.obsolete_fields = {}
        self.obsolete_field_index = {}
        self.override_mnemonics_date = ['DDIS_AVG_MTY_ISSUER', 'DDIS_AVG_MTY_ISSUER_SUBS', 'DDIS_AVG_MTY_TICKER', 'DDIS_AVG_MTY']
        self.override_mnemonics_price = ['YAS_BNCHMRK_BOND_PX_FORMAT']
        self.override_mnemonics_string = []
        self.override_mnemonics_bool = ['YAS_BOND_PX_PREC']
        self.load(fname)

    def load(self, fname):
        """
        Loads the class with data from the bbfields.tbl file.
        """
        self.fields = {}
        self.field_index = {}
        
        f = file(fname)
        while 1:
            line = f.readline()
            if line == "":
                break;
            tokens = line.split("|")
            if len(tokens) != 10:
                print "invalid line format"
                break

            field_info = BbFieldInfo()
            
            field_info.FieldCatCode = int(tokens[0])
            field_info.FieldCatName = tokens[1]
            field_info.FieldSubCode = int(tokens[2])
            field_info.FieldSubName = tokens[3]
            field_info.FieldId = int(tokens[4], 16)
            field_info.FieldDesc = tokens[5]
            field_info.FieldMnemonic = tokens[6]
            field_info.FieldMarket = int(tokens[7])
            field_info.FieldSource = int(tokens[8])
            field_info.FieldFormat = int(tokens[9])

            if field_info.FieldMnemonic in self.override_mnemonics_string:
                field_info.FieldFormat = BB_FMT_STRING
            elif field_info.FieldMnemonic in self.override_mnemonics_date:
                field_info.FieldFormat = BB_FMT_DATE
            elif field_info.FieldMnemonic in self.override_mnemonics_price:
                field_info.FieldFormat = BB_FMT_PRICE
            elif field_info.FieldMnemonic in self.override_mnemonics_bool:
                field_info.FieldFormat = BB_FMT_BOOL

            if field_info.FieldCatCode == 999:
                self.obsolete_fields[field_info.FieldId] = field_info
                self.obsolete_field_index[field_info.FieldMnemonic] = field_info.FieldId
            else:
                self.fields[field_info.FieldId] = field_info
                self.field_index[field_info.FieldMnemonic] = field_info.FieldId

        f.close()

    def GetFieldFromId(self, id):
        """
        Given an id return the field as a BbFieldInfo.
        """
        if id in self.fields:
            return  self.fields[id]
        else:
            return self.obsolete_fields[id]

    def GetFieldFromMnemonic(self, mnemonic):
        """
        Given a mnemonic return the field as a BbFieldInfo.
        """
        if mnemonic in self.field_index:
            return self.GetFieldFromId(self.field_index[mnemonic])
        else:
            return self.GetFieldFromId(self.obsolete_field_index[mnemonic])
        
class Bloomberg:
    """
    This class provides a simple interface to the Bloomberg OpenClient API.
    """
    def __init__(self, fname = r"C:\blp\API\bbfields.tbl"):
        self.data_dict = BbDataDict(fname)

    def open(self, *args, **kwargs):
	open(*args, **kwargs)

    def close(self):
	close()

    def getdata(self, *args, **kwargs):
        """
        This method returns the data for the specified securities and fields.

        getdata(tickers, fields, overrides)

        tickers: a sequence of bloomberg tickers: e.g. ['VOD LN Equity', 'US0006M Index']
        fields: a sequence of field mnemonics: e.g. ['NAME', 'CRNCY', 'BID']
        overrides: a sequence of tuple-2's of field mnemonic and string value: e.g. [('ZSPD_BID', '210')]
        """
        arg_defs = [('tickers', True), ('fields', True), ('overrides', False)]
        arg_dict = {}
        if len(args) > len(arg_defs):
            raise TypeError, 'getdata() invalid args'
        for i in range(len(args)):
            arg_dict[arg_defs[i][0]] = args[i]
        for key, value in kwargs:
            arg_dict[key] = value
        for arg_def in arg_defs:
            if arg_def[1] and arg_def[0] not in arg_dict:
                raise TypeError, 'getdata() invalid args'
        
        _tickers = self.prepare_tickers(arg_dict['tickers'])
        _fields = self.prepare_fields(arg_dict['fields'])
        if 'overrides' in arg_dict:
            _overrides = self.prepare_overrides(arg_dict['overrides'])
        else:
            _overrides = []
        
        return self.unpack_secdata(getdata(_tickers, _fields.keys(), _overrides), _fields)

    def requestdata(self, *args, **kwargs):
        """
        This method returns the data for the specified securities and fields. The call
        is asynchronus. When data arrives the callback function is invoked.

        requestdata(tickers, fields, overrides, callback)

        tickers: a sequence of bloomberg tickers: e.g. ['VOD LN Equity', 'US0006M Index']
        fields: a sequence of field mnemonics: e.g. ['NAME', 'CRNCY', 'BID']
        overrides: a sequence of tuple-2's of field mnemonic and string value: e.g. [('ZSPD_BID', '210')]
        """
        arg_defs = [('tickers', True), ('fields', True), ('overrides', False), ('callback', True)]
        arg_dict = {}
        if len(args) > len(arg_defs):
            raise TypeError, 'requestdata() invalid args'
        for i in range(len(args)):
            arg_dict[arg_defs[i][0]] = args[i]
        for key, value in kwargs:
            arg_dict[key] = value
        for arg_def in arg_defs:
            if arg_def[1] and arg_def[0] not in arg_dict:
                raise TypeError, 'requestdata() invalid args'
        
        _tickers = self.prepare_tickers(arg_dict['tickers'])
        _fields = self.prepare_fields(arg_dict['fields'])
        _callback = self.prepare_fields(arg_dict['callback'])
        if 'overrides' in arg_dict:
            _overrides = self.prepare_overrides(arg_dict['overrides'])
        else:
            _overrides = []
        
        requestdata(_tickers, _fields.keys(), _overrides, lambda args: _callback(Bloomberg.unpack_secdata(self, args, _fields)))
	
    def getheader(self, tickers):
        """
        This method returns the header for the specified securities.
        """
        return self.unpack_secheader(getheader(self.prepare_tickers(tickers)))

    def requestheader(self, tickers, callback):
        """
        This method returns the header for the specified securities. The call is asynchronus.
        When the data arrives the supplied callback is invoked. The callback takes one argument
        which is a dictionary result set.

        requestdata(tickers, callback)

        tickers: a sequence of bloomberg tickers: e.g. ['VOD LN Equity', 'US0006M Index']
        callback: a function which takes a single argument, a dictionary result set.
        """
        requestheader(self.prepare_tickers(tickers), lambda args: callback(Bloomberg.unpack_secheader(self, args)))

    def gethistdata(self, *args, **kwargs):
        """
        This method returns historical data for the specified fields and securities, between the specified dates,
        with an interval specified in the flags.
        """
        arg_defs = [('tickers', True), ('fields', True), ('from', True), ('to', True), ('flags', False)]
        arg_dict = {}
        if len(args) > len(arg_defs):
            raise TypeError, 'gethistdata() invalid args'
        for i in range(len(args)):
            arg_dict[arg_defs[i][0]] = args[i]
        for key, value in kwargs:
            arg_dict[key] = value
        for arg_def in arg_defs:
            if arg_def[1] and arg_def[0] not in arg_dict:
                raise TypeError, 'gethistdata() invalid args'
        
        _tickers = self.prepare_tickers(arg_dict['tickers'])
        _fields = self.prepare_fields(arg_dict['fields'], 0x08)
        _from = self.prepare_date(arg_dict['from'])
        _to = self.prepare_date(arg_dict['to'])
        
        if 'flags' in arg_dict:
            _flags = arg_dict['flags']
        else:
            _flags = 0
        return self.unpack_sechistdata(gethistdata(_tickers, _fields.keys(), _from, _to, _flags), _fields)
        
    def requestticks(self, tickers, categories, callback):
        requestticks(self.prepare_tickers(tickers), categories, lambda args: callback(Bloomberg.unpack_sectickdata(self, args)))
        
    def derequestticks(self, tickers):
        derequestticks(self.prepare_tickers(tickers))

    def dispatchloop(self, continuous, timeout):
        dispatchloop(continuous, timeout)

    def dispatchevent(self, socket):
        dispatchevent(socket)

    def getsocket(self):
        return getsocket()

    def isqueueempty(self):
        return isqueueempty()

    def prepare_tickers(self, tickers):
        """Returns a list of bloomberg field id's
        """
        _tickers = []
        for ticker in tickers:
            if len(ticker) > 32:
                raise RuntimeError, 'Ticker too long \'%s\'' % ticker
            elif type(ticker) is tuple or type(ticker) is list:
				_tickers.append(ticker)
			else:
	            _tickers.append([ticker, BB_IDX_TICKERX])
        return _tickers

    def prepare_fields(self, fields, field_source=31):
        """Returns a dictionary with the fieldid as key and the bloomberg field name as value
        """
        _fields = {}
        for field in fields:
            field_info = self.data_dict.GetFieldFromMnemonic(field)
            if field_info.FieldSource & field_source == 0:
                raise RuntimeError, "invalid mnemonic: %s" % field
            _fields[field_info.FieldId] = field
        return _fields

    def prepare_overrides(self, overrides):
        _overrides = []
        for override in overrides:
            _overrides.append([self.data_dict.GetFieldFromMnemonic(override[0]).FieldId, override[1]])
        return _overrides

    def prepare_date(self, date):
        return date.year * 10000 + date.month * 100 + date.day

    def unpack_value(self, format, value):
        if value.rstrip() in ('', 'N.A.', 'FLD UNKNOWN', 'DENIED'):
            return None
        else:
            if format == BB_FMT_STRING:
                return self.unpack_string(value)
            elif format == BB_FMT_NUMERIC:
                return self.unpack_numeric(value)
            elif format == BB_FMT_PRICE:
                return self.unpack_price(value)
            elif format == BB_FMT_SECURITY:
                return self.unpack_security(value)
            elif format == BB_FMT_DATE:
                return self.unpack_date(value)
            elif format == BB_FMT_TIME:
                return self.unpack_time(value)
            elif format == BB_FMT_DATETIME:
                return self.unpack_datetime(value)
            elif format == BB_FMT_BULK:
                return self.unpack_bulk(value)
            elif format == BB_FMT_MONTHYEAR:
                return self.unpack_monthyear(value)
            elif format == BB_FMT_BOOL:
                return self.unpack_bool(value)
            elif format == BB_FMT_ISOCCY:
                return self.unpack_isoccy(value)
            else:
                return value

    def unpack_string(self, value):
        return value

    def unpack_numeric(self, value):
        if value[-2:] == ' M':
            return float(value.split(' ')[0]) * 1000000
        else:
            return float(value)

    def unpack_price(self, value):
        return pricetodouble(value)

    def unpack_security(self, value):
        return value

    def unpack_date(self, value):
        if len(value) == 10:
            # Format mm/dd/yyyy
            d = datetime.date(int(value[6:10]), int(value[0:2]), int(value[3:5]))
            return d
        else:
            return value

    def unpack_time(self, value):
        if len(value) == 8:
            # Format hh:mm:ss
            t = datetime.time(int(value[0:2]), int(value[3:5]), int(value[6:8]))
            return t
        else:
            return value

    def unpack_datetime(self, value):
        if len(value) == 19:
            # Format mm/dd/yyyy hh:mm:ss
            return datetime.datetime(int(value[6:10]), int(value[0:2]), int(value[3:5]), int(value[11:13]), int(value[14:16]), int(value[17:19]))
        elif len(value) == 10:
            return self.unpack_date(value)
        elif len(value) == 8:
            return self.unpack_time(value)
        else:
            return value

    def unpack_bulk(self, value):
        tokens = value[1:].split(value[0])
            
        ndims = int(tokens[0])
        tokens = tokens[1:]
            
        bounds = []
        for i in range(0, ndims):
            dim = int(tokens[i])
            bounds.append(dim)
        tokens = tokens[ndims:]

        data = {}
        data["bounds"] = bounds
        for index in range(0, len(tokens)/2):
            format = int(tokens[index*2])
            value = tokens[index*2+1]
            indices = from_index(bounds, index)
            data[indices] = self.unpack_value(format, value)

        return data
    
    def unpack_monthyear(self, value):
        # can be mm/yy or yyyy or maybe anything? Treat as string until we know more.
        return value

    def unpack_bool(self, value):
        if value == "Y":
            return 1
        else:
            return 0

    def unpack_isoccy(self, value):
        return value

    def unpack_secdata(self, secdata, fields = {}):
        dict = {}
        for sec in secdata.keys():
            dict[sec] = self.unpack_data(secdata[sec], fields)
        return dict

    def unpack_data(self, data, fields={}):
        dict = {}
        for field in data.keys():
            field_info = self.data_dict.GetFieldFromId(field)

            if field in fields:
                mnemonic = fields[field]
            else:
                mnemonic = field_info.FieldMnemonic

            try:
                dict[mnemonic] = self.unpack_value(field_info.FieldFormat, data[field])
            except Exception, e:
                raise 'Failed to unpack "%s" with value "%s"' % (mnemonic, data[field])

        return dict

    def unpack_secheader(self, secdata):
        dict = {}
        for sec in secdata.keys():
            dict[sec] = self.unpack_header(secdata[sec])
        return dict

    def unpack_header(self, header):
        new_header = {}
        for mnemonic, value in header.items():
            if mnemonic in ("PRICE_OPEN", "PRICE_HIGH", "PRICE_LOW", "PRICE_LAST", "PRICE_SETTLE", "PRICE_BID", "PRICE_BID", "PRICE_ASK", "LIMIT_UP", "LIMIT_DOWN", "YEST_PRICE_LAST", "PRICE_AT_TRADE", "PRICE_AT_TRADE_TDY", "PRICE_MID", "PRICE_MID_TDY", "VWAP"):
                new_header[mnemonic] = self.unpack_value(BB_FMT_PRICE, value)
            elif mnemonic in ("YIELD_BID", "YIELD_ASK", "OPEN_INTEREST", "SCALE", "YIELD_LAST", "YIELD_LAST_TDY", "YIELD_OPEN", "YIELD_OPEN_TDY", "YIELD_HIGH", "YIELD_HIGH_TDY", "YIELD_LOW", "YIELD_LOW_TDY", "YEST_YIELD_LAST"):
                new_header[mnemonic] = self.unpack_value(BB_FMT_NUMERIC, value)
            elif mnemonic in ("TIME_LAST", "TIME_START", "TIME_END"):
                new_header[mnemonic] = self.unpack_value(BB_FMT_TIME, value)
            elif mnemonic in ("DATE"):
                new_header[mnemonic] = self.unpack_value(BB_FMT_DATE, value)
            elif mnemonic in ("EXCHANGE_LAST", "TICK_DIRECTION", "SIZE_BID", "SIZE_ASK", "CONDITION_BID", "CONDITION_ASK", "CONDITION_LAST", "CONDITION_MARKET", "VOLUME_TOTAL", "TICKS_TOTAL", "FORMAT"):
                new_header[mnemonic] = self.unpack_value(BB_FMT_NUMERIC, value)
            else:
                new_header[mnemonic] = value

        return new_header

    def unpack_sectickdata(self, secdata):
        dict = {}
        for sec in secdata.keys():
            try:
                dict[sec] = self.unpack_tickdata(secdata[sec])
            except Exception, e:
                print e
                
        return dict

    def unpack_tickdata(self, tickdata):
        new_tickdata = {}
        for mnemonic, value in tickdata.items():
            if mnemonic in ("TRADE_PRICE", "BID_PRICE", "ASK_PRICE", "HIT_PRICE", "TAKE_PRICE", "SETTLE", "HIGH", "LOW", "BID_MKT_MAKER_PRICE", "ASK_MKT_MAKER_PRICE", "MID_PRICE", "BT_LSE_LAST_PRICE", "OPEN", "BT_SEC_BID", "BT_SEC_ASK", "VWAP_PRICE", "AT_TRADE_PRICE"):
                new_tickdata[mnemonic] = self.unpack_value(BB_FMT_PRICE, value)
            elif mnemonic in ("TRADE_SIZE", "BID_SIZE", "ASK_SIZE", "HIT_SIZE", "TAKE_SIZE", "VOLUME", "BID_YIELD", "ASK_YIELD", "BID_MKT_MAKER_SIZE", "ASK_MKT_MAKER_SIZE", "BT_LSE_LAST_SIZE", "VWAP_VOLUME", "AT_TRADE_VOLUME"):
                new_tickdata[mnemonic] = self.unpack_value(BB_FMT_NUMERIC, value)
            elif mnemonic in ("HIGH_YIELD", "LOW_YIELD", "YIELD", "AT_TRADE_VOLUME"):
                new_tickdata[mnemonic] = self.unpack_value(BB_FMT_NUMERIC, value)
            elif mnemonic in ("TRADE_TIME"):
                new_tickdata[mnemonic] = self.unpack_value(BB_FMT_TIME, value)
            elif mnemonic in ("MKT_INDICATOR_TADING", "MKT_INDICATOR_QUOTATION"):
                new_tickdata[mnemonic] = self.unpack_value(BB_FMT_BOOL, value)
            else:
                new_tickdata[mnemonic] = value

        return new_tickdata                
    
    def unpack_sechistdata(self, sechistdata, fields = {}):
        new_sechistdata = {}
        for sec in sechistdata.keys():
            new_sechistdata[sec] = self.unpack_histdata(sechistdata[sec], fields)
        return new_sechistdata

    def unpack_histdata(self, histdata, fields = {}):
        new_histdata = {}
        for field in histdata.keys():
            field_info = self.data_dict.GetFieldFromId(field)
            if field in fields:
                new_histdata[fields[field]] = self.unpack_histvalue(field_info.FieldFormat, histdata[field])
            else:
                new_histdata[field_info.FieldMnemonic] = self.unpack_histvalue(field_info.FieldFormat, histdata[field])
        return new_histdata

    def unpack_histvalue(self, format, histvalue):
        new_histvalue = {}
        for date in histvalue.keys():
            new_histvalue[datetime.date(date / 10000, date / 100 % 100, date % 100)] = histvalue[date]
        return new_histvalue
    
if __name__ == '__main__':
    bb = Bloomberg()
    #callables = ['XS0183668358 Corp']
    callables = [
        'USU19461AC35 Corp', 'US482620AS08 Corp', 'US428040BU24 Corp', 'XS0076449221 Corp', 'US828807BC04 Corp',
        'US74927QAA58 Corp', 'XS0190007889 Corp', 'US617446HR39 Corp', 'US92931DAD49 Corp', 'XS0097773427 Corp',
        'DE0001962108 Corp', 'US45763PAA49 Corp', 'US17248RAF38 Corp', 'XS0194420120 Corp', 'XS0133582147 Corp',
        'US828807AQ09 Corp', 'US912810DB18 Corp', 'US126408GB36 Corp', 'US373298BS66 Corp', 'US29364GAC78 Corp',
        'US56460EAA29 Corp', 'US912810DL99 Corp', 'XS0181803916 Corp', 'US235811AH93 Corp', 'US577778BZ54 Corp',
        'US85590AAF12 Corp', 'XS0163504516 Corp', 'US194832AE17 Corp', 'XS0180158387 Corp', 'XS0171231060 Corp',
        'US69364SAD53 Corp', 'US22541LAB99 Corp', 'US92852EAG08 Corp', 'US866810AF15 Corp', 'XS0109825454 Corp',
        'IT0003086581 Corp', 'XS0173307488 Corp', 'XS0169064416 Corp', 'US428040BW89 Corp', 'GB0057047275 Corp',
        'US05523UAA88 Corp', 'US370442BS34 Corp', 'US460690AK64 Corp', 'US008685AB51 Corp', 'XS0102826673 Corp',
        'US549271AD60 Corp', 'XS0147559263 Corp', 'US27876GAN88 Corp', 'XS0086770699 Corp', 'USU13621AB02 Corp',
        'US59000GAA85 Corp', 'US413627AM28 Corp', 'US25156PAF09 Corp', 'IT0001490819 Corp', 'US48282BAC72 Corp',
        'US892335AL43 Corp', 'XS0177448015 Corp', 'US369300AA61 Corp', 'US60462EAD67 Corp', 'US62941FAA66 Corp',
        'US92931DAB82 Corp', 'US500657AA94 Corp', 'USG47818AA03 Corp', 'US912810DN55 Corp', 'US45820EAH53 Corp',
        'US92931NAA81 Corp', 'XS0125133057 Corp', 'US98412JAY01 Corp', 'XS0122149015 Corp', 'US552953AG66 Corp',
        'US749274AA41 Corp', 'XS0201978540 Corp', 'US009037AE28 Corp', 'US91311QAE52 Corp', 'XS0109827583 Corp',
        'XS0151428470 Corp', 'NL0000121325 Corp', 'US26156FAA12 Corp', 'US25156PAE34 Corp', 'US003924AD93 Corp',
        'XS0173649798 Corp', 'US378272AA66 Corp', 'XS0111469614 Corp', 'US912810DF22 Corp', 'XS0194289400 Corp',
        'XS0184639895 Corp', 'XS0128976692 Corp', 'US912810CM81 Corp', 'XS0203712186 Corp', 'US902118AT52 Corp',
        'XS0187400444 Corp', 'XS0202522438 Corp', 'US828807BA48 Corp', 'US987202AD09 Corp', 'XS0165785683 Corp',
        'XS0189334625 Corp', 'XS0187425516 Corp', 'US912810CK26 Corp', 'XS0148887564 Corp', 'US984121BN27 Corp',
        'USU24658AC79 Corp', 'XS0139186356 Corp', 'XS0094626941 Corp', 'US549271AE44 Corp', 'CH0008011048 Corp',
        'XS0192672888 Corp', 'XS0202261375 Corp', 'US60462EAG98 Corp', 'XS0163503039 Corp', 'IT0001394565 Corp',
        'USY3994LBM91 Corp', 'XS0197153371 Corp', 'XS0192575685 Corp', 'XS0099177726 Corp', 'US65250GAC87 Corp',
        'US14441UAA97 Corp', 'USQ78063AA41 Corp', 'US92928WAF23 Corp', 'US449909AL48 Corp', 'NL0000122885 Corp',
        'FR0000476376 Corp', 'US156686AJ67 Corp', 'US902118BJ61 Corp', 'US007924AF01 Corp', 'US929043AB30 Corp',
        'US87927VAA61 Corp', 'DE0002491909 Corp', 'XS0118496107 Corp', 'US370442BT17 Corp', 'DE0006872351 Corp',
        'XS0196635402 Corp', 'US76182KAK16 Corp', 'US449909AK64 Corp', 'US227116AB64 Corp', 'XS0190009232 Corp',
        'US76182KAM71 Corp', 'XS0056156580 Corp', 'XS0137145685 Corp', 'US920232AA31 Corp', 'US00209AAG13 Corp',
        'USF7813KAE13 Corp', 'US552953AN18 Corp', 'US003669AB45 Corp', 'US456036AC65 Corp', 'US144418AN05 Corp',
        'US912810DJ44 Corp', 'XS0140197582 Corp', 'XS0177187381 Corp', 'XS0177495289 Corp', 'US552953AF83 Corp',
        'US14063RAB15 Corp', 'US008685AA78 Corp', 'US700690AJ90 Corp', 'US413627AN01 Corp', 'US92852EAH80 Corp',
        'NL0000120319 Corp', 'USU37818AA62 Corp', 'US912810CP13 Corp', 'US828807AN77 Corp', 'US69364SAF02 Corp',
        'XS0203614762 Corp', 'US431282AE26 Corp', 'USF7063CAL03 Corp', 'US552953AH40 Corp', 'US912810CV80 Corp',
        'NL0000118925 Corp', 'XS0141884105 Corp', 'XS0172006156 Corp', 'XS0197968927 Corp', 'US92852EAE59 Corp',
        'US779273AH42 Corp', 'US61166WAA99 Corp', 'US45820EAB83 Corp', 'XS0126063386 Corp', 'US828806AC30 Corp',
        'XS0173306597 Corp', 'XS0177495107 Corp', 'USU4582KAA89 Corp', 'US368836AF95 Corp', 'USG9796VAB92 Corp',
        'XS0171467854 Corp', 'US762397AE75 Corp', 'US196877AC85 Corp', 'XS0189327686 Corp', 'US492386AU15 Corp',
        'XS0138717953 Corp', 'XS0113196298 Corp', 'XS0200848041 Corp', 'DE0008512021 Corp', 'US983730AA02 Corp',
        'US912810CY20 Corp', 'XS0031287849 Corp', 'XS0125134295 Corp', 'US003672AA09 Corp', 'US001957BD05 Corp',
        'XS0183668358 Corp', 'XS0187043079 Corp', 'XS0166965797 Corp', 'USQ77974AW52 Corp', 'XS0105516925 Corp',
        'FR0010034298 Corp', 'US912810CS51 Corp', 'US883203BJ93 Corp', 'XS0190027051 Corp', 'US137114AB00 Corp',
        'US892335AH31 Corp', 'US428040BS77 Corp', 'US615337AB89 Corp', 'US902118AL27 Corp']
    callables1 = [
        'USU19461AC35 Corp', 'US482620AS08 Corp', 'US428040BU24 Corp', 'XS0076449221 Corp', 'US828807BC04 Corp',
        'US74927QAA58 Corp', 'XS0190007889 Corp', 'US617446HR39 Corp', 'US92931DAD49 Corp', 'XS0097773427 Corp',
        'DE0001962108 Corp', 'US45763PAA49 Corp', 'US17248RAF38 Corp', 'XS0194420120 Corp', 'XS0133582147 Corp',
        'US828807AQ09 Corp', 'US912810DB18 Corp', 'US126408GB36 Corp', 'US373298BS66 Corp', 'US29364GAC78 Corp',
        'US56460EAA29 Corp', 'US912810DL99 Corp', 'XS0181803916 Corp', 'US235811AH93 Corp', 'US577778BZ54 Corp',
        'US85590AAF12 Corp', 'XS0163504516 Corp', 'US194832AE17 Corp', 'XS0180158387 Corp', 'XS0171231060 Corp',
        'US69364SAD53 Corp', 'US22541LAB99 Corp', 'US92852EAG08 Corp', 'US866810AF15 Corp', 'XS0109825454 Corp',
        'IT0003086581 Corp', 'XS0173307488 Corp', 'XS0169064416 Corp', 'US428040BW89 Corp', 'GB0057047275 Corp',
        'US05523UAA88 Corp', 'US370442BS34 Corp', 'US460690AK64 Corp', 'US008685AB51 Corp', 'XS0102826673 Corp',
        'US549271AD60 Corp', 'XS0147559263 Corp', 'US27876GAN88 Corp', 'XS0086770699 Corp', 'USU13621AB02 Corp',
        'US59000GAA85 Corp', 'US413627AM28 Corp', 'US25156PAF09 Corp', 'IT0001490819 Corp', 'US48282BAC72 Corp',
        'US892335AL43 Corp', 'XS0177448015 Corp', 'US369300AA61 Corp', 'US60462EAD67 Corp', 'US62941FAA66 Corp',
        'US92931DAB82 Corp', 'US500657AA94 Corp', 'USG47818AA03 Corp', 'US912810DN55 Corp', 'US45820EAH53 Corp',
        'US92931NAA81 Corp', 'XS0125133057 Corp', 'US98412JAY01 Corp', 'XS0122149015 Corp', 'US552953AG66 Corp',
        'US749274AA41 Corp', 'XS0201978540 Corp', 'US009037AE28 Corp', 'US91311QAE52 Corp', 'XS0109827583 Corp',
        'XS0151428470 Corp', 'NL0000121325 Corp', 'US26156FAA12 Corp', 'US25156PAE34 Corp', 'US003924AD93 Corp',
        'XS0173649798 Corp', 'US378272AA66 Corp', 'XS0111469614 Corp', 'US912810DF22 Corp', 'XS0194289400 Corp',
        'XS0184639895 Corp', 'XS0128976692 Corp', 'US912810CM81 Corp', 'XS0203712186 Corp', 'US902118AT52 Corp']
    callables2 = [
        'USU19461AC35 Corp', 'US482620AS08 Corp', 'US428040BU24 Corp', 'XS0076449221 Corp', 'US828807BC04 Corp',
        'US74927QAA58 Corp', 'XS0190007889 Corp', 'US617446HR39 Corp', 'US92931DAD49 Corp', 'XS0097773427 Corp',
        'DE0001962108 Corp', 'US45763PAA49 Corp', 'US17248RAF38 Corp', 'XS0194420120 Corp', 'XS0133582147 Corp',
        'US828807AQ09 Corp', 'US912810DB18 Corp', 'US126408GB36 Corp', 'US373298BS66 Corp', 'US29364GAC78 Corp',
        'US56460EAA29 Corp', 'US912810DL99 Corp', 'XS0181803916 Corp', 'US235811AH93 Corp', 'US577778BZ54 Corp',
        'US85590AAF12 Corp', 'XS0163504516 Corp', 'US194832AE17 Corp', 'XS0180158387 Corp', 'XS0171231060 Corp',
        'US69364SAD53 Corp', 'US22541LAB99 Corp', 'US92852EAG08 Corp', 'US866810AF15 Corp', 'XS0109825454 Corp',
        'IT0003086581 Corp', 'XS0173307488 Corp', 'XS0169064416 Corp', 'US428040BW89 Corp', 'GB0057047275 Corp',
        'US05523UAA88 Corp', 'US370442BS34 Corp', 'US460690AK64 Corp', 'US008685AB51 Corp', 'XS0102826673 Corp']
    callables3_1_1 = [
        'USU19461AC35 Corp', 'US482620AS08 Corp', 'US428040BU24 Corp', 'XS0076449221 Corp', 'US828807BC04 Corp']
    callables3_1_2 = [
        'US74927QAA58 Corp', 'XS0190007889 Corp', 'US617446HR39 Corp', 'US92931DAD49 Corp', 'XS0097773427 Corp']
    callables3_2 = [
        'DE0001962108 Corp', 'US45763PAA49 Corp', 'US17248RAF38 Corp', 'XS0194420120 Corp', 'XS0133582147 Corp',
        'US828807AQ09 Corp', 'US912810DB18 Corp', 'US126408GB36 Corp', 'US373298BS66 Corp', 'US29364GAC78 Corp',
        'US56460EAA29 Corp', 'US912810DL99 Corp', 'XS0181803916 Corp', 'US235811AH93 Corp', 'US577778BZ54 Corp']
    callables4 = [
        'US85590AAF12 Corp', 'XS0163504516 Corp', 'US194832AE17 Corp', 'XS0180158387 Corp', 'XS0171231060 Corp',
        'US69364SAD53 Corp', 'US22541LAB99 Corp', 'US92852EAG08 Corp', 'US866810AF15 Corp', 'XS0109825454 Corp',
        'IT0003086581 Corp', 'XS0173307488 Corp', 'XS0169064416 Corp', 'US428040BW89 Corp', 'GB0057047275 Corp',
        'US05523UAA88 Corp', 'US370442BS34 Corp', 'US460690AK64 Corp', 'US008685AB51 Corp', 'XS0102826673 Corp']
    
    secdata = bb.getdata(['USU19461AC35 Corp'], ["CALLABLE"])
    print secdata
    secdata = bb.getdata(['USU19461AC35 Corp'], ["CALL_SCHEDULE"])
    print secdata

    data = secdata['XS0183668358 Corp']
    call_sched = data["CALL_SCHEDULE"]
    bounds = call_sched['bounds']
    for i in range(0, bounds[0]):
        print call_sched[i, 0], call_sched[i, 1]
