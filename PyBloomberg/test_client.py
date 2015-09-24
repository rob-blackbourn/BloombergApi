import sys
sys.path.append(r".\Debug Client")
sys.path.append(".")
import bloomberg
bb = bloomberg.Bloomberg()
bb.open(8194)
print bb.getdata(['VOD LN Equity'], ['NAME'])
bb.close()
print "press return to exit"
sys.__stdin__.readline()

