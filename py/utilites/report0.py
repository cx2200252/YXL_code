
from report import Assist
from report import Report
import func
import json
import os
import sys
import shutil

escape_fn_prefix=[]

# 每一行的注释
col0="c1:无deform proxy\nc2:无deform proxy"
col1="c1:泛化结果\nc2:无deform proxy"
description_mapping={"aa":col0+"\n \nproxy 男\nname:",
              "ab": col0+"\n \nproxy 女\nname:",
              "ba":col1+"\n \nbenchmark 男\nname:",
              "bb":col1+"\n \nbenchmark 女\nname:"
              }
match_ids={}
def FilenameMapping(fn):
    if fn in match_ids:
        id = match_ids[fn]+1
        if fn[1:2]=='a':
            return 'aa'+'%03d'%id
        elif fn[1:2]=='b':
            return 'ab'+'%03d'%id

    if fn[0:2]=='bb':
        fn="n"+fn[2:]
    if fn[0:2]=='ba':
        fn="t"+fn[2:]
    if fn in match_ids:
        id = match_ids[fn]+1
        if fn[0:1]=='t':
            return 'aa'+'%03d'%id
        elif fn[0:1]=='n':
            return 'ab'+'%03d'%id
            
    return ""
def SortKey(elem):
    if elem[0:2]=="aa" or elem[0:2]=="ab":
        return -1
    weight=1
    if elem[0:2]=="bb":
        weight=10000
    elem = os.path.splitext(elem)[0]
    if elem in match_ids:
        return match_ids[elem]+weight
    return 100000000

if __name__=="__main__":
    if 1:
        if len(sys.argv)<2:
            print("report dir not set")
            exit(0)
        report_dir=sys.argv[1]
    else:
        report_dir="20190829"

    #
    with open("../report0/" + report_dir + "/match_id.json", 'r') as f:
        fn_content = f.read()
    match_ids = json.loads(fn_content)
    #
    generator=Assist.ImageReportGenereator()

    generator.addSource(["../report0/img/","", ".jpg"], "照片")
    generator.addSource(["../report0/"+report_dir+"/0-1/deform/","", ".jpg"], "c1(详见各行说明)")
    generator.addSource(["../report0/"+report_dir+"/0-1/proxy/","", ".jpg"], "c2(详见各行说明)")
    generator.addSource(["../report0/img/","", ".jpg", FilenameMapping], "proxy原图")
    generator.addSource(["../report0/"+report_dir+"/landmark/","", ".jpg"], "特征点（绿2d，红3d）")
    generator.addSource(["../report0/"+report_dir+"/45/deform/","", ".jpg"],"c1的45°视角")
    generator.addSource(["../report0/"+report_dir+"/90/deform/","", ".jpg"],"c1的90°视角")

    generator.withHeader(True)
    generator.withDescription("说明", 0.07, {"font_size":0.03})
    generator.setSortKeyGetter(SortKey)
    generator.setDescriptionMap(description_mapping)
    generator.setIgnorePrefix(escape_fn_prefix)

    shutil.copy("report0.py",  "../report0/"+report_dir+"/")
    out_json="../report0/"+report_dir+"/新风格结果对比"+report_dir+".json"
    generator.generate(out_json)