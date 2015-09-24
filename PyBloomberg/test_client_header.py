import sys
sys.path.append(r".\Debug Client")
sys.path.append(".")
import bloomberg
bb = bloomberg.Bloomberg()
bb.open(8194)
#tickers = ['VOD LN Equity']
tickers = ['US042735AK67 Corp']

data = bb.getheader(tickers)
print data
print data['US042735AK67 Corp']['RT_PCS']
bb.close()
print "press return to exit"
sys.__stdin__.readline()

