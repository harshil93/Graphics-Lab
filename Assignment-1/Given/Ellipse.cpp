#include <math.h>
#include <windows.h>
#include "Ellipse.h"

DRAWING_DATA gDrawData; // global data
void drawEllipse(int ma,int mb);
LRESULT CALLBACK DlgAxis(HWND hdlg,UINT mess,WPARAM more,LPARAM pos);

void reDraw(HWND hwnd)
{
  InvalidateRect(hwnd, NULL, 1);
}

void drawPoint(int x, int y)
{
  Ellipse(gDrawData.hdcMem,x-2,y-2,x+2,y+2);
}

void setupMenus(HWND hwnd)
{
  HMENU hMenu;
  
  hMenu=GetMenu(hwnd);

  switch (gDrawData.drawMode)
  {
    case READY_MODE :
      // enable ellipse menu
      EnableMenuItem(hMenu, ID_ELLIPSE,
                     MF_BYCOMMAND|MF_ENABLED);
      break;
    case DRAW_MODE :
    case DRAWN_MODE :
      // disable ellipse menu
      EnableMenuItem(hMenu, ID_ELLIPSE,
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
  gDrawData.hDrawPen=CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

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
  gDrawData.centre.x = gDrawData.centre.y = 0;

  gDrawData.drawMode = READY_MODE;

  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, 
         gDrawData.maxBoundary.cy, PATCOPY);

  reDraw(hwnd);
  setupMenus(hwnd);
}

void plot_sympoint(int ex, int ey, COLORREF clr)
{
  int cx = gDrawData.centre.x;
  int cy = gDrawData.centre.y;

  SetPixel(gDrawData.hdcMem, ex+cx,cy-ey, clr);
  SetPixel(gDrawData.hdcMem, -ex+cx,cy-ey, clr);
  SetPixel(gDrawData.hdcMem, -ex+cx,cy+ey, clr);
  SetPixel(gDrawData.hdcMem, ex+cx,cy+ey, clr);
}

void addPoint(HWND hwnd, int x, int y)
{
  switch (gDrawData.drawMode)
  {
    case DRAW_MODE:
      /* the coordinates of the centre is stored 
         and the ellipse is drawn */
      SelectObject(gDrawData.hdcMem, gDrawData.hDrawPen);
      drawPoint(x,y);
      gDrawData.centre.x = x;
      gDrawData.centre.y = y;
      drawEllipse(gDrawData.iMajorDiameter, 
                  gDrawData.iMinorDiameter);
      setDrawMode(DRAWN_MODE, hwnd);
      reDraw(hwnd);
      break;
    case DRAWN_MODE:
      MessageBox(hwnd,
       "Ellipse already drawn, now you can clear the area", 
          "Warning",MB_OK);
      break;
    default:
      break;
  }
}

void drawImage(HDC hdc)
{
  BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx, 
    gDrawData.maxBoundary.cy, gDrawData.hdcMem, 
    0, 0, SRCCOPY);
}

void processLeftButtonDown(HWND hwnd, int x, int y)
{
  addPoint(hwnd,x,y);
}

void processCommand(int cmd, HWND hwnd)
{
  int response;
  switch(cmd)
  {
    case ID_CLEAR:
      reset(hwnd);
      setDrawMode(READY_MODE, hwnd);
      break;
    case ID_ELLIPSE:
      setDrawMode(DRAW_MODE, hwnd);
      DialogBox(NULL,"MyDB",hwnd,(DLGPROC)DlgAxis);
      break;
    case ID_EXIT:
        response=MessageBox(hwnd,
          "Quit the program?", "EXIT", 
          MB_YESNO);
        if(response==IDYES) 
            PostQuitMessage(0);
        break;
    default:
      break;
  }
}

void drawEllipse(int ma,int mb)
{
  int x,y;
  x=0;
  y=mb;
  plot_sympoint(x,y, RGB(0,0,0));

  double d=(mb*mb)-(ma*ma*mb)+(ma*ma/4);
  while((ma*ma*(y-(1/2)))>(mb*mb*(x+1)))  
  {
    /* in region 1*/
    if(d<0)
    {
      /*choose E*/
      d=d+(mb*mb*(2*x+3));
      x=x+1;
    }
    else                              
    {
      /*choose SE*/
      d=d+(mb*mb*(2*x+3))+(ma*ma*(2-2*y));
      x=x+1;
      y=y-1;
    }
    plot_sympoint(x,y, RGB(0,0,0));
  }
  d=(mb*mb*(x+(1/2))*(x+1/2))
    +ma*ma*(y-1)*(y-1)-(ma*ma*mb*mb);
  while(y>0)   
  {
    /*in region 2*/
    if(d<0)
    {
      /*choose SE*/
      d=d+mb*mb*(2*x+2)+ma*ma*(3-2*y);
      x=x+1;
      y=y-1;
    }
    else
    {
      /*choose S*/
      d=d+ma*ma*(3-2*y);
      y=y-1;
    }
    plot_sympoint(x,y, RGB(0,0,0));
  }
}

LRESULT CALLBACK DlgAxis(HWND hdlg,UINT mess,WPARAM more,LPARAM pos)
{
  char str[20];
  switch(mess)
  {
    case WM_COMMAND:
      switch(more)
      {
        case ID_OK:
          GetDlgItemText(hdlg,ID_EB1,str,20);
          gDrawData.iMajorDiameter=(long int)(atof(str))/2;
          GetDlgItemText(hdlg,ID_EB2,str,20);
          gDrawData.iMinorDiameter=(long int)(atof(str))/2;
          if(gDrawData.iMajorDiameter < 5 || 
             gDrawData.iMajorDiameter > 700 ||
             gDrawData.iMinorDiameter < 5 || 
             gDrawData.iMinorDiameter > 400)
          {
            MessageBox(hdlg,
              "Diameter should be between 5 and 400.", 
              "Warning!",MB_ICONERROR);
          } 
          EndDialog(hdlg,TRUE);
          return 1;
          break;

        case ID_CANCEL:
          EndDialog(hdlg,TRUE);
          break;
      }
      break;
    default:
      break;
  }
  return 0;
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

    case WM_PAINT:
      hdc = BeginPaint(hwnd, &ps);
      drawImage(hdc);
      EndPaint(hwnd, &ps);
      break;

    case WM_DESTROY:
      cleanup();
      PostQuitMessage(0);
      break;
  }
  // Call the default window handler
  return DefWindowProc(hwnd, message, wParam, lParam);
}