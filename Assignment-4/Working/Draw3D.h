#define ID_LABEL     40001
#define ID_CLEAR     40002
#define ID_EXIT      40003
#define ID_THETA     40004
#define ID_PHI       40005
#define ID_R         40006
#define ID_DIST      40007
#define ID_DRAW      40008 
#define ID_CANCEL    40009 

#define ID_VARLBL0   40010
#define ID_VARFLD0   40011
#define ID_VARLBL1   40012
#define ID_VARFLD1   40013
#define ID_VARLBL2   40014
#define ID_VARFLD2   40015

#define ID_CUBE      40016
#define ID_CONE      40017
#define ID_CYLINDER  40018
#define ID_SPHERE    40019

#define ID_TRANSLATE 40020
#define ID_ROTATE    40021
#define ID_SCALE     40022
#define ID_REFLECT	 40023

#define ID_VARP1XLBL   40024
#define ID_VARP1YLBL   40025
#define ID_VARP1ZLBL   40026
#define ID_VARP2XLBL   40027
#define ID_VARP2YLBL   40028
#define ID_VARP2ZLBL   40029
#define ID_VARP3XLBL   40030
#define ID_VARP3YLBL   40031
#define ID_VARP3ZLBL   40032

#define ID_VARP1X   40033
#define ID_VARP1Y   40034
#define ID_VARP1Z   40035
#define ID_VARP2X   40036
#define ID_VARP2Y   40037
#define ID_VARP2Z   40038
#define ID_VARP3X   40039
#define ID_VARP3Y   40040
#define ID_VARP3Z   40041


typedef enum
{
  READY_MODE,
  DRAW_MODE,
  DRAWN_MODE,
  TRANSLATE_MODE,
  ROTATE_MODE,
  SCALE_MODE,
  REFLECT_MODE,
  TRANSFORMED_MODE,
} MODE;
 
const COLOR clrBG = {255,255,255};
const COLOR clrTransform = {0,0,100};

typedef enum
{
  ROTATE_X,
  ROTATE_Y,
  ROTATE_Z
} ROTATE_TYPE;

LRESULT CALLBACK DlgTrnsltScale(HWND hdlg,UINT mess,
                                WPARAM more,LPARAM pos);
LRESULT CALLBACK DlgReflect(HWND hdlg, UINT mess,
	WPARAM more, LPARAM pos);
LRESULT CALLBACK DlgRotate(HWND hdlg,UINT mess,WPARAM more,LPARAM pos);
void translate3D(POINT3D & cartesianPt, double delx, 
                 double dely, double delz);
void translate3D(POINT3D & cartesianPt, const POINT3D& origin);
void rotate3D(POINT3D & cartesianPt, ROTATE_TYPE rotType, 
              double rotAngle);
void scale3D(POINT3D & cartesianPt, double sx, double sy, double sz);
void transformAllSurfaces();
void calcNormalVec(double p1x, double p1y, double p1z,
	double p2x, double p2y, double p2z, double p3x, double p3y, double p3z, double &alpha, double &beta);
void reflect3D(POINT3D & cartesianPt, double p1x, double p1y, double p1z,
	double alpha, double beta);
