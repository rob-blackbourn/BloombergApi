import sys
sys.path.append(r".\Debug Client")
sys.path.append(".")
import bloomberg
bb = bloomberg.Bloomberg()
bb.open(8194)

tickers = ['VOD LN Equity']

def cb(args):
    print args

bloomberg.enablefeature(bloomberg.FeatureALL_PRICE_COND_CODE)
bb.requestticks(tickers, [bloomberg.bCategoryCORE, bloomberg.bCategoryMARKET_DEPTH], cb)

bb.dispatchloop(1, 10)

bb.close()
print "press return to exit"
sys.__stdin__.readline()

