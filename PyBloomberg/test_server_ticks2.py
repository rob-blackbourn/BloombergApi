import sys
from select import select
sys.path.append(r".\Debug Server")
sys.path.append(".")
import bloomberg
bb = bloomberg.Bloomberg()

# Local PC
#server = "127.0.0.1"
server = "192.168.70.79"
port = 8194

print "connecting to", server, port
bb.open(server, port)

tickers = ['ATV NA Equity']

def cb(args):
    print args

fd = bb.getsocket()

#bloomberg.enablefeature(bloomberg.FeatureALL_PRICE_COND_CODE)
#bb.requestticks(tickers, [bloomberg.bCategoryCORE, bloomberg.bCategoryMARKET_DEPTH], cb)
bb.requestticks(tickers, [bloomberg.bCategoryCORE], cb)

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

