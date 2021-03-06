import sys
sys.path.append(r".\Debug Server")
sys.path.append(".")
import bloomberg
bb = bloomberg.Bloomberg()

# Local PC
server = "127.0.0.1"
port = 8199

print "connecting to", server, port
bb.open(server, port)

tickers = [
	"QQQQ US Equity",
    "JDSU US Equity",
    "SPY US Equity",
    "MSFT US Equity",
    "TOY US Equity",
    "CSCO US Equity",
    "INTC US Equity",
    "LU US Equity",
    "ORCL US Equity",
    "MCLD US Equity",
    "EGY US Equity",
    "SIRI US Equity",
    "TWX US Equity",
    "GM US Equity",
    "AAPL US Equity",
    "SMH US Equity",
    "SUNW US Equity",
    "VIA/B US Equity",
    "XOM US Equity",
    "PA US Equity",
    "AMAT US Equity",
    "WMT US Equity",
    "F US Equity",
    "NT US Equity",
    "NWS/A US Equity",
    "RIMM US Equity",
    "PFE US Equity",
    "DELL US Equity",
    "VOD LN Equity",
    "BNP FP Equity",
    "DTE GY Equity"]
   
tickers_ukx = [
    "AAL LN Equity",
    "ABA LN Equity",
    "ABF LN Equity",
    "ABG LN Equity",
    "ABP LN Equity",
    "ACM LN Equity",
    "AGA LN Equity",
    "AGG LN Equity",
    "AGK LN Equity",
    "AGS LN Equity",
    "AIE LN Equity",
    "AL/ LN Equity",
    "ALLD LN Equity",
    "AMEC LN Equity",
    "AML LN Equity",
    "ANTO LN Equity",
    "ARI LN Equity",
    "ARM LN Equity",
    "ARU LN Equity",
    "ATK LN Equity",
    "AUN LN Equity",
    "AV/ LN Equity",
    "AVE LN Equity",
    "AVZ LN Equity",
    "AWG LN Equity",
    "AZN LN Equity",
    "BA/ LN Equity",
    "BAA LN Equity",
    "BAB LN Equity",
    "BARC LN Equity",
    "BATS LN Equity",
    "BAY LN Equity",
    "BB/ LN Equity",
    "BBA LN Equity",
    "BBY LN Equity",
    "BDEV LN Equity",
    "BFP LN Equity",
    "BG/ LN Equity",
    "BI/ LN Equity",
    "BKG LN Equity",
    "BLND LN Equity",
    "BLT LN Equity",
    "BNZL LN Equity",
    "BOC LN Equity",
    "BOOT LN Equity",
    "BOS LN Equity",
    "BOY LN Equity",
    "BP/ LN Equity",
    "BPB LN Equity",
    "BRBY LN Equity",
    "BRE LN Equity",
    "BRT LN Equity",
    "BSY LN Equity",
    "BT/A LN Equity",
    "BUR LN Equity",
    "BVIT LN Equity",
    "BVS LN Equity",
    "BWNG LN Equity",
    "BWY LN Equity",
    "BXTN LN Equity",
    "CAP LN Equity",
    "CAT LN Equity",
    "CBG LN Equity",
    "CBRY LN Equity",
    "CCC LN Equity",
    "CCL LN Equity",
    "CHS LN Equity",
    "CHTR LN Equity",
    "CKSN LN Equity",
    "CLLN LN Equity",
    "CNA LN Equity",
    "CNE LN Equity",
    "COB LN Equity",
    "CPG LN Equity",
    "CPI LN Equity",
    "CPR LN Equity",
    "CPW LN Equity",
    "CRDA LN Equity",
    "CRST LN Equity",
    "CS/ LN Equity",
    "CSR LN Equity",
    "CSTL LN Equity",
    "CTM LN Equity",
    "CTT LN Equity",
    "CW/ LN Equity",
    "CWD LN Equity",
    "CYD LN Equity",
    "DCG LN Equity",
    "DDT LN Equity",
    "DGE LN Equity",
    "DLAR LN Equity",
    "DMGT LN Equity",
    "DNX LN Equity",
    "DVR LN Equity",
    "DVSG LN Equity",
    "DWV LN Equity",
    "DXNS LN Equity",
    "ECM LN Equity",
    "EGG LN Equity",
    "EID LN Equity",
    "EMA LN Equity",
    "EMG LN Equity",
    "EMI LN Equity",
    "ENO LN Equity",
    "ETI LN Equity",
    "EXL LN Equity",
    "EXR LN Equity",
    "EZJ LN Equity",
    "FCAM LN Equity",
    "FCCN LN Equity",
    "FCD LN Equity",
    "FDL LN Equity",
    "FGP LN Equity",
    "FKI LN Equity",
    "FP/ LN Equity",
    "FPT LN Equity",
    "FRS LN Equity",
    "FTC LN Equity",
    "GET LN Equity",
    "GFS LN Equity",
    "GKN LN Equity",
    "GLH LN Equity",
    "GMG LN Equity",
    "GNK LN Equity",
    "GOG LN Equity",
    "GPOR LN Equity",
    "GRG LN Equity",
    "GRI LN Equity",
    "GSK LN Equity",
    "GUS LN Equity",
    "GWG LN Equity",
    "HAS LN Equity",
    "HBOS LN Equity",
    "HBR LN Equity",
    "HEAD LN Equity",
    "HFD LN Equity",
    "HG/ LN Equity",
    "HHG LN Equity",
    "HLMA LN Equity",
    "HMSO LN Equity",
    "HMV LN Equity",
    "HNS LN Equity",
    "HOF LN Equity",
    "HSBA LN Equity",
    "HSV LN Equity",
    "HSX LN Equity",
    "HTE LN Equity",
    "IAP LN Equity",
    "ICI LN Equity",
    "ICP LN Equity",
    "IHG LN Equity",
    "IMI LN Equity",
    "IMT LN Equity",
    "INCH LN Equity",
    "INVP LN Equity",
    "IOT LN Equity",
    "IPR LN Equity",
    "IRV LN Equity",
    "ISYS LN Equity",
    "ITRK LN Equity",
    "ITV LN Equity",
    "JDW LN Equity",
    "JJB LN Equity",
    "JLT LN Equity",
    "JMAT LN Equity",
    "JPR LN Equity",
    "KEL LN Equity",
    "KESA LN Equity",
    "KGF LN Equity",
    "KID LN Equity",
    "LAND LN Equity",
    "LARD LN Equity",
    "LCI LN Equity",
    "LGEN LN Equity",
    "LII LN Equity",
    "LLOY LN Equity",
    "LMC LN Equity",
    "LMI LN Equity",
    "LMR LN Equity",
    "LMSO LN Equity",
    "LNGO LN Equity",
    "LOG LN Equity",
    "LSE LN Equity",
    "MAB LN Equity",
    "MCA LN Equity",
    "MCTY LN Equity",
    "MDK LN Equity",
    "MFI LN Equity",
    "MGCR LN Equity",
    "MGGT LN Equity",
    "MKS LN Equity",
    "MLC LN Equity",
    "MNU LN Equity",
    "MNZS LN Equity",
    "MONI LN Equity",
    "MPI LN Equity",
    "MRW LN Equity",
    "MSLH LN Equity",
    "MSY LN Equity",
    "MTC LN Equity",
    "MTN LN Equity",
    "MTO LN Equity",
    "MWLM LN Equity",
    "NEX LN Equity",
    "NFDS LN Equity",
    "NGT LN Equity",
    "NIS LN Equity",
    "NRK LN Equity",
    "NTG LN Equity",
    "NVR LN Equity",
    "NWG LN Equity",
    "NXT LN Equity",
    "OML LN Equity",
    "OOM LN Equity",
    "PAG LN Equity",
    "PDG LN Equity",
    "PEA LN Equity",
    "PFG LN Equity",
    "PFL LN Equity",
    "PHY LN Equity",
    "PILK LN Equity",
    "PLR LN Equity",
    "PMO LN Equity",
    "PNN LN Equity",
    "PO/ LN Equity",
    "PON LN Equity",
    "PRU LN Equity",
    "PSN LN Equity",
    "PSON LN Equity",
    "PUB LN Equity",
    "RAC LN Equity",
    "RAT LN Equity",
    "RB/ LN Equity",
    "RBS LN Equity",
    "RDW LN Equity",
    "REL LN Equity",
    "REX LN Equity",
    "RIO LN Equity",
    "RMC LN Equity",
    "RNK LN Equity",
    "ROR LN Equity",
    "RPS LN Equity",
    "RR/ LN Equity",
    "RSA LN Equity",
    "RSW LN Equity",
    "RTO LN Equity",
    "RTR LN Equity",
    "SAB LN Equity",
    "SBRY LN Equity",
    "SCTN LN Equity",
    "SDR LN Equity",
    "SFL LN Equity",
    "SGC LN Equity",
    "SGE LN Equity",
    "SGP LN Equity",
    "SHB LN Equity",
    "SHEL LN Equity",
    "SHI LN Equity",
    "SHP LN Equity",
    "SIG LN Equity",
    "SIV LN Equity",
    "SKP LN Equity",
    "SKS LN Equity",
    "SLOU LN Equity",
    "SLY LN Equity",
    "SMDS LN Equity",
    "SMG LN Equity",
    "SMIN LN Equity",
    "SMWH LN Equity",
    "SN/ LN Equity",
    "SOF LN Equity",
    "SPT LN Equity",
    "SPW LN Equity",
    "SPX LN Equity",
    "SRH LN Equity",
    "SRP LN Equity",
    "SSE LN Equity",
    "SSL LN Equity",
    "STAN LN Equity",
    "STJ LN Equity",
    "SVT LN Equity",
    "SXS LN Equity",
    "TATE LN Equity",
    "TBI LN Equity",
    "TFI LN Equity",
    "THUS LN Equity",
    "TLW LN Equity",
    "TNI LN Equity",
    "TNN LN Equity",
    "TOMK LN Equity",
    "TPK LN Equity",
    "TPT LN Equity",
    "TSCO LN Equity",
    "TTG LN Equity",
    "TWOD LN Equity",
    "UBM LN Equity",
    "ULE LN Equity",
    "ULVR LN Equity",
    "UU/ LN Equity",
    "VCT LN Equity",
    "VDY LN Equity",
    "VED LN Equity",
    "VMOB LN Equity",
    "VOD LN Equity",
    "VRD LN Equity",
    "VTG LN Equity",
    "WBY LN Equity",
    "WEIR LN Equity",
    "WG/ LN Equity",
    "WHM LN Equity",
    "WIN LN Equity",
    "WKP LN Equity",
    "WLB LN Equity",
    "WLW LN Equity",
    "WMH LN Equity",
    "WMPY LN Equity",
    "WOLV LN Equity",
    "WOS LN Equity",
    "WPP LN Equity",
    "WTB LN Equity",
    "WUN LN Equity",
    "XAN LN Equity",
    "XTA LN Equity",
    "YELL LN Equity",
    "YULC LN Equity"]

tickers1 = ["VOD LN Equity"]
    
def cb(args):
    print args

bloomberg.enablefeature(bloomberg.FeatureALL_PRICE_COND_CODE)
bb.requestticks(tickers1, [bloomberg.bCategoryCORE, bloomberg.bCategoryMARKET_DEPTH], cb)

bb.dispatchloop(1, 10)

bb.close()
print "press return to exit"
sys.__stdin__.readline()

