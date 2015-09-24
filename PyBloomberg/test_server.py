import sys
sys.path.append(r".\Debug Server")
sys.path.append(".")
import bloomberg
bb = bloomberg.Bloomberg()

print "press any key to continue"
sys.__stdin__.readline()

# Robs PC
#server = "127.0.0.1"
#server = "PC785"
#server = "192.168.65.26"
#port = 8199

# Production Server
#server = "LDNPRDAPP13"
server = "192.168.70.79"
port = 8194

# Simple Open
#print "connecting to", server, port
#bb.open(server, port)

## Login Credentials
# Default
#uuid=0
#sid=0
#sidInstance=0
#terminalSid=0
#terminalSidInstance=0
# Kelly
#uuid=3035903
#sid=720267
#sidInstance=1
#terminalSid=923219
#terminalSidInstance=1
# rblackbourn
uuid=2122246
sid=476575
sidInstance=3
terminalSid=1294297
terminalSidInstance=1

# Full Open
print "connecting to", server, port, uuid, sid, sidInstance, terminalSid, terminalSidInstance
bb.open(server, port, uuid, sid, sidInstance, terminalSid, terminalSidInstance)

print bb.getdata(['VOD LN Equity'], ['NAME', 'CHNG_WORK_CAP'])

bb.close()
print "press any key to continue"
sys.__stdin__.readline()
