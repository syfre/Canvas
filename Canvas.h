/*
	Abstract class for Canvas

	Copyright (C) 2017 sylvain Frere
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef Canvas_h
#define Canvas_h

#include <Arduino.h>

typedef int16_t		COORD;
typedef	uint16_t	COLOR;
typedef uint16_t	Value;

enum Orientation {portrait=0,landscape=1};
enum Alignment {leftAlign=0, centerAlign=1, rightAlign=2};

class Canvas;
class Point;
class Rect;

class Display {
public:
	Canvas & canvas;
	inline bool enable() { return _enable; };

	void enable(bool value) { if (_enable!=value) { _enable=value; enableChanged(); } }; 
	Display(Canvas& _canvas, bool __enable=0): canvas(_canvas), _enable(__enable) { };
	virtual void paint(void) {} ;
	
protected:
	bool _enable;
	void enableChanged(void) { paint(); };
};

class Widget {
public:
	Widget *next;
	virtual void print() {  };
	virtual void paint(Canvas &canvas) {  };
};

class WidgetList {
public:
	Widget* 	head;
	Widget* 	tail;
	WidgetList(void) { tail=NULL; head=NULL;};
	WidgetList& add(Widget& w) { if (tail) tail->next=&w; w.next = NULL; tail=&w; if (!head) head=&w; return *this; };
	WidgetList& paint(Canvas &canvas) { Widget *w=head; while(w) { w->paint(canvas); w=w->next; } return *this; };
	WidgetList& print(char * c=NULL) { Widget *w=head; if (c) Serial.println(c); while(w) { w->print(); w=w->next; } return *this; };
};

class Point: public Widget {
public:
	COORD	x,y;
	inline Point& position(COORD _x, COORD _y) { x=_x; y=_y; return *this; };
	Point& center(Rect& rc);
	Point& leftTop(Rect& rc);
	Point& rightBottom(Rect& rc);
};

class Rect: public Widget {
public:
	COORD	top;
	COORD	left;
	COORD	right;
	COORD	bottom;
	Rect(void) { top=0; left=0; right=0; bottom=0; };
	Rect& serialPrint(void) { 
		char buffer[40];
		sprintf(buffer,"L:%u T:%u R:%u B:%u",left,top,right,bottom);
		Serial.println(buffer);
		return *this;
	};
	Rect& serialPrint(char * c) { 
		char buffer[40];
		sprintf(buffer,"%s L:%u T:%u R:%u B:%u",c,left,top,right,bottom);
		Serial.println(buffer);
		return *this;
	};
	inline Rect& set(COORD _left, COORD _top, COORD _right, COORD _bottom) { left=_left; top=_top; right=_right; bottom=_bottom; return *this; };
	inline COORD width(void) { return right-left; }
	inline Rect& width(COORD value) { right = left+value-1; return *this; };
	inline COORD height(void) { return bottom-top; }
	inline Rect& height(COORD value) { bottom = top+value-1; return *this; };
	inline Rect& position(COORD _left, COORD _top) { COORD ww=right-left; COORD hh=bottom-top; left=_left; top=_top; right=left+ww; bottom=top+hh; return *this; };
	inline Rect& size(COORD _width, COORD _height) { right=left+_width; bottom=top+_height; return *this; };
	inline Rect& inflate(COORD dx, COORD dy) { left-=dx; right+=dx; top-=dy; bottom+=dy; return *this; };
	inline bool inside(COORD x, COORD y) { return (x>=left)&&(x<right)&&(y>=top)&&(y<bottom); };
	Point center(void) { Point rslt; rslt.x = (left+right)/2; rslt.y = (top+bottom)/2; return rslt; }
	Rect& inflatePercent(double pcx, double pcy) { return inflate( (COORD) (right-left)*pcx/100, (COORD) (bottom-top)*pcy/100);	};
	Rect& fill(Canvas &canvas, COLOR color);
	Rect& draw(Canvas &canvas, COLOR color);
	Rect& grid(Canvas &canvas, uint16_t ww, uint16_t hh, COLOR color);
	Rect& clear(Canvas &canvas);
	Rect& text(Canvas &canvas, Alignment align, char * c, COLOR color);
	Rect& text(Canvas &canvas, Alignment align, char * c);
	Rect& textDiff(Canvas &canvas, Alignment align, char * newText, char * oldText, COLOR ftColor, COLOR bkColor);
	
	void print(void) { serialPrint(); }
	void paint(Canvas &canvas) { clear(canvas); };
};

class Grid: public Rect {
private:
	uint16_t cWidth;
	uint16_t cHeight;
public:
	Grid& operator=(const Rect& rc);
	inline Grid& rect(Rect& rc) { left=rc.left; top=rc.top; right=rc.right; bottom=rc.bottom; return *this; };
	inline uint16_t cellWidth(void) { return cWidth; };
	inline Grid&  	cellWidth(uint16_t val) { cWidth=val; return *this; };
	inline uint16_t cellHeight(void) { return cHeight; };
	inline Grid&    cellHeight(uint16_t val) { cHeight=val; return *this; };
	inline uint16_t horzCount(void) { if (cWidth) return (right-left)/cWidth; else return 0; };
	inline Grid&    horzCount(uint16_t val) { cWidth = (right-left)/val; return *this; };
	inline uint16_t vertCount(void) { if (cHeight) return (bottom-top)/cHeight; else return 0; };
	inline Grid&    vertCount(uint16_t val) { cHeight = (bottom-top)/val; return *this; };

	inline Grid& indexCell(int index, COORD& cx, COORD& cy) { cy = index/horzCount(); cx= index % horzCount(); return *this; }
	inline int cellIndex(COORD cx, COORD cy) { return cy*horzCount() + cx; };
	bool cellAtPos(COORD x, COORD y, COORD&  cx, COORD& cy);
	Rect cellRect(COORD cx, COORD cy);
	Grid& fillCell(Canvas &canvas, COORD cx, COORD cy, COLOR color);
	Grid& textCell(Canvas &canvas, COORD cx, COORD cy, char * text, COLOR color);
	Grid& textCell(Canvas &canvas, COORD cx, COORD cy, char * text, COLOR front, COLOR back);

	Grid& draw(Canvas &canvas, COLOR color);
	void paint(Canvas &canvas);
};

#define cMaxMenuItem	18

class Menu: public Grid {
private:
	char *	texts[cMaxMenuItem];
	COORD	oldcx,oldcy;
public:
	int		firstValue;
	COLOR	lineColor;
	COLOR	textColor;
	COLOR	backColor;
	COLOR	selectedText;
	COLOR	selectedBack;
	//
	Menu(void);
	inline Menu& colors(COLOR _line, COLOR _text, COLOR _back, COLOR _SText, COLOR _SBack) { lineColor=_line; textColor=_text; backColor=_back; selectedText=_SText; selectedBack=_SBack; return *this; };
	inline char* textItem(int index) { if (index<cMaxMenuItem) return texts[index]; else return NULL; };
	inline Menu& textItem(int index, char * c) { if (index<cMaxMenuItem) texts[index] = c; return *this; };
	inline int itemCount(void) { return horzCount()*vertCount(); };

	Menu& draw(Canvas &canvas);
	int selected(Canvas& canvas, COORD x, COORD y);
	Menu& selected(Canvas& canvas, int value);
};

class Text: public Rect {
public:
	Alignment	align;
	char *		text;
	Text(Alignment _align=leftAlign, char * _text=NULL) :align(_align),text(_text) {};
	inline Text& alignment(Alignment _align) { align=_align; return *this;}
	inline Alignment alignment(void) { return align; }
	inline Text& setText(char * c) { text=c; return *this; };
	inline Text& setRect(Rect& r) { left=r.left, top=r.top; right=r.right; bottom=r.bottom; return *this; }
	void paint(Canvas &canvas);
};


////////////////////////////////////
// Canvas
////////////////////////////////////
class Canvas {
protected:
	virtual COORD _getDisplayWidth(void) = 0;
	virtual COORD _getDisplayHeight(void) = 0;
	virtual void _fillPixels(COORD x, COORD y, uint16_t w, uint16_t h, COLOR color) = 0;
	virtual void _setPixel(COORD x, COORD y, COLOR color) = 0;
	virtual void _drawHLine(COORD x, COORD y, uint16_t w, COLOR color) = 0;
	virtual void _drawVLine(COORD x, COORD y, uint16_t h, COLOR color) = 0;

	virtual void _fillRect(Rect& rect, COLOR color);
	virtual void _drawLine(COORD x0, COORD y0, COORD x1, COORD y1, COLOR color);

	virtual	void _setTextSize(uint8_t value);
	virtual COORD _textWidth(char *c);
	virtual COORD _textHeight(char *c);
	virtual void _drawChar(COORD x, COORD y, uint8_t c, COLOR color, COLOR backcolor);
	virtual void _text(COORD x, COORD y, char* c, COLOR color, COLOR backColor);
public:

	Rect	clientRect;
	COLOR	textColor;
	COLOR	backColor;
	uint8_t	textSize;
	Canvas(void) {  };
	~Canvas() {};
	
	virtual void setOrientation(Orientation value) = 0 ; 
	
	virtual COLOR rgb(uint8_t r,uint8_t g, uint8_t b) = 0;
	void setBackColor(COLOR color) { backColor = color; };
	void setFontColor(COLOR color) { textColor = color; };

	inline COLOR colorBlack(void) { return rgb(0,0,0); }
	inline COLOR colorWhite(void) { return rgb(255,255,255); }
	inline COLOR colorRed(void) { return rgb(255,0,0); }
	inline COLOR colorGreen(void) { return rgb(0,255,0); }
	inline COLOR colorBlue(void) { return rgb(0,0,255); }
	
	inline COORD width(void) {return clientRect.width(); }
	inline COORD height(void) {return clientRect.height(); }
	
	Canvas& setPixel(COORD x, COORD y, uint8_t pixelSize, COLOR color);
	inline	Canvas& setPixel(COORD x, COORD y, COLOR color) { _setPixel(x,y,color); return *this; };
	inline	Canvas& setPixel(Point& pt, COLOR color) { _setPixel(pt.x,pt.y,color); return *this; };
	inline	Canvas& setPixel(COORD x, COORD y) { _setPixel(x,y,textColor); return *this; } ;
	inline	Canvas& setPixel(Point& pt) { _setPixel(pt.x,pt.y,textColor); return *this; };
	inline  Canvas& fillPixels(COORD x, COORD y, uint16_t w, uint16_t h, COLOR color) { _fillPixels(x,y,w,h,color); return *this; };
	
	inline	Canvas& setFontSize(uint8_t value) { _setTextSize(value); return *this; };
	inline 	COORD textWidth(char *c) { return _textWidth(c); };
	inline 	COORD textHeight(char *c) { return _textHeight(c); };
			Rect textRect(Point& pt, char *c);

	inline  Canvas& text(COORD x, COORD y, char* c, COLOR color) { _text(x,y,c,color,color); return *this; };
	inline  Canvas& text(COORD x, COORD y, char* c, COLOR color, COLOR bkColor) { _text(x,y,c,color,bkColor); return *this; };
	inline	Canvas& text(COORD x, COORD y, char* c) { _text(x,y,c,textColor,textColor); return *this; };
	inline 	Canvas& text(Point& pt, char* c, COLOR color) { _text(pt.x,pt.y,c,color,color); return *this; };
	inline 	Canvas& text(Point& pt, char* c, COLOR ftColor, COLOR bkColor) { _text(pt.x,pt.y,c,ftColor,bkColor); return *this; };

	inline	Canvas& fillRectangle(Rect& rect, COLOR color) { _fillRect(rect,color); return *this; };
	inline	Canvas& fillRectangle(Rect& rect)  { _fillRect(rect,backColor); return *this; };
			Canvas& drawRectangle(Rect& rect, COLOR color);
	inline	Canvas& clearScreen(COLOR color) { _fillRect(clientRect, color); return *this; };
	inline	Canvas& clearScreen(void) { _fillRect(clientRect, backColor); return *this; };
	inline	Canvas& lineTo(COORD x0, COORD y0, COORD x1, COORD y1, COLOR color) { _drawLine(x0,y0,x1,y1,color); return *this; };
	inline	Canvas& lineTo(Point pt1, Point pt2, COLOR color) { _drawLine(pt1.x,pt1.y,pt2.x,pt2.y,color); return *this; };
	
	friend Rect;
};

#define TOUCH_PORTRAIT  0
#define TOUCH_LANDSCAPE 1

class Touch {
public:
    Value vx,vy;			 // Raw values
    COORD x,y;				 // see Portrat / Landscape 
    virtual bool touched(uint16_t tick) = 0;
    virtual void calibrateX(COORD p1, Value v1, COORD p2, Value v2) = 0;
    virtual void calibrateY(COORD p1, Value v1, COORD p2, Value v2) = 0;
	void calibrate(Canvas &tft);
	void test(Canvas &tft, uint8_t pin=0);
};

inline Rect& Rect::clear(Canvas &canvas) { canvas.fillRectangle(*this, canvas.backColor); return *this; };
inline Rect& Rect::fill(Canvas &canvas, COLOR color) { canvas.fillRectangle(*this, color); return *this; };
inline Rect& Rect::draw(Canvas &canvas, COLOR color) { canvas.drawRectangle(*this, color); return *this; };

inline Point& Point::center(Rect& rc) { x = (rc.left+rc.right)/2; y = (rc.top+rc.bottom)/2; return *this; };
inline Point& Point::leftTop(Rect& rc) { x = rc.left; y = rc.top; return *this; };
inline Point& Point::rightBottom(Rect& rc) { x = rc.right; y = rc.bottom; return *this; };

#endif
