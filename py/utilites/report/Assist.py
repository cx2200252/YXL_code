import utilites.report.Report as Report
import json

def LoadFile(path):
    with open(path) as f:
        return f.read()

def Build(config, par):
    t=config["type"]
    if "rect" == t:
        ret=Report.RepEleRect(par)
    elif "grid" == t:
        ret=Report.RepEleGrid(par)
    elif "text" == t:
        ret=Report.RepEleText(par)
    elif "image" == t:
        ret=Report.RepEleImage(par)
    else:
        return
    ret.set(config["setting"])
    if "child" in config:
        for c in config["child"]:
            Build(c, ret)
    return ret


def DrawJson(json_content, doc):
    ret = json.loads(json_content)
    # print(ret)
    for ele in ret:
        v = Build(ret[ele], doc)
        v.draw(doc)

def DrawJsonFromFile(path, doc):
    DrawJson(LoadFile(path), doc)