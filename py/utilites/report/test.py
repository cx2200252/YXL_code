
from . import Report
from . import Assist

def testGrid(doc):
    rect = Report.RepEleRect(doc)
    config_rect = {'size': [0.2, 0.2], "pos": [0, 0], "is_abs": False, "padding": [0, 0], "has_border": True,
                   "border_color": Report.Colors.red, "border_width": 0.01, "border_expand": -1,
                   "is_fill": True, "fill_color": Report.Colors.yellowgreen}
    rect.set(config_rect)
    #
    rect2 = Report.RepEleRect(doc)
    rect2.set(config_rect)
    rect2.set({"size": [0.5, 0.5], "pos": [0.3, 0]})

    config_rect2 = {'size': [0.2, 0.2], "pos": [0.4, 0.4], "is_abs": False, "padding": [0, 0], "has_border": True,
                    "border_color": Report.Colors.aliceblue, "border_width": 0.01, "border_expand": -1,
                    "is_fill": True, "fill_color": Report.Colors.blueviolet}
    rect3 = Report.RepEleRect(rect)
    rect3.set(config_rect2)
    #
    # # # x, y, w, h =rect.draw(doc)
    # #
    grid = Report.RepEleGrid(rect2)

    config_grid = {'size': [0.8, 0.8], "pos": [0.1, 0.1], "is_abs": False, "padding": [0.1, 0], "has_border": True,
                   "border_width": 0.01,
                   "border_color": Report.Colors.red, "border_expand": 0,
                   "is_fill": True, "fill_color": Report.Colors.aliceblue, "is_round": True, "round_radius": 0.05,
                   "rows": [1, 2, 1], "cols": [2, 2, 4], "line_width": 0.02, "line_color": Report.Colors.blueviolet, }
    grid.set(config_grid)
    rect.draw(doc)
    # rect3.setParent(rect2)
    rect2.draw(doc)

def testImg(doc):
    config_img = {'size': [0.5, 0.2], "pos": [0.0, 0.25], "is_abs": False, "padding": [0.0, 0.0], "has_border": False,
                  "border_color": Report.Colors.red, "border_width": 0.01, "border_expand": 0,
                  "img_path": "G:/C++/p2a/p2a_master_check/p2aBenchmark/andriodBenchmark/pushFiles/nv/26.png",
                  "keep_aspect": True}
    img = Report.RepEleImage(doc)
    img.set(config_img)
    img.draw(doc)

def testText(doc):
    rect = Report.RepEleRect(doc)
    size=[300, 100]
    pos=[0, 0]

    config_rect = {'size':size, "pos": pos, "is_abs": True, "has_border": False,"is_fill": True, "fill_color": Report.Colors.aliceblue}
    rect.set(config_rect)
    rect.draw(doc)

    Report.importFont("华文行楷","C:/Program Files (x86)/Microsoft Office/root/VFS/Fonts/private/STXINGKA.TTF")

    config_txt={"size":size, "pos":pos, "is_abs":True, "font_size":9, "font":"华文行楷","word_space": 3}
    txt=Report.RepEleText(doc)
    txt.set(config_txt)
    # txt.setText("well she hit Net Solutions and she registered her own .com site now and filled it up with yahoo  rofile pics she snarfed in one night now and she made 50 million when Hugh Hefner bought up the rights now and she'll have fun fun fun til her Daddy takes the keyboard away With many apologies to the Beach Boys and anyone else who finds this objectionable")
    txt.setText("reportlab不是python的标准库，它的强大之处在于能满足绝大部分报表的需求形式。")
    txt.draw(doc)

if __name__=="__main__":
    # temp = Assist.textTemplate2Json("test/test.txt")
    Assist.jsonTemplate2Pdf("G:/C++/p2a/p2a/p2a_client/result/rep/report.json")
