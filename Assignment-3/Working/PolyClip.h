#include <vector>
#include "windows.h"

using namespace std;

typedef struct { double x, y; } vec_t, *vec;

vector<POINT> polycliphandler(vec_t *c, vec_t *s, int clen, int slen);