from . import Assist
import sys
import os

if __name__=="__main__":
    if len(sys.argv)==0:
        print("len of params: %d"%len(sys.argv))
        exit(0)
    fn=sys.argv[1]
    if os.path.splitext(fn)[1]!=".json" or os.path.exists(fn)==False:
        print("json path error: %s"%fn)
        exit(0);
    Assist.jsonTemplate2Pdf(fn)