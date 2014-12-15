#include <windows.h>
#include <math.h>
#include "DrawUtil3D.h"
#include "Global3D.h"

LRESULT CALLBACK WindowF(HWND hwnd,UINT message,WPARAM wParam,
                         LPARAM lParam) 
{
  HDC hdc;
  PAINTSTRUCT ps;

  switch(message)
  { 
    case WM_CREATE:
      hdc = GetDC(hwnd);
      initialize(hwnd, hdc);
      initialize_shapeData();
      setShapeData();
      drawWireframe(clrEdge);
      ReleaseDC(hwnd, hdc);
      break;
    
    case WM_PAINT:
      hdc = BeginPaint(hwnd, &ps);
      drawImage(hdc);
      EndPaint(hwnd, &ps);
      break;

    case WM_LBUTTONDOWN:
      genImage(hwnd);
      break;

    case WM_DESTROY:
      cleanup();
      PostQuitMessage(0);
      break;

    default:
      break;
  } 
  return DefWindowProc(hwnd, message, wParam, lParam);
}
