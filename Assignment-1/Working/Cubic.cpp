#include <math.h>
#include <windows.h>
#include "Cubic.h"
#include <tchar.h>
#include <algorithm>

#define DBL double
DRAWING_DATA gDrawData; // global data
DBL xcoeff[4], ycoeff[4];
void drawEllipse(int ma,int mb);
void drawSpline(DBL a[], DBL b[]);
void drawSplineByMidPointAlgo(DBL a[], DBL b[]);
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

void drawImage(HDC hdc)
{
	BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx,
		gDrawData.maxBoundary.cy, gDrawData.hdcMem,
		0, 0, SRCCOPY);
}

// Plots(Setting up the pixel) a point at (ex, ey)
// with the origin being at (cx, cy)
void plotPoint(int ex, int ey, COLORREF clr)
{
	ey = -ey;
	int cx = gDrawData.centre.x;
	int cy = gDrawData.centre.y;

	SetPixel(gDrawData.hdcMem, cx + ex, cy + ey, clr);
	// drawImage(gDrawData.hdcMem);
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
	  drawSplineByMidPointAlgo(xcoeff, ycoeff);
      setDrawMode(DRAWN_MODE, hwnd);
      reDraw(hwnd);
      break;
    case DRAWN_MODE:
      MessageBox(hwnd,
		  TEXT("Ellipse already drawn, now you can clear the area"),
		  TEXT("Warning"), MB_OK);
      break;
    default:
      break;
  }
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
	  DialogBox(NULL, TEXT("MyDB"), hwnd, (DLGPROC)DlgAxis);
      break;
    case ID_EXIT:
        response=MessageBox(hwnd,
			TEXT("Quit the program?"), TEXT("EXIT"),
          MB_YESNO);
        if(response==IDYES) 
            PostQuitMessage(0);
        break;
    default:
      break;
  }
}

// Finds one of the coordinate ( x or y) values depending upon the coefficient values
DBL findCoordinate(DBL u, DBL coeff[])
{
	return coeff[0] + coeff[1] * u + coeff[2] * u*u + coeff[3] * u*u*u;
}

// Finds the derivative of the parametric function at a given value of u. Coefficient has to be passed.
DBL finddYbydX(DBL u, DBL coeff[])
{
	return coeff[1] + 2 * coeff[2] * u + 3 * coeff[3] * u*u;
}

//Finds the derivates sign over various intervals.
void finddYbydXSign(DBL A[], int (&isPostive)[3], DBL (&intervals)[3])
{
	if (A[3] == 0)
	{
		if (A[2] == 0)
		{
			if (A[1] == 0)
			{
				isPostive[0] = 0;
				intervals[0] = 1.0;
			}
			else
			{
				isPostive[0] = (A[1] > 0.0) ? 1 : (-1);
				intervals[0] = 1.0;
			}
		}
		else
		{
			int slopeSign = (A[2] > 0) ? 1 : (-1);
			DBL intercept = (-1) * (A[1] / (2 * A[2]));

			if (intercept <= 0.0)
			{
				isPostive[0] = slopeSign;
				intervals[0] = 1.0;
			}
			else if (intercept >= 1.0)
			{
				isPostive[0] = slopeSign * (-1);
				intervals[0] = 1.0;
			}
			else
			{
				isPostive[0] = slopeSign * (-1);
				intervals[0] = intercept;
				isPostive[1] = slopeSign;
				intervals[1] = 1.0;
			}
		}
	}
	else
	{
		DBL a = 3 * A[3], b = 2 * A[2], c = A[1];
		DBL discriminant = b*b - 4 * a*c;

		if (discriminant <= 0.0)
		{
			isPostive[0] = ((a>0) ? 1 : (-1));
			intervals[0] = 1.0;
		}
		else
		{
			DBL x = (-b + sqrt(discriminant)) / (2 * a);
			DBL y = (-b - sqrt(discriminant)) / (2 * a);

			if (y < x)
			{
				DBL temp = y;
				y = x;
				x = temp;
			}

			if (x >= 1.0 || y <= 0.0)
			{
				isPostive[0] = ((a > 0) ? 1 : (-1));
				intervals[0] = 1.0;
			}
			else
			{
				int i = 0;
				if (x > 0.0)
				{
					isPostive[i] = ((a > 0) ? 1 : (-1));
					intervals[i] = x;
					i++;
				}

				if (y < 1.0)
				{
					isPostive[i] = ((a > 0) ? (-1) : 1);
					intervals[i] = y;
					i++;

					isPostive[i] = ((a > 0) ? 1 : (-1));
					intervals[i] = 1.0;
				}
				else
				{
					isPostive[i] = ((a > 0) ? (-1) : 1);
					intervals[i] = 1.0;
				}
			}
		}
	}
}

// Finds the other coordinate.
DBL findYforX(int x, DBL A[], int y, DBL B[], DBL& U)
{
	if (A[3] != 0)
	{
		DBL a = A[3], b = A[2], c = A[1], d = A[0] - x;
		DBL f = (((3 * c) / a) - ((b*b) / (a*a))) / 3;
		DBL g = (((2 * b*b*b) / (a*a*a)) - ((9 * b*c) / (a*a)) + ((27 * d) / a)) / 27;
		DBL h = (((g*g) / 4) + ((f*f*f) / 27));

		if (h > 0)
		{
			DBL r = (g / (-2.0)) + sqrt(h);
			DBL s = ((r<0) ? (-1) : 1) * pow(abs(r), (1.0) / 3);
			DBL t = (g / (-2.0)) - sqrt(h);
			DBL u = ((t<0) ? (-1) : 1) * pow(abs(t), (1.0) / 3);
			U = (s + u) - (b / (3 * a));

			return findCoordinate(U, B);
		}
		else if (h == 0.0 && f == 0.0 && g == 0.0)
		{
			DBL U = (((d / a) < 0) ? 1 : -1) * pow(abs(d / a), (1.0 / 3));

			return findCoordinate(U, B);
		}
		else if (h <= 0)
		{
			DBL i = sqrt(((g*g) / 4) - h);
			DBL j = ((i<0) ? (-1) : 1) * pow(abs(i), (1.0) / 3);
			DBL k = acos(-(g / (2 * i)));
			DBL l = j * (-1);
			DBL m = cos(k / 3);
			DBL n = sqrt(3.0) * sin(k / 3);
			DBL p = (b / (3 * a)) * (-1);
			DBL u1 = (2 * j * cos(k / 3)) - (b / (3 * a));
			DBL u2 = l*(m + n) + p;
			DBL u3 = l*(m - n) + p;

			DBL y1 = findCoordinate(u1, B);
			DBL y2 = findCoordinate(u2, B);
			DBL y3 = findCoordinate(u3, B);

			U = (abs(y1 - y) < abs(y2 - y)) ? u1 : u2;
			U = (abs(findCoordinate(U, B) - y) < abs(y3 - y)) ? U : u3;
			return findCoordinate(U, B);
		}
	}
	else if (A[2] != 0)
	{
		DBL a = A[2], b = A[1], c = A[0] - x;
		DBL discriminant = b*b - 4 * a*c;

		if (discriminant == 0.0)
		{
			U = (-b) / (2 * a);
			return findCoordinate(U, B);
		}
		else if (discriminant > 0.0)
		{
			DBL u1 = (-b + sqrt(discriminant)) / (2 * a);
			DBL u2 = (-b - sqrt(discriminant)) / (2 * a);

			U = (abs(findCoordinate(u1, B) - y) < abs(findCoordinate(u2, B) - y)) ? u1 : u2;
			return findCoordinate(U, B);
		}
	}
	else if (A[1] != 0)
	{
		DBL a = A[1], b = A[0] - x;
		
		U = (-b) / a;
		return findCoordinate(U, B);
	}
}

void drawSplineByMidPointAlgo(DBL a[], DBL b[])
{

	int isPositiveX[3], isPositiveY[3];
	DBL intervalsX[3], intervalsY[3];
	DBL curU = 0.0, nxtUVal;
	int i = 0, j = 0;
	int x = a[0], y = b[0];
	DBL x1, y1;



	finddYbydXSign(a, isPositiveX, intervalsX);
	finddYbydXSign(b, isPositiveY, intervalsY);
	
	plotPoint(x, y, RGB(0, 0, 0));

	while ( !(isPositiveX[0] == 0 && isPositiveY[0] == 0) 
			&& curU <= 1.0
		)
	{
		nxtUVal = min(intervalsX[i], intervalsY[j]);

		while (curU <= nxtUVal)
		{
			// sampling on x because dy/dx < 1
			if (abs(finddYbydX(curU, b)) < abs(finddYbydX(curU, a)))
			{
				x += isPositiveX[i]; 
				y1 = findYforX(x, a, y, b, curU);
				if ((abs(y1 - y) > 0.5)){
					y += isPositiveY[j];
				}
			}
			else
			{
				y += isPositiveY[j];
				x1 = findYforX(y, b, x, a, curU);
				if (abs(x1 - x) > 0.5)
				{
					x += isPositiveX[i];
				}
			}

			plotPoint(x, y, RGB(0, 0, 0));
		}

		if (intervalsX[i] < intervalsY[j])
			i++;
		else
			j++;
	}
}

void drawSplineBySamplingU(DBL a[], DBL b[])
{
	int x, y;
	for (DBL u = 0.0; u <= 1.0; u += 0.001)
	{
		x = floor(a[0] + a[1] * u + a[2] * u*u + a[3] * u*u*u);
		y = floor(b[0] + b[1] * u + b[2] * u*u + b[3] * u*u*u);
		plotPoint(x, y, RGB(0, 0, 0));
	}
}


LRESULT CALLBACK DlgAxis(HWND hdlg,UINT mess,WPARAM more,LPARAM pos)
{
  TCHAR str[20];
  switch(mess)
  {
  case WM_INITDIALOG:
	  SetDlgItemInt(hdlg, ID_EB1, (UINT)1, FALSE);
	  SetDlgItemInt(hdlg, ID_EB2, (UINT)80, FALSE);
	  SetDlgItemInt(hdlg, ID_EB3, (UINT)1, FALSE);
	  SetDlgItemInt(hdlg, ID_EB4, (UINT)1, FALSE);
	  SetDlgItemInt(hdlg, ID_EB5, (UINT)1, FALSE);
	  SetDlgItemInt(hdlg, ID_EB6, (UINT)1, FALSE);
	  SetDlgItemInt(hdlg, ID_EB7, (UINT)1, FALSE);
	  SetDlgItemInt(hdlg, ID_EB8, (UINT)90, FALSE);
    case WM_COMMAND:
      switch(more)
      {
        case ID_OK:
          /*GetDlgItemText(hdlg,ID_EB1,str,20);
          gDrawData.iMajorDiameter=(long int)(_tstof(str))/2;
          GetDlgItemText(hdlg,ID_EB2,str,20);
          gDrawData.iMinorDiameter=(long int)(_tstof(str))/2;
          if(gDrawData.iMajorDiameter < 5 || 
             gDrawData.iMajorDiameter > 700 ||
             gDrawData.iMinorDiameter < 5 || 
             gDrawData.iMinorDiameter > 400)
          {
            MessageBox(hdlg,
              TEXT("Diameter should be between 5 and 400."), 
			  TEXT("Warning!"), MB_ICONERROR);
          }*/

		  GetDlgItemText(hdlg, ID_EB1, str, 20);
		  xcoeff[0] = (DBL)(_tstof(str));
		  GetDlgItemText(hdlg, ID_EB2, str, 20);
		  xcoeff[1] = (DBL)(_tstof(str));
		  GetDlgItemText(hdlg, ID_EB3, str, 20);
		  xcoeff[2] = (DBL)(_tstof(str));
		  GetDlgItemText(hdlg, ID_EB4, str, 20);
		  xcoeff[3] = (DBL)(_tstof(str));
		  
		  GetDlgItemText(hdlg, ID_EB5, str, 20);
		  ycoeff[0] = (DBL)(_tstof(str));
		  GetDlgItemText(hdlg, ID_EB6, str, 20);
		  ycoeff[1] = (DBL)(_tstof(str));
		  GetDlgItemText(hdlg, ID_EB7, str, 20);
		  ycoeff[2] = (DBL)(_tstof(str));
		  GetDlgItemText(hdlg, ID_EB8, str, 20);
		  ycoeff[3] = (DBL)(_tstof(str));

          EndDialog(hdlg,TRUE);
          return 1;
          break;

        case ID_CANCEL:
          EndDialog(hdlg,TRUE);
          break;
		default:
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