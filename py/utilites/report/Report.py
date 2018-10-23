#coding=utf-8
# author: yixuan, lu

# pip3 install ReportLab

from reportlab.pdfgen import canvas
from reportlab.lib.utils import ImageReader
import reportlab.lib.pagesizes as PageSizes
import reportlab.lib.colors as Colors
import reportlab.lib.units as Units
import reportlab.pdfbase.ttfonts
import os

sys_font_dir="C:/Windows/Fonts/"
font_name_mapping={
    "等线 常规":"Deng.ttf",
    "等线 粗体":"Dengb.ttf",
    "等线 细体": "Dengl.ttf",
    "方正等线": "DengXian.ttf",
    "方正兰亭超细黑简体": "FZLTCXHJW.TTF",
    "方正舒体": "FZSTK.TTF",
    "方正姚体": "FZYTK.TTF",
    "仿宋": "simfang.ttf",
    "黑体":"simhei.ttf",
    "华文彩云": "STCAIYUN.TTF",
    "华文仿宋": "STFANGSO.TTF",
    "华文行楷": "STXINGKA.TTF",
    "华文琥珀": "STHUPO.TTF",
    "华文楷体": "STKAITI.TTF",
    "华文隶书": "STLITI.TTF",
    "华文宋体": "STSONG.TTF",
    "华文细黑": "STXIHEI.TTF",
    "华文新魏": "STXINWEI.TTF",
    "华文中宋": "STZHONGS.TTF",
    "楷体": "simkai.ttf",
    "隶书": "SIMLI.TTF",
    "宋体": "simsun.ttc",
    "微软雅黑 常规": "msyh.ttc",
    "微软雅黑 粗体": "msyhbd.ttc",
    "微软雅黑 细体": "msyhl.ttc",
    "新宋体": "simsun.ttc",
    "幼圆": "SIMYOU.TTF",
}

def importFont(name, path):
    # print(name, " ",path)
    reportlab.pdfbase.pdfmetrics.registerFont(reportlab.pdfbase.ttfonts.TTFont(name, path))

class Doc:
    'base'
    def __init__(self, doc_path, page_size=PageSizes.A4):
        self.canvas=canvas.Canvas(doc_path, bottomup=1)
        self.w,self.h=page_size
        self.w_r=1.0
        self.h_r=1.0
        self.canvas.setPageSize(page_size)
    def newPage(self):
        self.canvas.showPage()
    def getDoc(self):
        return self.canvas
    def saveDoc(self):
        self.canvas.save()
# set params:
# { 'size':[300,300], "pos":[50, 100], "is_abs":True, "padding":[0,0] }
class ReportElement:
    'report elememt'
    def __init__(self, par):
        self.is_abs=True
        self.w=0
        self.h=0
        self.w_r=0
        self.h_r=0
        self.padding=[0.0, 0.0]
        self.padding_r=[0.0, 0.0]
        self.pos=[0,0]
        self.pos_r=[0,0]
        self.par=par
        self.child=[]
        if hasattr(par, 'child'):
            par.child.append(self)
    def set(self, config):
        if "parent" in config:
            self.setParent(config["parent"])
        if "is_abs" in config:
            self.setAbs(config["is_abs"])
        if "size" in config:
            self.setSize(config["size"])
        if "pos" in config:
            self.setPos(config["pos"])
        if "padding" in config:
            self.setPadding(config["padding"])
    def setSize(self,size):
        px,py,pw,ph=self.getParentRect()
        if self.getVal("is_abs", True) == True:
            self.w, self.h=size
            self.w_r=self.w/pw
            self.h_r=self.h/ph
        else:
            self.w_r, self.h_r=size
            self.w=self.w_r*pw
            self.h=self.h_r*ph
    def setParent(self, par):
        if hasattr(self.par, "child"):
            for i in range(0, len(self.par.child)):
                if self.par.child[i]==self:
                    self.par.child.pop(i)
        self.par = par
        if hasattr(par, 'child'):
            par.child.append(self)
        # change parent, recompute size
        if self.is_abs==False:
            self.setSize([self.w_r, self.h_r])
            self.setPos(self.pos_r)
            self.setPadding(self.padding_r)
        for ch in self.child:
            ch.setParent(self)
    def setPos(self, pos):
        px, py, pw, ph = self.getParentRect()
        if self.getVal("is_abs", True) == True:
            self.pos=pos
            self.pos_r=[self.pos[0]/pw, self.pos[1]/ph]
        else:
            self.pos_r=pos
            self.pos=[self.pos_r[0]*pw, self.pos_r[1]*ph]
    def setAbs(self, is_abs):
        self.is_abs=is_abs
    def setPadding(self, padding):
        px, py, pw, ph = self.getParentRect()
        if self.getVal("is_abs", True) == True:
            self.padding=padding
            self.padding_r=[self.padding[0]/pw, self.padding[1]/ph]
        else:
            self.padding_r=padding
            self.padding=[self.padding_r[0]*pw, self.padding_r[1]*ph]
    def getParentRect(self):
        if hasattr(self.par, "par"):
            xx, yy, ww, hh=self.par.getParentRect()
        else:
            xx=0
            yy=0
        w = self.par.w
        h = self.par.h
        if hasattr(self.par, 'pos'):
            x = self.par.pos[0]
            y = self.par.pos[1]
        else:
            x = 0
            y = 0
        return [x+xx, y+yy, w, h]
    def getRect(self, y_up):
        x,y,w,h=self.getParentRect()
        if y_up==False:
            return [x + self.pos[0], (x + self.pos[1]), self.w, self.h]
        else:
            return [x+self.pos[0], h-(x+self.pos[1]+self.h), self.w, self.h]
    def getDrawRect(self, doc):
        x,y,w,h=self.getParentRect()
        ww=self.w-self.padding[0]*2.0
        hh=self.h-self.padding[1]*2.0
        xx=x+self.pos[0]+self.padding[0]
        yy=doc.h - (y+self.pos[1]+self.padding[1] + hh)
        return [xx, yy, ww, hh]
    def getVal(self, attr, def_val):
        if hasattr(self, attr):
            t = ''.join(['self.', attr])
            return eval(t)
        else:
            return def_val
    def getAbsRelativeVal(self, attr, def_val_abs, def_val_relative, scale_is_relative):
        if self.is_abs:
            ret = self.getVal(attr, def_val_abs)
        else:
            ret = self.getVal(attr, def_val_relative)*scale_is_relative
        return ret
    def drawChild(self, doc):
        if hasattr(self, 'child'):
            for ch in self.child:
                ch.draw(doc)
class RepEleNewPage(ReportElement):
    "new page"
    def __init__(self, par):
        ReportElement.__init__(self, par)
    def draw(self, doc, is_draw_child=True):
        doc.newPage()
# set params:
# { "has_border":True, "border_color":Report.Colors.red, "border_width":5, "border_expand":0, "is_fill":True, "fill_color":Report.Colors.yellowgreen, "is_round": True, "round_radius":5}
class RepEleRect(ReportElement):
    'rect'
    def __init__(self, par):
        ReportElement.__init__(self, par)
        self.has_border=True
        self.is_fill=False
    def set(self, config):
        if "has_border" in config:
            self.setBorder(config["has_border"])
        if "border_color" in config:
            self.setStrokeColor(config["border_color"])
        if "border_width" in config:
            self.setBorderWidth(config["border_width"])
        if "border_expand" in config:
            self.setBorderExpand(config["border_expand"])
        if "is_fill" in config:
            self.setFill(config["is_fill"])
        if "fill_color" in config:
            self.setFillColor(config["fill_color"])
        if "is_round" in config and "round_radius" in config:
            self.setRound(config["is_round"], config["round_radius"])
        ReportElement.set(self, config)
    def setBorder(self, has_border):
        self.has_border=has_border
    def setStrokeColor(self, color):
        self.border_color=color
    def setBorderWidth(self, width):
        self.border_width=width
    # <0 inner
    # =0 in/out
    # >0 outter
    def setBorderExpand(self, expand):
        self.border_expand=expand
    def setFill(self, is_fill):
        self.is_fill=is_fill
    def setFillColor(self, color):
        self.fill_color=color
    def setRound(self, is_round, round_radius):
        self.is_round=is_round
        self.round_radius=round_radius
    def getBorderWidth(self):
        return self.getAbsRelativeVal("border_width", 1, 0.001, self.par.w)
    def getBorderRadius(self):
        return self.getAbsRelativeVal("round_radius", 3, 0.03, self.par.w)
    def draw(self, doc, is_draw_child=True):
        # print("ReportRect", self.getParentRect(), self.getDrawRect(doc))
        x, y, w, h=self.getDrawRect(doc)
        _doc=doc.getDoc()
        border_width=self.getBorderWidth()
        _doc.setLineWidth(border_width)
        c=self.getVal('fill_color', Colors.white)
        _doc.setFillColor(self.getVal('fill_color', Colors.white))
        _doc.setStrokeColor(self.getVal('border_color', Colors.black))
        border_expand=self.getVal('border_expand', 0)
        if border_expand < 0:
            border_expand=-1
        elif border_expand >0:
            border_expand=1
        if hasattr(self, 'is_round') and self.is_round==True:
            radius=self.getBorderRadius()
            _doc.roundRect(x - border_expand*border_width * 0.5, y - border_expand * border_width * 0.5, w + border_expand * border_width, h + border_expand * border_width, radius,
                           fill=self.is_fill, stroke=self.has_border)
        else:
            _doc.rect(x - border_expand * border_width * 0.5, y - border_expand * border_width * 0.5, w + border_expand * border_width, h + border_expand * border_width,
                      fill=self.is_fill, stroke=self.has_border)
        if is_draw_child:
            self.drawChild(doc)
# set params:
# { "rows":[1,2,1], "cols":[2,2,4], "line_width": 3, "line_color":Report.Colors.blueviolet }
class RepEleGrid(RepEleRect):
    'grid'
    def __init__(self, par):
        RepEleRect.__init__(self, par)
        self.rows=[1]
        self.cols=[1]
    def set(self, config):
        if "rows" in config:
            self.setRows(config["rows"])
        if "cols" in config:
            self.setCols(config["cols"])
        if "line_width" in config:
            self.setLineWidth(config["line_width"])
        if "line_color" in config:
            self.setLineColor(config["line_color"])
        RepEleRect.set(self, config)
    def setRows(self, rows):
        self.rows=rows
    def setCols(self, cols):
        self.cols=cols
    def setLineWidth(self, width):
        self.line_width=width
    def setLineColor(self, color):
        self.line_color=color
    def getLineWidth(self):
        return self.getAbsRelativeVal("line_width", 1, 0.001, self.par.w)
    def draw(self, doc, is_draw_child=True):
        # print("ReportGrid", self.getParentRect(), self.getDrawRect(doc))
        RepEleRect.draw(self, doc, False)

        x,y,w,h=self.getDrawRect(doc)
        _doc=doc.getDoc()
        border_width = self.getBorderWidth()
        expand_dir = 1-self.getVal('border_expand', 0)
        border_width_inner = border_width*expand_dir*0.5
        line_width=self.getLineWidth()
        line_color=self.getVal('line_color', Colors.black)
        _doc.setLineWidth(line_width)
        _doc.setStrokeColor(line_color)
        #
        inner_width=w-border_width_inner*2
        inner_height=h-border_width_inner*2

        xx=x+border_width_inner
        yy=y+h-border_width_inner
        # horizontal line
        acc=0
        offset=0
        for i in self.rows:
            acc=acc+i
        for i in self.rows:
            offset = offset + i*inner_height/acc
            if offset < inner_height:
                _doc.line(xx, yy-offset, xx+inner_width, yy-offset)
        # vertical line
        acc = 0
        offset = 0
        for i in self.cols:
            acc = acc + i
        for i in self.cols:
            offset = offset + i*inner_width/acc
            if offset < inner_width:
                _doc.line(xx+offset, yy, xx+offset, yy-inner_height)
        # draw border if necessary
        if self.has_border == True:
            is_fill=self.is_fill
            self.is_fill=False
            RepEleRect.draw(self, doc, False)
            self.is_fill=is_fill
        # children
        if is_draw_child:
            self.drawChild(doc)
# set params:
# {'size': [1.0, 0.5], "pos": [0.0, 0.25], "is_abs": False, "padding": [0.0, 0.0], "has_border": True, "border_color": Report.Colors.red, "border_width": 0.01, "border_expand": 0, "img_path": "", "keep_aspect":True}
class RepEleImage(ReportElement):
    'image'
    def __init__(self, par):
        ReportElement.__init__(self, par)
        self.border=RepEleRect(self)
        self.border.setBorder(False)
    def set(self, config):
        if "img_path" in config:
            self.setImage(config["img_path"])
        if "has_border" in config:
            self.border.setBorder(config["has_border"])
        if "border_color" in config:
            self.border.setStrokeColor(config["border_color"])
        if "border_width" in config:
            self.border.setBorderWidth(config["border_width"])
        if "border_expand" in config:
            self.border.setBorderExpand(config["border_expand"])
        if "is_abs" in config:
            self.border.setAbs(config["is_abs"])
        if "keep_aspect" in config:
            self.setAspect(config["keep_aspect"])
        ReportElement.set(self, config)
    def setImage(self, img_path):
        if os.path.exists(img_path) == False:
            return
        self.img_path=img_path
        self.img=ImageReader(img_path)
        self.img_size=self.img.getSize()
    def setBorder(self, has_border):
        self.has_border=has_border
    def setStrokeColor(self, color):
        self.border_color=color
    def setBorderWidth(self, width):
        self.border_width=width
    def setAspect(self, is_keep):
        self.is_keep_aspect=is_keep
    def getBorderWidth(self):
        return self.getAbsRelativeVal("border_width", 1, 0.01, self.par.w)
    def draw(self, doc, is_draw_child=True):
        # draw border
        # print("ReportImage", self.getParentRect(), self.getDrawRect(doc))
        x, y, w, h = self.getDrawRect(doc)

        if hasattr(self, "img")==True:
            _doc = doc.getDoc()
            if self.getVal("is_keep_aspect", True)==True:
                r0=self.img._width/self.img._height
                r1=w/h
                if r0 < r1:
                    w2=r0*h
                    x=x+(w-w2)/2
                    w=w2
                else:
                    h2=w/r0
                    y=y+(h-h2)/2
                    h=h2
            _doc.drawImage(self.img, x, y, w, h)

        if is_draw_child:
            self.border.setPos([0,0])
            if self.is_abs:
                self.border.setSize([w, h])
            else:
                self.border.setSize([1.0,1.0])
            self.drawChild(doc)

class RepEleText(ReportElement):
    'text'
    def __init__(self, par):
        ReportElement.__init__(self, par)
        self.text=""
        self.font_size=12
        self.fill_gray=1.0
        self.fill_color=Colors.black
        self.char_space=0
        self.word_space=0
        self.horizontal_sacle=100
        self.fill_alpha=1
    def set(self, config):
        if "text" in config:
            self.setText(config["text"])
        if "font" in config:
            self.setFont(config["font"])
        if "font_size" in config:
            self.setFontSize(config["font_size"])
        if "fill_gray" in config:
            self.setFillGray(config["fill_gray"])
        if "fill_color" in config:
            self.setFillColor(config["fill_color"])
        if "fill_alpha" in config:
            self.setFillAlpha(config["fill_alpha"])
        if "char_space" in config:
            self.setCharSpace(config["char_space"])
        if "word_space" in config:
            self.setWordSpace(config["word_space"])
        if "horizontal_scale" in config:
            self.setHorizontalScale(config["horizontal_scale"])
        if "interline_space" in config:
            self.setInterlineSpace(config["interline_space"])
        if "rise" in config:
            self.setRise(config["rise"])
        ReportElement.set(self, config)
    def setText(self, text):
        self.text=text
    def setFont(self, font):
        self.font=font
    def setFontSize(self, size):
        if size>0:
            self.font_size=size
    def setFillGray(self, gray):
        if gray<=0:
            self.fill_gray=0
        elif gray>=1.0:
            self.fill_gray=1.0
        else:
            self.fill_gray=gray
    def setFillColor(self, color):
        self.fill_color=color
    def setFillAlpha(self, alpha):
        if alpha<0.0:
            self.fill_alpha=0.0
        elif alpha>1.0:
            self.fill_alpha=1.0
        else:
            self.fill_alpha=alpha
    def setCharSpace(self, space):
        if space<=0:
            self.char_space=0
        else:
            self.char_space=space
    def setWordSpace(self, space):
        if space<=0:
            self.word_space=0
        else:
            self.word_space=space
    def setHorizontalScale(self, scale):
        if scale<0:
            self.horizontal_sacle=0
        else:
            self.horizontal_sacle=scale
    def setInterlineSpace(self, space):
        self.leading_ofs=space
    def setRise(self, rise):
        self.rise=rise
    def getAllFonts(self, doc):
        return doc.getDoc().getAvailableFonts()
    def getFont(self, doc):
        font=self.getVal("font", "Times-Roman")
        try:
            reportlab.pdfbase.pdfmetrics.getFont(font)
            return font
        except Exception  as e:
            if font in font_name_mapping and os.path.exists(sys_font_dir+font_name_mapping[font]):
                importFont(font, sys_font_dir+font_name_mapping[font])
                return font
            else:
                return "Times-Roman"
    def getStringWidth(self, doc, str, font, font_size):
        len=doc.getDoc().stringWidth(str, font, font_size)
        for c in str:
            if c==' ':
                len=len+self.word_space
            len = len+self.char_space
        len=len-self.char_space
        len = int(len*self.horizontal_sacle/100.0)
        return len
    # def setTextTransform(self):
    #     pass
    def splitStringToFitSize(self, doc, str, font, font_size, max_width):
        ret=[]
        start_pos=0
        end_pos = len(str)
        slen=len(str)
        while start_pos != end_pos:
            while self.getStringWidth(doc, str[start_pos:end_pos], font, font_size)>max_width:
                end_pos = int((end_pos - start_pos)/2+start_pos)
            while end_pos != slen and self.getStringWidth(doc, str[start_pos:end_pos+1], font, font_size)<max_width:
                end_pos=end_pos+1
            # if str[end_pos-1] != ' ' and end_pos != len(str):
            #     --end_pos;
            #     ret.append(str[start_pos:end_pos]+"-")
            # else:
            ret.append(str[start_pos:end_pos])
            start_pos=end_pos
            end_pos=slen
        return ret

    def draw(self, doc, is_draw_child=True):
        # print("ReportString", self.getParentRect(), self.getDrawRect(doc))
        x, y, w, h = self.getDrawRect(doc)
        _doc=doc.getDoc()
        font=self.getFont(doc)
        font_size=self.getAbsRelativeVal("font_size", 9, 0.01, self.par.h)
        # font_size=self.font_size
        y = y + h - font_size
        to=_doc.beginText()
        to.setTextOrigin(x, y)

        leading=font_size*1.2

        # to.setTextTransform(a,b,c,d,e,f)
        to.setFont(font,font_size)
        to.setFillGray(self.fill_gray)
        to.setFillColor(self.fill_color, self.fill_alpha)
        to.setCharSpace(self.char_space)
        to.setWordSpace(self.word_space)
        to.setHorizScale(self.horizontal_sacle)
        if hasattr(self, "leading_ofs"):
            leading=self.getVal("leading_ofs", -100)
            to.setLeading(leading)
        if hasattr(self, "rise"):
            to.setRise(self.rise)
        # positive y move down!!!
        # to.moveCursor(12, 12)

        #
        cur_h=0
        lines=self.text.split(u'\n')
        lines_to_draw=[]
        if w >= font_size:
            for line in lines:
                tmp=self.splitStringToFitSize(doc, line, font, font_size, w)
                for s in tmp:
                    if cur_h+leading<h:
                        lines_to_draw.append(s)
                        cur_h=cur_h+leading
        to.textLines(lines_to_draw)

        _doc.drawText(to)

        if is_draw_child:
            self.drawChild(doc)