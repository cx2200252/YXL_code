# basic elements
- document

```C
import report.Report as Report

doc = Report.Doc("A4.pdf", Report.PageSizes.A4)

//do something

doc.saveDoc()
```

- ReportElement
  - base class for all element
  - params
    - parent: Report.Doc or Report.ReportElement and it's sub-class
    - is_abs: coordinate system, relative or absolute
    - size: [w, h], metric relative to "is_abs"
    - pos: [x, y], metric relative to "is_abs"
    - padding: [horizontal, vertical], metric relative to "is_abs"

```C
ele=Report.ReportElement(doc)
// absolute position
ele.set({ 'size':[300,300], "pos":[50, 100], "is_abs":True, "padding":[5,5] })
// relative position
// at center, 30% width of parent's width, 40% width of parent's height
ele.set({ 'size':[0.5,0.5], "pos":[0.25, 0.25], "is_abs":True, "padding":[0.1,0.05] })
```

- RepEleRect
  - params
    - has_border: draw border or not
    - border_color: border's color, type of Request.Colors.color
    - border_width: border's width, metric relative to "is_abs"
    - border_expand: 
      - < 0: inside
      - = 0: half outside, half inside
      - \> 0: outside
    - is_fill: draw background or not
    - fill_color: background's color
    - is_round: 
    - round_radius: metric relative to "is_abs"
    - see ReportElement

```C
rect = Report.RepEleRect(doc)
config_rect = {'size': [0.5, 0.5], "pos": [0, 0], "is_abs": False, 
	"padding": [0, 0], "has_border": True, "border_color": Report.Colors.red,
	"border_width": 0.01, "border_expand": -1, "is_fill": True,
	"fill_color": Report.Colors.yellowgreen}
rect.set(config_rect)
rect.draw(doc)
```

- RepEleGrid
  - params
    - rows: ratio among rows
    - cols: ratio among cols
    - line_width: line width inside rect
    - line_color: line color inside rect
    - see RepEleRect

```C
//rows/cols: ratio among rows/cols
config_grid = {'size': [0.8, 0.8], "pos": [0.1, 0.1], "is_abs": False, 
	"padding": [0.1, 0], "has_border": True, "border_width": 0.01,
	"border_color": Report.Colors.red, "border_expand": 0, "is_fill": True,
	"fill_color": Report.Colors.aliceblue, "is_round": True, "round_radius": 0.05,
	"rows": [1, 2, 1], "cols": [2, 2, 4], "line_width": 0.02,
	"line_color": Report.Colors.blueviolet}
// a grid inside previous rect
grid = Report.RepEleGrid(rect)
grid.set(config_grid)

//since grid is child of rect, no need to call draw function explictly
rect.draw(doc)
```

- RepEleImage
  - params
    - img_path: path of image to show
    - has_border: draw border or not
    - border_color: see RepEleRect
    - border_width: see RepEleRect
    - border_expand: see RepEleRect
    - keep_aspect: keep image's aspect
    - see ReportElement

```C
config_img = {'size': [0.5, 0.2], "pos": [0.0, 0.25], "is_abs": False,
	"padding": [0.0, 0.0], "has_border": False, "border_color": Report.Colors.red, 
	"border_width": 0.01, "border_expand": 0,"img_path": "",
	"keep_aspect": True}
img = Report.RepEleImage(doc)
img.set(config_img)
img.draw(doc)
```

- RepEleText
  - not fine in auto-return support
  - clip text if no enough space to show
  - horizontal text only
  - params
    - text: text to show
    - font: font name
    - font_size: font size
    - fill_grey: color scale, [0, 1.0]
    - fill_color: text color
    - fill_alpha: visibility of text, [0, 1.0]
    - char_space: space between charactors
    - word_space: space between words
    - horizontal_scale: scale along horizontal
    - interline_space: space between lines, 1.2*font_size by default
    - rise: superscript, subscript

```C
// import font for chiness
Report.importFont("华文行楷","C:/Program Files (x86)/Microsoft Office/root/VFS/Fonts/private/STXINGKA.TTF")

config_txt={"size":size, "pos":pos, "is_abs":True,
	"font_size":9, "font":"华文行楷","word_space": 3}
txt=Report.RepEleText(doc)
txt.set(config_txt)
txt.setText("reportlab不是python的标准库，它的强大之处在于能满足绝大部分报表的需求形式。")
// draw
txt.draw(doc)
```