import sys
sys.path.append(r".\Debug Client")
sys.path.append(".")
import bloomberg
bb = bloomberg.Bloomberg()
bb.open(8194)


############
# OAS Spread
############
# Get the bid and ask from a number of providers
providers = ['RBOS', 'HSET', 'BBVA', 'WLBT', 'SOCG', 'MCCX']
isin = 'XS0169888558'   #VOD 5 06/04/18
tickers = ["%s@%s Corp" % (isin, provider) for provider in providers]
fields = ['BID', 'ASK', 'PRICING_SOURCE']
best_bid_ask_diff = 100000
best_bid = best_ask = 0
best_price_source = ''
secdata = bb.getdata(tickers, fields)
for ticker, data in secdata.items():
    price_source = data['PRICING_SOURCE']
    bid = data['BID']
    ask = data['ASK']

    if (ask - bid) < best_bid_ask_diff:
        best_bid_ask_diff = ask - bid
        best_bid, best_ask, best_price_source = bid, ask, price_source

print "Best bid ask %s: %f, %f" % (best_price_source, best_bid, best_ask)

# Bump the spread of the best price source by 10bp
ticker = '%s@%s Corp' % (isin, best_price_source)
secdata = bb.getdata([ticker], ['BID_OAS_SPREAD'])
spread = secdata[ticker]['BID_OAS_SPREAD']
spread += 10
overrides = [('OAS_SPREAD_BID', str(spread))]
secdata = bb.getdata([ticker], ['BID'], overrides)
print 'Price of bond with the spread bumped by 10bp is %f' % secdata[ticker]['BID']


bb.close()
print "press return to exit"
sys.__stdin__.readline()

