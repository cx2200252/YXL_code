
from report import Assist
from report import Report
import func

row_height=0.5
# col_ratio=[0.04, 0.24, 0.24, 0.24, 0.24]
# col_type=["text", "image", "image", "image", "image"]

# col_ratio=[0.04, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24]
col_ratio=[0.04, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12]
col_type=["text", "image", "image", "image", "image", "image", "image", "image", "image"]

# ["dir","postfix","extension"]
img_dir=[
    ["../report0/img/","", ".jpg"],
    ["../report0/new_proxy/0/","", ".png"],
    ["../report0/old_proxy_old_render/0/","", ".png"],
    ["../report0/zepeto/","", ".png"],
    ["../report0/new_proxy/45/","", ".png"],
    ["../report0/old_proxy_old_render/45/","", ".png"],
    ["../report0/new_proxy/90/","", ".png"],
    ["../report0/old_proxy_old_render/90/","", ".png"]
    ]

if __name__=="__main__":
    json_str=func.GenJson(row_height, col_ratio, col_type, img_dir)
    with open("../report0/report.json", 'w') as f:
        f.write(json_str)

    Assist.jsonTemplate2Pdf("../report0/report.json",Report.PageSizes.landscape((Report.mm*5000, Report.mm*11000)))
