import sys
sys.path.append(r".\Debug Server")
sys.path.append(".")
import bloomberg
bb = bloomberg.Bloomberg()

def status_cb(request_id, service_code, return_code, num_items):
	print request_id, service_code, return_code, num_items

bloomberg.registerstatushandler(status_cb)

# Local PC
#server = "127.0.0.1"
#port = 8199

# Production Server
#server = "LDNPRDAPP13"
server = "192.168.70.79"
port = 8194

## Login Credentials
# Default
uuid=0
sid=0
sidInstance=0
terminalSid=0
terminalSidInstance=0
# Kelly
#uuid=3035903
#sid=720267
#sidInstance=1
#terminalSid=923219
#terminalSidInstance=1
# rblackbourn
#uuid=2122246
#sid=476575
#sidInstance=3
#terminalSid=1294297
#terminalSidInstance=1

# Simple Open
#print "connecting to", server, port
#bb.open(server, port)
# Full Open
print "connecting to", server, port, uuid, sid, sidInstance, terminalSid, terminalSidInstance
bb.open(server, port, uuid, sid, sidInstance, terminalSid, terminalSidInstance)

for i in range(100):
	print "dispatch"
	bb.dispatchloop(0, 10)
	
#tickers1 = ["VOD LN Equity"]
#    
#def cb(args):
#    print args
#
#bloomberg.enablefeature(bloomberg.FeatureALL_PRICE_COND_CODE)
#bb.requestticks(tickers1, [bloomberg.bCategoryCORE, bloomberg.bCategoryMARKET_DEPTH], cb)


#bb.dispatchloop(1, 10)

bb.close()
print "press return to exit"
sys.__stdin__.readline()

