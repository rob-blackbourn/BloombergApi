import sys
from select import select

# Setup for Bloomberg
sys.path.insert(0, r".\Debug Server")
#sys.path.insert(0, r".\Debug Client")
sys.path.insert(0, ".")
import bloomberg

bb = bloomberg.Bloomberg()
print "connecting to server"
bb.open("127.0.0.1", 8199)
#bb.open(8194)

import win32com.client, time, pywintypes
from AdoWrapper import *

cxn = win32com.client.Dispatch('ADODB.Connection')
cxn.CursorLocation = adUseClient

server = 'PC785'
database = 'rtb'
cxn.Open("Provider='SQLOLEDB';Data Source='%s';Initial Catalog='%s';Integrated Security='SSPI';" % (server, database))
cxn.CommandTimeout = 600

def cb(args):
    for ticker, data in args.items():
        for mnemonic, value in data.items():
            if mnemonic in ("BID_PRICE", "ASK_PRICE", "TRADE_PRICE", "ALL_PRICE", "BT_LSE_LAST_PRICE"):
                print ticker, mnemonic, value

                try:
                    cmd = win32com.client.Dispatch('ADODB.Command')
                    cmd.CommandTimeout = 600
                    cmd.CommandType = adCmdText
                    cmd.CommandText = "insert into prices(update_time, ticker, mnemonic, price) values (getdate(), ?, ?, ?)"
                    cmd.ActiveConnection = cxn

                    cmd.Parameters.Append(cmd.CreateParameter("ticker", adBSTR, adParamInput, len(ticker), ticker))
                    cmd.Parameters.Append(cmd.CreateParameter("mnemonic", adBSTR, adParamInput, len(mnemonic), mnemonic))
                    cmd.Parameters.Append(cmd.CreateParameter("price", adDouble, adParamInput, 0, value))

                    (rs, status) = cmd.Execute()
                except Exception, e:
                    print "Error:", e
                    
fd = bb.getsocket()
bloomberg.enablefeature(bloomberg.FeatureALL_PRICE_COND_CODE)

tickers = ['JPY Curncy', 'GBP Curncy', 'EUR Curncy']
#tickers = ['VOD LN Equity', 'SBRY LN Equity', 'TSCO LN Equity', 'BP/ LN Equity', 'HSBA LN Equity', 'GSK LN Equity', 'RDSA LN Equity']
#tickers = ['IBM US Equity', 'MSFT UQ Equity', 'AIG UN Equity', 'CSCO UQ Equity', 'T UN Equity']
bb.requestticks(tickers, [bloomberg.bCategoryCORE], cb)

count = 100
while count >= 0:
    print "waiting ..."
    (iwtd, owtd, ewtd) = select([fd], [], [], 10)
    if fd in iwtd:
        print "dispatching"
        bb.dispatchevent(fd)
    count = count - 1


cxn.Close()

bb.close()
print "press any key to continue"
sys.__stdin__.readline()
