#define GO_RIGHT 1
#define GO_LEFT  0

int LRrange(RECT* rect, int var, char xy);
int IsInsideRect(RECT* rect, POINT* pt);
double getMin(int len, double *x);
double getMax(int len, double *x);
double getMean(int len, double *x);
int iabs(int x);
short _double2short(double x);
double _short2double(short x);