import sys
from select import select
sys.path.append(r".\Debug Client")
sys.path.append(".")
import bloomberg
bb = bloomberg.Bloomberg()
bb.open(8194)

tickers = ['VOD LN Equity', 'IBM UN Equity', 'EUR Curncy']

def cb(args):
    print args

fd = bb.getsocket()

bloomberg.enablefeature(bloomberg.FeatureALL_PRICE_COND_CODE)
bb.requestticks(tickers, [bloomberg.bCategoryCORE, bloomberg.bCategoryMARKET_DEPTH], cb)

count = 0
while count < 50:
    print "waiting ..."
    (iwtd, owtd, ewtd) = select([fd], [], [], 10)
    if fd in iwtd:
        print "dispatching"
        bb.dispatchevent(fd)
    count = count + 1
    
bb.close()
print "press return to exit"
sys.__stdin__.readline()

