import os
import json

def GenJson(row_height, col_ratio, col_type, img_dir):
    ret={}
    ret['height']=row_height
    ret['ratio']=col_ratio
    ret['type']=col_type

    imgs=[]
    for fn in os.listdir(img_dir[0][0]):
        if img_dir[0][2] == os.path.splitext(fn)[1]:
            imgs.append(fn)

    rows=[]
    for fn in imgs:
        row=[]
        tmp=os.path.splitext(fn)
        row.append(tmp[0])
        for dir in img_dir:
            row.append(os.path.join(dir[0],tmp[0]+dir[1]+dir[2]))
        rows.append(row)
    ret['rows']=rows
    return json.dumps(ret)