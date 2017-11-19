#include "Canvas.h"
#include "font.h"

//#define DEBUG

Rect& Rect::text(Canvas &canvas, Alignment align, char * c, COLOR color) {
	switch (align) {
		case leftAlign :
			canvas.text(left, top, c, color); 
			break;
		case rightAlign :
			canvas.text(right-canvas.textWidth(c), top, c, color); 
			break;
		case centerAlign :
			canvas.text( (left+right)/2 - canvas.textWidth(c)/2, (top+bottom)/2 -canvas.textHeight(c)/2 , c, color); 
			break;
	}
	return *this;
}

Rect& Rect::textDiff(Canvas &canvas, Alignment align, char * newText, char * oldText, COLOR ftColor, COLOR bkColor) {

	int x,diff;
	int lnew=strlen(newText);
	int lold=strlen(oldText);
	int csize = canvas.textWidth("A");

	switch (align) {
		case leftAlign :
		
			x = left;
			for (int i=0; i<lnew; i++) {
				if (i<lold) {
					if (newText[i]!=oldText[i]) {
						canvas._drawChar(x,top,newText[i],ftColor,bkColor);
					}
				} else {
					canvas._drawChar(x,top,newText[i],ftColor,bkColor);
				}
				x += csize;
			}
			for (int i=lnew; i<lold; i++) {
				canvas._drawChar(x,top,0x20,ftColor,bkColor);
				x += csize;
			}
			break;
			
		case rightAlign :

			x = right - csize;
			for (int i=0; i<lnew; i++) {
				if (i<lold) {
					if (newText[lnew-i-1]!=oldText[lold-i-1]) { 
						canvas._drawChar(x,top,newText[lnew-i-1],ftColor,bkColor);
					} 
				} else {
					canvas._drawChar(x,top,newText[lnew-i-1],ftColor,bkColor);
				}
				x -= csize;
			}
			for (int i=lnew; i<lold; i++) {
				canvas._drawChar(x,top,0x20,ftColor,bkColor);
				x -= csize;
			}
			break;
			
		case centerAlign :
		
			if (lold>lnew) {
				Rect rc;
				rc.left = (left+right)/2 - canvas.textWidth(oldText)/2;
				rc.right = rc.left + canvas.textWidth(oldText);
				rc.top = (top+bottom)/2 -canvas.textHeight(oldText)/2;
				rc.bottom = rc.top + canvas.textHeight(oldText);
				canvas.fillRectangle(rc, bkColor);
			}

			canvas.text( (left+right)/2 - canvas.textWidth(newText)/2, (top+bottom)/2 -canvas.textHeight(newText)/2 , newText, ftColor, bkColor); 
			break;
	}

	#ifdef DEBUG_TEXTDIFF
	Serial.println("");
	#endif
	return *this;
}


Rect& Rect::text(Canvas &canvas, Alignment align, char * c) {
	return text(canvas,align, c, canvas.textColor);
}

Rect& Rect::grid(Canvas &canvas, uint16_t ww, uint16_t hh, COLOR color) {

	#ifdef DEBUG
	char buffer[80];
	sprintf(buffer,"(CV) grid L:%u T:%u R:%u B:%u W:%u H:%u",left,top,right,bottom,ww,hh);
	Serial.println(buffer);
	#endif

	COORD xx;
	xx = top;
	while (xx<bottom) { canvas.lineTo(left,xx,right,xx,color); xx += hh; }
	xx = left;
	while (xx<right) { canvas.lineTo(xx,top,xx,bottom,color); xx += ww; }
	return *this;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Grid& Grid::operator=(const Rect& rc) {
	left = rc.left;
	top = rc.top;
	right = rc.right;
	bottom = rc.bottom;
	
}

void Grid::paint(Canvas &canvas) { 
	grid(canvas, cWidth, cHeight, canvas.textColor);
};

Grid& Grid::draw(Canvas &canvas, COLOR color) {
	grid(canvas, cWidth, cHeight, color);
}

bool Grid::cellAtPos(COORD x, COORD y, COORD& cx, COORD& cy) {

	if ((cWidth==0) || (cHeight==0) || !inside(x,y)) {
		cx = -1;
		cy = -1;
		return 0;
	}
	
	cx = (x-left) / cWidth;
	cy = (y-top) / cHeight;

	#ifdef DEBUG
	char buffer[40];
	sprintf(buffer,"(MN) ww:%u hh:%u x:%u y:%u cx:%u cy:%u",cWidth,cHeight,x,y,cx,cy);
	Serial.println(buffer);
	#endif

	return 1;
}

Rect Grid::cellRect(COORD  cx, COORD cy) {
	Rect rc;
	if ( (cx<0)||(cy<0) ) return rc;
	//
	rc.left = left+cx*cWidth+1;
	rc.right = rc.left+cWidth-1;
	//
	rc.top = top+cy*cHeight+1;
	rc.bottom = rc.top+cHeight-1;
	return rc;
}

Grid& Grid::fillCell(Canvas  &canvas, COORD cx, COORD cy, COLOR color) {
	Rect rc = cellRect(cx,cy);
	rc.fill(canvas,color);
}

Grid& Grid::textCell(Canvas &canvas, COORD cx, COORD cy, char * text, COLOR color) {
	Rect rc = cellRect(cx,cy);
	rc.text(canvas, centerAlign, text, color);
}

Grid& Grid::textCell(Canvas &canvas, COORD cx, COORD cy, char * text, COLOR front, COLOR back) {
	Rect rc = cellRect(cx,cy);
	rc.fill(canvas,back);
	rc.text(canvas, centerAlign, text, front);
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Menu::Menu(void) {
	oldcx=-1;
	oldcy=-1;
	firstValue = 0;
}

Menu& Menu::draw(Canvas &canvas) {
	
	Rect::fill(canvas,backColor);
	Grid::draw(canvas,lineColor);
	
	for (int i=0; i<horzCount(); i++) 
	for (int j=0; j<vertCount(); j++) {
		textCell(canvas, i,j, texts[cellIndex(i,j)], textColor, backColor);
	}
	
}

Menu& Menu::selected(Canvas& canvas, int value) {

	value -= firstValue;

	if (oldcx!=-1) {
	  textCell(canvas, oldcx,oldcy, texts[cellIndex(oldcx,oldcy)], textColor, backColor);
	}
	indexCell(value, oldcx,oldcy);
	textCell(canvas, oldcx,oldcy, texts[value], selectedText, selectedBack );
}

int Menu::selected(Canvas& canvas, COORD x, COORD y) {

	COORD cx,cy;

	if (cellAtPos(x, y, cx, cy)) {
	
		int value = firstValue + cellIndex(cx,cy);
		selected(canvas, value);
		return value;
	}
	
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Text::paint(Canvas &canvas) {
	Rect::text(canvas, align, text, canvas.textColor);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Rect Canvas::textRect(Point& pt, char *c) {
	Rect rc;
	uint16_t w = _textWidth(c)/2;
	uint16_t h = _textHeight(c)/2;
	rc.left = pt.x-w; 
	rc.right = pt.x+w;
	rc.top = pt.y-h; 
	rc.bottom = pt.y+h;
	return rc;
}

Canvas& Canvas::setPixel(COORD x, COORD y, uint8_t pixelSize, COLOR color) {

	_fillPixels(x,y,pixelSize,pixelSize,color);
	return *this;
}

Canvas& Canvas::drawRectangle(Rect& rect, COLOR color) {
	COORD ww = rect.width();
	COORD hh = rect.height();
	_drawHLine(rect.left,rect.top,ww,color); 
	_drawHLine(rect.left,rect.bottom,ww,color); 
	_drawVLine(rect.right,rect.top,hh,color); 
	_drawVLine(rect.left,rect.top,hh,color); 
	return *this; 
};


void Canvas::_fillRect(Rect& rect, COLOR color) {

	#ifdef DEBUG
	char buffer[80];
	sprintf(buffer,"(CV) fillRect l:%u t:%u r:%u b:%u c:%u",rect.left,rect.top,rect.right,rect.bottom,color);
	Serial.println(buffer);
	#endif
	
	COORD _width = _getDisplayWidth();
	COORD _height = _getDisplayHeight();

	int16_t h = rect.height(); 
	int16_t w = rect.width();
	if ( (rect.left >= _width) || (rect.top >= _height) || (h < 1) || (w < 1)) return;
	
	if ((rect.left + w - 1) >= _width)  w = _width  - rect.left;
	if ((rect.top + h - 1) >= _height) h = _height - rect.top;

	if (w == 1 && h == 1) {
		_setPixel(rect.left,rect.top,color);
		return;
	}
	
	_fillPixels(rect.left,rect.top,w,h,color);
}

#define swap(a, b) { int16_t t = a; a = b; b = t; }

void Canvas::_drawLine(COORD x0, COORD y0, COORD x1, COORD y1, COLOR color) {

	#ifdef DEBUG
	char buffer[80];
	sprintf(buffer,"(CV) drawLine x0:%u y0:%u x1:%u y1:%u C:%u",x0,y0,x1,y1,color);
	Serial.println(buffer);
	#endif

	COORD _width = _getDisplayWidth();
	COORD _height = _getDisplayHeight();

	if ((y0 < 0 && y1 <0) || (y0 > _height && y1 > _height)) return;
	if ((x0 < 0 && x1 <0) || (x0 > _width && x1 > _width)) return;
	if (x0 < 0) x0 = 0;
	if (x1 < 0) x1 = 0;
	if (y0 < 0) y0 = 0;
	if (y1 < 0) y1 = 0;

	if (y0 == y1) {
		if (x1 > x0) {
			_drawHLine(x0, y0, x1 - x0 + 1, color);
		}
		else if (x1 < x0) {
			_drawHLine(x1, y0, x0 - x1 + 1, color);
		}
		else {
			_setPixel(x0, y0, color);
		}
		return;
	}
	else if (x0 == x1) {
		if (y1 > y0) {
			_drawVLine(x0, y0, y1 - y0 + 1, color);
		}
		else {
			_drawVLine(x0, y1, y0 - y1 + 1, color);
		}
		return;
	}

	bool steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	}
	else {
		ystep = -1;
	}

	int16_t xbegin = x0;
	
	if (steep) {
		for (; x0 <= x1; x0++) {
			err -= dy;
			if (err < 0) {
				int16_t len = x0 - xbegin;
				if (len) {
					_drawVLine (y0, xbegin, len + 1, color);
				}
				else {
					_setPixel(y0, x0, color);
				}
				xbegin = x0 + 1;
				y0 += ystep;
				err += dx;
			}
		}
		if (x0 > xbegin + 1) {
			_drawVLine(y0, xbegin, x0 - xbegin, color);
		}

	}
	else {
		for (; x0 <= x1; x0++) {
			err -= dy;
			if (err < 0) {
				int16_t len = x0 - xbegin;
				if (len) {
					_drawHLine(xbegin, y0, len + 1, color);
				}
				else {
					_setPixel(x0, y0, color);
				}
				xbegin = x0 + 1;
				y0 += ystep;
				err += dx;
			}
		}
		if (x0 > xbegin + 1) {
			_drawHLine(xbegin, y0, x0 - xbegin, color);
		}
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void Canvas::_setTextSize(uint8_t value) {
	textSize = value;
}

COORD Canvas::_textWidth(char *c) {
	return textSize * (FONT_WIDTH+FONT_SEP_WIDTH) * strlen(c); // fixed width
}

COORD Canvas::_textHeight(char *c) {
	return textSize * (FONT_HEIGHT+FONT_SEP_HEIGHT); // fixed heigth
}

void Canvas::_drawChar(COORD x, COORD y, uint8_t c, COLOR color, COLOR backColor)
{
	#ifdef DEBUG_CHAR
	char buffer[80];
	sprintf(buffer,"(CV) Char x:%u y:%u char:%c FB:%u FW:%u FH:%u",x,y,c,FONT_BASE,FONT_WIDTH,FONT_HEIGHT);
	Serial.println(buffer);
	#endif

	uint8_t i,j,si,sj,cf,cb,jj;
	uint8_t line;
	uint8_t size = textSize;

	_FONT_PTR_ font = TFT_FONT + (uint16_t)(c-FONT_BASE)*FONT_WIDTH;

	if (size==1) {
		for (i=0; i<FONT_WIDTH; i++ ) {
			line = *font++;
			for (j = 0; j<FONT_HEIGHT; j++) {
				if (line & 0x1) {
					_setPixel(x+i,y+j,color);
				}
				line >>= 1;
			}
		}

	} else {
		/*
		for (i=0; i<FONT_WIDTH; i++ ) {
			line = *font++;
			for (j = 0; j<FONT_HEIGHT; j++) {
				if (line & 0x1) {
					_fillPixels(x+i*size,y+j*size,size,size,color);
				}
				line >>= 1;
			}
		}
		*/
		for (i=0; i<FONT_WIDTH; i++ ) {
			line = *font++;

			j = 0; cf=0; cb=0;
			while (j<FONT_HEIGHT) {
				if (line & 0x1) { 
				
					if (cb){
						if (color!=backColor) { _fillPixels(x+i*size,y+jj*size,size,size*cb,backColor); }
						cb = 0;
					}
				
					if (!cf) jj = j;
					cf++; 
					
				} else {
					if (cf) { 
						_fillPixels(x+i*size,y+jj*size,size,size*cf,color);
						cf = 0;
					}
					
					if (!cb) jj = j;
					cb++; 
				}
				line >>= 1;
				j++;
			}
			
			if (cf) {
				_fillPixels(x+i*size,y+jj*size,size,size*cf,color);
			}
			if (cb) {
				if (color!=backColor) { _fillPixels(x+i*size,y+jj*size,size,size*cb,backColor); }
			}
		}
	}
}

void Canvas::_text(COORD x, COORD y, char* c, COLOR color, COLOR backColor) {

	#ifdef DEBUG_STR
	char buffer[80];
	sprintf(buffer,"(CV) Text x:%u y:%u size:%u C:%u BC:%u",x,y,textSize,color,backColor);
	Serial.println(buffer);
	#endif
	
	COORD _width = _getDisplayWidth();
	COORD _height = _getDisplayHeight();

	while (*c != 0) {
		_drawChar(x, y, *c++, color, backColor);
		x += textSize*(FONT_WIDTH+FONT_SEP_WIDTH);
		if (x + textSize*FONT_WIDTH > _width) {
			y += textSize*FONT_HEIGHT+FONT_SEP_HEIGHT;
			x = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Touch::calibrate(Canvas &tft) {

  uint8_t state;
  uint16_t vx1,vx2,vy1,vy2;
  uint16_t milliSeconds = 0;

  COLOR COLOR_BLACK = tft.rgb(0,0,0);
  COLOR COLOR_WHITE = tft.rgb(255,255,255);
  
  Rect rc;
  rc = tft.clientRect;
  tft.fillRectangle(rc,COLOR_BLACK);
  
  rc.right=rc.left+10;
  rc.bottom=rc.top+10;
  rc.draw(tft,COLOR_WHITE);
  
  state = 0;
  while (1) {
    //
	delay(1);
	milliSeconds++;

	if (touched(milliSeconds)) { 
		switch (state) {
		  case 0 :
			vx1 = vx;
			vy1 = vy;
			rc.draw(tft, COLOR_BLACK);
			rc = tft.clientRect;
			rc.left = rc.right-10;
			rc.top  = rc.bottom-10;
			rc.draw(tft,COLOR_WHITE);
			state = 1;
			break;
			
		  case 1:
			vx2 = vx;
			vy2 = vy;
			rc.draw(tft,COLOR_WHITE);
			rc = tft.clientRect;
			calibrateX(rc.left+5,vx1,rc.right-5,vx2);
			calibrateY(rc.top+5,vy1,rc.bottom-5,vy2);
			return;
		}
	}
  }
}

void Touch::test(Canvas &tft, uint8_t pin) {

  char buffer[9][12] = {"0","1","2","3","4","5","6","7","8"};
  uint16_t milliSeconds=0;

  COLOR COLOR_BLACK = tft.rgb(0,0,0);
  COLOR COLOR_WHITE = tft.rgb(255,255,255);
  COLOR COLOR_GREEN = tft.rgb(0,255,0);
  COLOR COLOR_RED = tft.rgb(255,0,0);
  
  Menu menu;
  
  menu.rect(tft.clientRect).horzCount(3).vertCount(3);
  menu.colors(COLOR_WHITE,COLOR_GREEN,COLOR_BLACK,COLOR_GREEN,COLOR_RED);
  
  for (int i=0; i<menu.itemCount(); i++) {
    menu.textItem(i, buffer[i] );
  }

  tft.clearScreen();
  menu.draw(tft);
  menu.selected(tft,0);

  while (1) {
    //
	delay(1);
	milliSeconds++;
	if (touched(milliSeconds)) { 
		if (pin) digitalWrite(pin, ! digitalRead(pin)); 

		char buf[40];
		sprintf(buf,"Touh x:%u y:%u",x,y);
		Serial.println(buf);

		switch (menu.selected(tft, x, y)) {
		  case -1: break;
		  case 0: break;
		}
	}
  }
}
