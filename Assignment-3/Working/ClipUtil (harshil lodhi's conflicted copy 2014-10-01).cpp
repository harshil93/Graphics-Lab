#include <windows.h>
#include <math.h>
#include "ClipUtil.h"
#include "Line.h"
#include "vector"
#include "PolyClip.h"

using namespace std;

DRAWING_DATA gDrawData; // global data
POINT startPt; // starting point of the polygon
POINT prevPt; // just prev point of the polygon (during drawing)
bool lineDrawing; // whether polygon drawing has started or not

struct line{
	POINT start;
	POINT end;
};

vector<POINT> polyVer;
void reDraw(HWND hwnd)
{
  InvalidateRect(hwnd, NULL, 1);
}

void setupMenus(HWND hwnd)
{
  HMENU hmenu, hsubmenu;
  hmenu=GetMenu(hwnd);
  hsubmenu=GetSubMenu(hmenu, 0);

  switch (gDrawData.drawMode)
  {
    case READY_MODE :
      //enable 'Draw Rectangle',disable 'Draw Line','Clip' menus
      EnableMenuItem(hsubmenu, ID_DRAW_RECTANGLE,
                     MF_BYCOMMAND|MF_ENABLED);
      EnableMenuItem(hsubmenu, ID_DRAW_LINE,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu, ID_CLIP,
                     MF_BYCOMMAND|MF_GRAYED);
      break;
    case DRAW_RECTANGLE_MODE :
      //enable 'Draw Line',disable 'Draw Rectangle','Clip' menus
      EnableMenuItem(hsubmenu, ID_DRAW_RECTANGLE,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu, ID_DRAW_LINE,
                     MF_BYCOMMAND|MF_ENABLED);
	  lineDrawing = false;
      EnableMenuItem(hsubmenu, ID_CLIP,
                     MF_BYCOMMAND|MF_GRAYED);
      break;
    case DRAW_LINE_MODE :
      //enable 'Clip',disable 'Draw Line, Rectangle' menus
      EnableMenuItem(hsubmenu, ID_DRAW_RECTANGLE,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu, ID_DRAW_LINE,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu, ID_CLIP,
                     MF_BYCOMMAND|MF_ENABLED);
      break;
    case CLIP_MODE :
    case CLIPPED_MODE :
      //disable 'Clip' and 'Draw Line, Rectangle' menus
      EnableMenuItem(hsubmenu,
                     ID_DRAW_RECTANGLE,MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu, ID_DRAW_LINE,
                     MF_BYCOMMAND|MF_GRAYED);
      EnableMenuItem(hsubmenu, ID_CLIP,
                     MF_BYCOMMAND|MF_GRAYED);
      break;
  }
}

void setDrawMode(MODE mode, HWND hwnd)
{
  gDrawData.drawMode = mode;
  setupMenus(hwnd);
}

void createMemoryBitmap(HDC hdc)
{
  gDrawData.hdcMem = CreateCompatibleDC(hdc);
  gDrawData.hbmp = CreateCompatibleBitmap(hdc, 
    gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy);
  SelectObject(gDrawData.hdcMem, gDrawData.hbmp);
  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, 
         gDrawData.maxBoundary.cy, PATCOPY);
}

void initialize(HWND hwnd, HDC hdc)
{
  gDrawData.beginPt.x = gDrawData.beginPt.y = 0;
  gDrawData.endPt.x = gDrawData.endPt.y = 0;
  gDrawData.lineEndPts[0].x = 
    gDrawData.lineEndPts[0].y = 0;
  gDrawData.lineEndPts[1].x = 
    gDrawData.lineEndPts[1].y = 0;
  gDrawData.rectCornerPts[0].x = 
    gDrawData.rectCornerPts[0].y = 0;
  gDrawData.rectCornerPts[1].x = 
    gDrawData.rectCornerPts[1].y = 0;

  gDrawData.maxBoundary.cx = GetSystemMetrics(SM_CXSCREEN);
  gDrawData.maxBoundary.cy = GetSystemMetrics(SM_CYSCREEN);

  createMemoryBitmap(hdc);
  setDrawMode(READY_MODE, hwnd);
}

void cleanup()
{
  DeleteDC(gDrawData.hdcMem);
}

void reset(HWND hwnd)
{
  gDrawData.beginPt.x = gDrawData.beginPt.y = 0;
  gDrawData.endPt.x = gDrawData.endPt.y = 0;
  gDrawData.lineEndPts[0].x = 
    gDrawData.lineEndPts[0].y = 0;
  gDrawData.lineEndPts[1].x = 
    gDrawData.lineEndPts[1].y = 0;
  gDrawData.rectCornerPts[0].x = 
    gDrawData.rectCornerPts[0].y = 0;
  gDrawData.rectCornerPts[1].x = 
    gDrawData.rectCornerPts[1].y = 0;
  
  gDrawData.drawMode = READY_MODE;

  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, 
         gDrawData.maxBoundary.cy, PATCOPY);

  reDraw(hwnd);
  polyVer.clear();
  setupMenus(hwnd);
}

void drawImage(HDC hdc)
{
  BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx, 
    gDrawData.maxBoundary.cy, gDrawData.hdcMem, 0, 0, SRCCOPY);
}

void performRubberBanding(HWND hwnd, int x, int y)
{
  int r = 0;
  HDC hdc = GetDC(hwnd);

  switch(gDrawData.drawMode)
  {
    case DRAW_LINE_MODE :
      drawLineSegment(hdc, gDrawData.beginPt, 
                      gDrawData.endPt, CLR_BG);
      drawRectangle(hdc,
                    gDrawData.rectCornerPts[0], 
                    gDrawData.rectCornerPts[1],
                    CLR_RECT); 
      gDrawData.endPt.x = x;
      gDrawData.endPt.y = y;
      drawLineSegment(hdc, gDrawData.beginPt, 
                      gDrawData.endPt, CLR_LINE); 
      break;

    case DRAW_RECTANGLE_MODE :
      drawRectangle(hdc, gDrawData.beginPt, 
                    gDrawData.endPt, CLR_BG);
      gDrawData.endPt.x = x;
      gDrawData.endPt.y = y;
      drawRectangle(hdc, gDrawData.beginPt, 
                    gDrawData.endPt, CLR_RECT);
      break;
    default :
      break;
  }
  ReleaseDC(hwnd, hdc) ;
}
void processRightButtonDown(HWND hwnd){
	switch (gDrawData.drawMode)
	{
	case DRAW_LINE_MODE:
			reDraw(hwnd);
			drawLineSegment(gDrawData.hdcMem, prevPt, startPt, CLR_LINE);

			lineDrawing = false;
			setDrawMode(DRAW_LINE_MODE, hwnd);
		break;
	default:
		return;
	}
}

void processLeftButtonDown(HWND hwnd, int x, int y)
{
  switch (gDrawData.drawMode)
  {
    case DRAW_LINE_MODE:
		if (!lineDrawing){
			startPt.x = x; 
			startPt.y = y;
			prevPt = startPt;
			lineDrawing = true;
			polyVer.push_back(startPt);
		}
		else{
			POINT curPoint;
			curPoint.x = x;
			curPoint.y = y;

			line l; l.start = prevPt;
			l.end = curPoint;
			polyVer.push_back(curPoint);
			reDraw(hwnd);
			drawLineSegment(gDrawData.hdcMem, prevPt, curPoint,CLR_LINE);
			prevPt = curPoint;
		}
		
		break;
    case DRAW_RECTANGLE_MODE:
      gDrawData.beginPt.x = gDrawData.endPt.x = x;
      gDrawData.beginPt.y = gDrawData.endPt.y = y;
      reDraw(hwnd);
      break;

    default:
      return;
  }
}

void processMouseMove(HWND hwnd, int x, int y)
{
  switch (gDrawData.drawMode)
  {
    case DRAW_LINE_MODE:
		break;
    case DRAW_RECTANGLE_MODE:
      performRubberBanding(hwnd, x, y);
      break;
    default:
      break;
  }
}

void draw()
{
  switch (gDrawData.drawMode)
  {
    case DRAW_LINE_MODE:
		break;
      gDrawData.lineEndPts[0].x = gDrawData.beginPt.x;
      gDrawData.lineEndPts[0].y = gDrawData.beginPt.y;

      gDrawData.lineEndPts[1].x = gDrawData.endPt.x;
      gDrawData.lineEndPts[1].y = gDrawData.endPt.y;

      drawLineSegment(gDrawData.hdcMem, gDrawData.beginPt, 
                      gDrawData.endPt, CLR_LINE);
      break;

    case DRAW_RECTANGLE_MODE:
      gDrawData.rectCornerPts[0].x = gDrawData.beginPt.x;
      gDrawData.rectCornerPts[0].y = gDrawData.beginPt.y;

      gDrawData.rectCornerPts[1].x = gDrawData.endPt.x;
      gDrawData.rectCornerPts[1].y = gDrawData.endPt.y;
      
      drawRectangle(gDrawData.hdcMem, 
        gDrawData.rectCornerPts[0], 
        gDrawData.rectCornerPts[1], CLR_RECT);
      break;

    default:
      return;
  }
}

void processCommonCommand(int cmd, HWND hwnd)
{
  int response;
  switch(cmd)
  {
    case ID_CLEAR:
      reset(hwnd);
      break;
    case ID_EXIT:
      response = 
        MessageBox(hwnd,"Quit the program?", "EXIT", MB_YESNO);
        if(response==IDYES) 
          PostQuitMessage(0);
        break;
    default:
      break;
  }
}

void swap(LONG& x, LONG& y)
{
  LONG tmp;
  tmp = x;
  x = y;
  y = tmp;
}


bool isValidRectangle(POINT a, POINT b)
{
  if ((abs(a.x - b.x) >= 10) &&
      (abs(a.y - b.y) >= 10))
    return true;
  else
    return false;
}

bool isValidLine(POINT a, POINT b)
{
  double length;
  double xDiff, yDiff;
  xDiff = a.x - b.x;
  yDiff = a.y - b.y;
  length = sqrt(xDiff * xDiff + yDiff * yDiff);
  if (length >= 10) 
    return true;
  else
    return false;
}

void processCommand(int cmd, HWND hwnd)
{
  switch(cmd)
  {
    case ID_DRAW_LINE:
      if (isValidRectangle(gDrawData.beginPt, gDrawData.endPt))
      {
        //valid rectangle, then switch to next mode
        //draw the rectangle first as drawn through rubber banding
        draw();
        setDrawMode(DRAW_LINE_MODE, hwnd);
      }
      else
      {
        MessageBox(hwnd,
         "Please draw a valid rectangle of at least 10x10 pixels", 
         "Warning!", MB_OK);
      }
      break;
    case ID_DRAW_RECTANGLE:
     setDrawMode(DRAW_RECTANGLE_MODE, hwnd);
     break;
    case ID_CLIP:
      if (isValidLine(gDrawData.beginPt, gDrawData.endPt))
      {
        // valid line, then switch to next mode
        // draw the line first as drawn through rubber banding
        
		  vector<vec_t> poly, rect;
		  for (int i = 0; i < polyVer.size(); i++)
		  {
			  vec_t temp;
			  temp.x = polyVer[i].x;
			  temp.y = polyVer[i].y;
			  poly.push_back(temp);
		  }

		  vec_t a1, a2, a3, a4;

		  // creating a clipping window
		  a1.x = gDrawData.rectCornerPts[0].x;
		  a1.y = gDrawData.rectCornerPts[0].y;
			
		  a2.x = gDrawData.rectCornerPts[0].x;
		  a2.y = gDrawData.rectCornerPts[1].y;

		  a3.x = gDrawData.rectCornerPts[1].x;
		  a3.y = gDrawData.rectCornerPts[1].y;

		  a4.x = gDrawData.rectCornerPts[1].x;
		  a4.y = gDrawData.rectCornerPts[0].y;

		  rect.push_back(a1); rect.push_back(a2); rect.push_back(a3); rect.push_back(a4);

		  vector<POINT> res = polycliphandler(&rect[0], &poly[0], rect.size(), poly.size());

		  // drawing the clipped polygon boundary
		  for (int i = 1; i < res.size(); i++)
		  {
			  reDraw(hwnd);
			  drawLineSegment(gDrawData.hdcMem, res[i - 1], res[i], CLR_BOUNDARY);
		  }

		  reDraw(hwnd);
		  if(res.size()!=0) drawLineSegment(gDrawData.hdcMem, res[0], res[res.size() - 1], CLR_BOUNDARY);
		  setDrawMode(CLIP_MODE, hwnd);
        
      }
      else
      {
        MessageBox(hwnd,
         "Please draw a valid line of length at least 10 pixels", 
         "Warning!", MB_OK);
      }
      break;
    default:
      processCommonCommand(cmd, hwnd);
      return;
  }
}

LRESULT CALLBACK WindowF(HWND hwnd,UINT message,WPARAM wParam,
                         LPARAM lParam) 
{
  HDC hdc;
  PAINTSTRUCT ps;
  int x,y;

  switch(message)
  {
    case WM_CREATE:
      hdc = GetDC(hwnd);
      initialize(hwnd, hdc);
      ReleaseDC(hwnd, hdc);
      break;
    
    case WM_COMMAND:
      processCommand(LOWORD(wParam), hwnd);
      break;

    case WM_LBUTTONDOWN:
      x = LOWORD(lParam);
      y = HIWORD(lParam);
      processLeftButtonDown(hwnd, x,y);
      break;

    case WM_MOUSEMOVE:
      if(wParam & MK_LBUTTON)
      {
        x = LOWORD(lParam);
        y = HIWORD(lParam);
        processMouseMove(hwnd, x,y);
      }
      break;
	case WM_RBUTTONDOWN:
		processRightButtonDown(hwnd);
		break;
    case WM_PAINT:
      hdc = BeginPaint(hwnd, &ps);
      drawImage(hdc);
      EndPaint(hwnd, &ps);
      break;

    case WM_DESTROY:
      cleanup();
      PostQuitMessage(0);
      break;
    
    default:
      break;
  }
  // Call the default window handler
  return DefWindowProc(hwnd, message, wParam, lParam);
}

bool replaceColor(HDC hdc, long x, long y,
                  COLORREF clrFrom, COLORREF clrTo)
{
  //returns true if replacement is applicable,otherwise false
  if(GetPixel(hdc, (int) x, (int) y) == clrFrom)
  {
    SetPixel(hdc, (int) x, (int) y, clrTo);
    return true;
  }
  return false;
}

void performCorrectionAtClipPts(HDC hdc, 
      POINT clipPt, COLORREF clrLine, COLORREF clrErase)
{
  POINT pt;

  if ((clipPt.x == gDrawData.clipMin.x) || 
      (clipPt.x == gDrawData.clipMax.x))
  {
    // on vertical side of clipping
    pt.x = clipPt.x;
    pt.y = clipPt.y;    
    while (replaceColor(hdc, pt.x, pt.y, clrLine, clrErase))
    {
      pt.y--;
    }
    pt.x = clipPt.x;
    pt.y = clipPt.y+1;
    while (replaceColor(hdc, pt.x, pt.y, clrLine, clrErase))
    {
      pt.y++;
    }    
  }
  if ((clipPt.y == gDrawData.clipMin.y) || 
      (clipPt.y == gDrawData.clipMax.y))
  {
    // on horizontal side of clipping
    pt.x = clipPt.x;
    pt.y = clipPt.y;    
    while (replaceColor(hdc, pt.x, pt.y, clrLine, clrErase))
    {
      pt.x--;
    }
    pt.x = clipPt.x+1;
    pt.y = clipPt.y;
    while (replaceColor(hdc, pt.x, pt.y, clrLine, clrErase))
    {
      pt.x++;
    }
  }
}

void drawRectangle(HDC hdc, POINT corner1, POINT corner2, 
                   COLORREF clr)
{
  POINT pt1, pt2;
  pt1.x = corner1.x;
  pt1.y = corner2.y;
  pt2.x = corner2.x;
  pt2.y = corner1.y;
  
  drawLineSegment(hdc, corner1, pt1, clr);
  drawLineSegment(hdc, pt1, corner2, clr);
  drawLineSegment(hdc, corner2, pt2, clr);
  drawLineSegment(hdc, pt2, corner1, clr);
}
