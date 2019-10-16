#coding=utf-8
# author: yixuan, lu

# import sys
# sys.path.append(".")

from . import Report
import json
import copy
import os

class Json2PDF:
    def LoadFile(path):
        with open(path) as f:
            return f.read()

    def Build(self, config, par):
        t = config["type"]
        if "rect" == t:
            ret = Report.RepEleRect(par)
        elif "grid" == t:
            ret = Report.RepEleGrid(par)
        elif "text" == t:
            ret = Report.RepEleText(par)
        elif "image" == t:
            ret = Report.RepEleImage(par)
        elif "new_page" == t:
            ret = Report.RepEleNewPage(par)
        else:
            return
        for s in config["setting"]:
            if s.endswith("_color"):
                v = config["setting"][s]
                config["setting"][s] = Report.Colors.Color(v[0], v[1], v[2])
        ret.set(config["setting"])
        ret.setSize(config["size"])
        ret.setPos(config["pos"])
        if "child" in config:
            for c in config["child"]:
                self.Build(c, ret)
        return ret

    def DrawJson(self, json_content, doc):
        ret = json.loads(json_content)
        # print(ret)
        # for ele in ret:
        if "type" in ret:
            v = self.Build(ret, doc)
            v.draw(doc)
        elif "pages" in ret:
            for p in ret["pages"]:
                v = self.Build(p, doc)
                v.draw(doc)

    def DrawJsonFromFile(self, path, doc):
        self.DrawJson(self.LoadFile(path), doc)

class ReportRow:
    "report as row"
    def __init__(self):
        self.child=[]
    def getDefaultSetting(self, type):
        ret={"is_abs":False, "padding":[0.002, 0.002]}
        if "text"==type:
            ret["font"]="宋体"
            ret["font_size"]=0.1
        elif "image"==type:
            ret["padding"]=[0.001, 0.009]
        elif "grid"==type:
            ret["border_expand"]=0
            ret["border_width"]=0.001
        return ret
    def setHight(self, h):
        self.h=h
    def add(self, config):
        available_type=["text", "image", "rect", "grid"]
        if "type" not in config or config["type"] not in available_type or "size" not in config:
            return
        cur={}
        cur["type"]=config["type"]
        cur["size"]=config["size"]
        cur["setting"]=self.getDefaultSetting(cur["type"])
        for n in config:
            if n == "type" or n =="size":
                continue
            cur["setting"][n]=config[n]
        self.child.append(cur)
    def getResult(self):
        if len(self.child)==0:
            return {}
        w_org=0
        cols=[]
        for c in self.child:
            w_org = w_org+c["size"][0]
            cols.append(c["size"][0])
        if w_org>1.0:
            scale=1.0/w_org
        else:
            scale = 1.0
            cols.append(1.0-w_org)
        x=0.0
        for c in self.child:
            w=c["size"][0] * scale
            # h=c["size"][1] * scale
            h=c["size"][1]
            c["size"] = [w, h]
            c["pos"]=[x, (1.0-h)/2]
            x=x+w
        ret={"type":"grid"}
        ret["setting"]=self.getDefaultSetting("grid")
        ret["setting"]["cols"]=cols
        ret["setting"]["padding"]=[0,0]
        ret["size"]=[1.0, self.h]
        ret["child"]=self.child
        # set incase direct draw
        ret["pos"]=[0,0]
        return ret

class ReportFile:
    "report"
    def __init__(self):
        self.rows=[]
        self.padding=[0.03,0.03]
    def setPadding(self, padding):
        self.padding=padding
    def addRow(self, row):
        self.rows.append(row)
    def setHeader(self, header):
        self.header=header
    def newPage(self):
        page = {"type": "rect", "pos":self.padding, "size":[1.0-self.padding[0]*2, 1.0-self.padding[1]*2], "child":[]}
        page["setting"] = {"is_abs":False, "has_border": True, "border_color":[100, 12, 56]}
        h=0
        if hasattr(self, 'header'):
            page["child"].append(copy.deepcopy(self.header))
            h = self.header['size'][1]
        return [page,h]
    def getResult(self):
        ret={"pages":[]}
        [page,h]=self.newPage()
        for _r in self.rows:
            r=copy.deepcopy(_r)
            if r["size"][1]+h > 1.0:
                page["child"].append({"type":"new_page", "setting":{}, "pos":[0,0],"size":[0,0]})
                ret["pages"].append(page)
                [page,h]=self.newPage()
            r["pos"]=[0, h]
            page["child"].append(r)
            h=h+r["size"][1]
        if h>0:
            ret["pages"].append(page)
        return ret

def textTemplate2Json(fn):
    ratio=[]
    map={"text":"text", "image":"img_path"}
    with open(fn, 'r') as f:
        # read row height
        line = f.readline()
        row_height=float(line)
        # read column ratio
        line = f.readline()
        t=line.split()
        for v in t:
            ratio.append(float(v))
        line = f.readline()
        type=line.split()

        lines=f.readlines()
        pages = ReportFile()
        r=range(0, len(lines), len(ratio))
        for idx in range(0, len(lines), len(ratio)):
            r=ReportRow()
            r.setHight(row_height)
            for i in range(0, len(ratio)):
                val=lines[idx+i]
                if "\n"==val[-1]:
                    val=val[:-1]
                if "image" == type[i] and os.path.exists(val) == False:
                    val = os.path.join(os.path.split(fn)[0], val)
                tmp={"type":type[i],"size":[ratio[i], 1.0], map[type[i]]:val}
                r.add(tmp)
            pages.addRow(r.getResult())
        return json.dumps(pages.getResult())

def jsonTemplate2Json(fn):
    with open(fn, 'r') as f:
        fn_content = f.read()
    values=json.loads(fn_content)
    row_height=values["height"]
    ratio=values["ratio"]
    type=values["type"]
    if "setting" in values:
        setting=values["setting"]
    else:
        setting=[]
        for i in range(0, len(ratio)):
            setting.append({})

    rows=values["rows"]
    pages = ReportFile()

    scale_h = 1.0
    if "header" in values:
        scale_h = 0.97
        r = ReportRow()
        r.setHight(0.03)
        for i in range(0, len(ratio)):
            tmp = {"type": "text", "size": [ratio[i], 1.0], "text": values["header"][i], "font_size":0.6}
            r.add(tmp)
        pages.setHeader(r.getResult())

    map = {"text": "text", "image": "img_path"}
    for row in rows:
        r = ReportRow()
        r.setHight(row_height*scale_h)
        for i in range(0, len(ratio)):
            if "image"==type[i] and os.path.exists(row[i])==False:
                row[i]=os.path.join(os.path.split(fn)[0],row[i])
            tmp = {"type": type[i], "size": [ratio[i], 1.0], map[type[i]]: row[i]}
            for str in setting[i]:
                tmp[str]=setting[i][str]
            r.add(tmp)
        pages.addRow(r.getResult())
    return json.dumps(pages.getResult())

def jsonTemplate2Pdf(fn, page_size=Report.PageSizes.A4):
    temp = jsonTemplate2Json(fn)
    fn_pdf = os.path.splitext(fn)[0]+".pdf"
    doc = Report.Doc(fn_pdf, page_size)
    drawer = Json2PDF()
    print("drawing...")
    drawer.DrawJson(temp, doc)
    print("saving: %s"%fn_pdf)
    doc.saveDoc()




class ImageReportGenereator:
    "ReportGenereator"
    def __init__(self):
        self.row_height=0.5
        self.ratio=[]
        self.type=[]
        self.img_dir=[]
        self.with_description=False
        self.header=[]
        self.col_type=[]
        self.col_ratio=[]
        self.col_setting=[]
        self.ignore_prefix=[]
    # img_dir=["dir", "postfix", "extension", "name mapping"]
    def addSource(self, img_dir, header=None, setting={}):
        self.img_dir.append(img_dir)
        self.col_setting.append(setting)
        self.col_type.append("image")
        self.header.append(header)
    def withHeader(self, with_header):
        self.with_header=with_header
    def withDescription(self, header="", ratio=0.07, setting={}):
        self.with_description=True
        self.col_type.insert(0, "text")
        self.col_setting.insert(0, setting)
        self.col_ratio.insert(0, ratio)
        self.header.insert(0, header)
    def setIgnorePrefix(self, prefix):
        self.ignore_prefix=prefix
    def setSortKeyGetter(self, getter):
        self.sort_key_getter=getter
    def setDescriptionMap(self, map):
        self.description_map=map
    def setRowHeight(self, row_height):
        self.row_height=row_height

    def isIgnore(self, fn):
        for m in self.ignore_prefix:
            if fn[0:len(m)] == m:
                return True
        return False
    def toDescription(self, fn):
        for m in self.description_map:
            if fn[0:len(m)] == m:
                return self.description_map[m] + fn[len(m):]
        return fn

    def generate(self, out_json, page_w=1070, page_h=2200):
        if self.with_description:
            w=(1.0-self.col_ratio[0])/len(self.img_dir)
        else:
            w=1.0/len(self.img_dir)
        for i in range(0, len(self.img_dir)):
            self.col_ratio.append(w)

        #basic
        ret = {}
        ret['height'] = self.row_height
        ret['ratio'] = self.col_ratio
        ret['type'] = self.col_type
        if self.with_header:
            ret['header'] = self.header
        ret['setting'] = self.col_setting
        #image list
        imgs = []
        for fn in os.listdir(self.img_dir[0][0]):
            if self.img_dir[0][2] == os.path.splitext(fn)[1]:
                imgs.append(fn)
        if hasattr(self, "sort_key_getter"):
            imgs.sort(key=self.sort_key_getter)
        #
        rows = []
        idx = 0
        for fn in imgs:
            if self.isIgnore(fn):
                continue
            idx = idx + 1
            row = []
            tmp = os.path.splitext(fn)
            row.append(self.toDescription(tmp[0]))
            for dir in self.img_dir:
                #use mapper
                if len(dir) == 4:
                    tmp2 = os.path.join(dir[0], dir[3](tmp[0]) + dir[1] + dir[2])
                    if os.path.isfile(tmp2):
                        row.append(tmp2)
                        continue
                #use same name
                tmp2 = os.path.join(dir[0], tmp[0] + dir[1] + dir[2])
                if os.path.isfile(tmp2):
                    row.append(tmp2)
                    continue
                #use counter
                tmp2 = os.path.join(dir[0], '%03d' % idx + dir[1] + dir[2])
                if os.path.isfile(tmp2):
                    row.append(tmp2)
                    continue
                row.append("")
            rows.append(row)
        ret['rows'] = rows
        #
        with open(out_json, 'w') as f:
            f.write(json.dumps(ret))
        jsonTemplate2Pdf(out_json, Report.PageSizes.landscape((Report.mm * page_w, Report.mm * page_h)))
