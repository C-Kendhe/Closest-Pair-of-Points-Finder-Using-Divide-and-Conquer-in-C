/*
 * Closest Pair of Points Finder
 * Native Win32 GUI, C11.  Build instructions are in README.md.
 */
#define WIN32_LEAN_AND_MEAN
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define MAX_POINTS 1000
#define PANEL_W 270
#define MARGIN 26


typedef struct {
  double x, y;
  int id;    /* Stable input identity. */
  int xRank; /* Position in the initial x-sorted order. */
} Point;
typedef struct {
  Point a, b;
  double distance;
  int valid;
} Pair;
typedef enum { NONE, BRUTE, DIVIDE } Algorithm;

#ifndef CLOSEST_PAIR_TEST
static Point points[MAX_POINTS];
static int pointCount = 0;
static Pair closest = {0};
static Algorithm selected = NONE;
static double bruteMs = -1.0, divideMs = -1.0;
static HWND hCount, hRunBrute, hRunDivide, hCompare, hClear, hStatus;
static LARGE_INTEGER performanceFrequency;
#endif

static double dist(Point a, Point b) {
  double dx = a.x - b.x, dy = a.y - b.y;
  return sqrt(dx * dx + dy * dy);
}
static Pair invalidPair(void) {
  Pair p = {{0, 0, 0, 0}, {0, 0, 0, 0}, 1e300, 0};
  return p;
}
static Pair better(Pair a, Pair b) {
  return (!a.valid || (b.valid && b.distance < a.distance)) ? b : a;
}
static Pair makePair(Point a, Point b) {
  Pair p = {a, b, dist(a, b), 1};
  return p;
}
static int distancesMatch(double a, double b) {
  double scale = fmax(1.0, fmax(fabs(a), fabs(b)));
  return fabs(a - b) <= 1e-9 * scale;
}

static Pair bruteForce(const Point *p, int n) {
  Pair best = invalidPair();
  int i, j;
  for (i = 0; i < n; ++i)
    for (j = i + 1; j < n; ++j)
      best = better(best, makePair(p[i], p[j]));
  return best;
}
static int cmpX(const void *a, const void *b) {
  const Point *p = a, *q = b;
  if (p->x != q->x)
    return (p->x > q->x) - (p->x < q->x);
  if (p->y != q->y)
    return (p->y > q->y) - (p->y < q->y);
  return (p->id > q->id) - (p->id < q->id);
}
static int cmpY(const void *a, const void *b) {
  const Point *p = a, *q = b;
  if (p->y != q->y)
    return (p->y > q->y) - (p->y < q->y);
  if (p->x != q->x)
    return (p->x > q->x) - (p->x < q->x);
  return (p->id > q->id) - (p->id < q->id);
}
/* px and py are respectively sorted by x and y.  Maintaining both orders
   avoids re-sorting the strip at every recursive level: O(n log n). */
static Pair divideRec(const Point *px, const Point *py, int n) {
  Point *leftY, *rightY, *strip;
  Pair left, right, best;
  double midX;
  int mid, i, j, li = 0, ri = 0, count = 0, middleRank;
  if (n <= 3)
    return bruteForce(px, n);
  mid = n / 2;
  midX = px[mid].x;
  middleRank = px[mid].xRank;
  leftY = malloc((size_t)mid * sizeof(*leftY));
  rightY = malloc((size_t)(n - mid) * sizeof(*rightY));
  if (!leftY || !rightY) {
    free(leftY);
    free(rightY);
    return bruteForce(px, n);
  }
  /* xRank is a unique x-order identity, so py gets precisely px's two subsets.
   */
  for (i = 0; i < n; ++i) {
    if (py[i].xRank < middleRank)
      leftY[li++] = py[i];
    else
      rightY[ri++] = py[i];
  }
  if (li != mid || ri != n - mid) {
    free(leftY);
    free(rightY);
    return bruteForce(px, n);
  }
  left = divideRec(px, leftY, mid);
  right = divideRec(px + mid, rightY, n - mid);
  best = better(left, right);
  strip = malloc((size_t)n * sizeof(*strip));
  if (!strip) {
    /* Allocation failure must not suppress cross-partition candidates. */
    free(leftY);
    free(rightY);
    return bruteForce(px, n);
  }
  for (i = 0; i < n; ++i)
    if (fabs(py[i].x - midX) < best.distance)
      strip[count++] = py[i];
  for (i = 0; i < count; ++i)
    for (j = i + 1; j < count && strip[j].y - strip[i].y < best.distance; ++j)
      best = better(best, makePair(strip[i], strip[j]));
  free(strip);
  free(leftY);
  free(rightY);
  return best;
}
static Pair divideConquer(const Point *source, int n) {
  Point *px, *py;
  int *rankById;
  Pair result;
  int i;
  if (n < 2)
    return invalidPair();
  px = malloc((size_t)n * sizeof(*px));
  py = malloc((size_t)n * sizeof(*py));
  rankById = malloc((size_t)n * sizeof(*rankById));
  if (!px || !py || !rankById) {
    free(px);
    free(py);
    free(rankById);
    return invalidPair();
  }
  for (i = 0; i < n; ++i) {
    px[i] = source[i];
    py[i] = source[i];
    px[i].id = py[i].id = i;
  }
  qsort(px, n, sizeof(*px), cmpX);
  for (i = 0; i < n; ++i) {
    px[i].xRank = i;
    rankById[px[i].id] = i;
  }
  qsort(py, n, sizeof(*py), cmpY);
  for (i = 0; i < n; ++i)
    py[i].xRank = rankById[py[i].id];
  result = divideRec(px, py, n);
  free(px);
  free(py);
  free(rankById);
  return result;
}
#ifndef CLOSEST_PAIR_TEST
static void initTiming(void) {
  QueryPerformanceFrequency(&performanceFrequency);
}
static double nowMs(void) {
  LARGE_INTEGER counter;
  if (performanceFrequency.QuadPart == 0)
    initTiming();
  if (performanceFrequency.QuadPart == 0 || !QueryPerformanceCounter(&counter))
    return 0.0;
  return 1000.0 * (double)counter.QuadPart /
         (double)performanceFrequency.QuadPart;
}
static void setStatus(const char *message) { SetWindowTextA(hStatus, message); }
static void updateCount(void) {
  char s[64];
  sprintf(s, "Points: %d / %d", pointCount, MAX_POINTS);
  SetWindowTextA(hCount, s);
}
static void run(Algorithm a) {
  double t;
  char msg[180];
  if (pointCount < 2) {
    setStatus("Add at least two points to run an algorithm.");
    return;
  }
  t = nowMs();
  closest = (a == BRUTE) ? bruteForce(points, pointCount)
                         : divideConquer(points, pointCount);
  t = nowMs() - t;
  selected = a;
  if (!closest.valid) {
    setStatus("Algorithm could not allocate required memory.");
    return;
  }
  if (a == BRUTE)
    bruteMs = t;
  else
    divideMs = t;
  sprintf(msg, "%s: closest distance = %.3f pixels | %.4f ms",
          a == BRUTE ? "Brute force" : "Divide & conquer", closest.distance, t);
  setStatus(msg);
  InvalidateRect(GetParent(hStatus), NULL, TRUE);
}
static void compare(void) {
  double t;
  char msg[250];
  Pair bruteResult, divideResult;
  if (pointCount < 2) {
    setStatus("Add at least two points to compare.");
    return;
  }
  t = nowMs();
  bruteResult = bruteForce(points, pointCount);
  bruteMs = nowMs() - t;
  t = nowMs();
  divideResult = divideConquer(points, pointCount);
  divideMs = nowMs() - t;
  if (!bruteResult.valid || !divideResult.valid) {
    setStatus("Comparison failed: insufficient memory.");
    return;
  }
  if (!distancesMatch(bruteResult.distance, divideResult.distance)) {
    closest = bruteResult;
    selected = BRUTE;
    sprintf(msg, "Verification failed: brute %.12g, divide %.12g",
            bruteResult.distance, divideResult.distance);
    setStatus(msg);
    InvalidateRect(GetParent(hStatus), NULL, TRUE);
    return;
  }
  closest = divideResult;
  selected = DIVIDE;
  sprintf(msg,
          "Verified: brute %.4f ms | divide & conquer %.4f ms | speedup %.2fx",
          bruteMs, divideMs, divideMs > 0 ? bruteMs / divideMs : 0.0);
  setStatus(msg);
  InvalidateRect(GetParent(hStatus), NULL, TRUE);
}
static void generatePoints(HWND hwnd) {
  char text[32];
  int n, w, h, i;
  GetWindowTextA(GetDlgItem(hwnd, 101), text, sizeof(text));
  n = atoi(text);
  if (n < 2 || n > MAX_POINTS) {
    setStatus("Enter a value from 2 to 1000.");
    return;
  }
  {
    RECT r;
    GetClientRect(hwnd, &r);
    w = r.right - PANEL_W - 2 * MARGIN;
    h = r.bottom - 2 * MARGIN;
  }
  for (i = 0; i < n; ++i) {
    points[i].x = PANEL_W + MARGIN + rand() % (w > 1 ? w : 2);
    points[i].y = MARGIN + rand() % (h > 1 ? h : 2);
    points[i].id = i;
    points[i].xRank = 0;
  }
  pointCount = n;
  closest.valid = 0;
  selected = NONE;
  updateCount();
  setStatus("Random points generated. Choose an algorithm.");
  InvalidateRect(hwnd, NULL, TRUE);
}
static void drawText(HDC dc, int x, int y, const char *s, COLORREF color,
                     int size) {
  HFONT f =
      CreateFontA(-size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                  CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
  COLORREF oldColor = SetTextColor(dc, color);
  int oldMode = SetBkMode(dc, TRANSPARENT);
  if (f) {
    HFONT oldFont = (HFONT)SelectObject(dc, f);
    TextOutA(dc, x, y, s, (int)strlen(s));
    SelectObject(dc, oldFont);
    DeleteObject(f);
  } else
    TextOutA(dc, x, y, s, (int)strlen(s));
  SetTextColor(dc, oldColor);
  SetBkMode(dc, oldMode);
}
#endif

#ifdef CLOSEST_PAIR_TEST
static int runCase(const char *name, const Point *input, int n,
                   double expected) {
  Pair brute = bruteForce(input, n), divide = divideConquer(input, n);
  int ok = brute.valid && divide.valid &&
           distancesMatch(brute.distance, expected) &&
           distancesMatch(brute.distance, divide.distance);
  printf("%-24s %s (brute %.12g, divide %.12g)\n", name, ok ? "PASS" : "FAIL",
         brute.distance, divide.distance);
  return ok ? 0 : 1;
}
static int runRandomConsistencyTest(void) {
  Point data[64];
  int sample, i;
  for (sample = 0; sample < 100; ++sample) {
    int n = 2 + rand() % 63;
    Pair brute, divide;
    for (i = 0; i < n; ++i) {
      data[i] = (Point){rand() % 17, rand() % 17, i, 0};
    }
    brute = bruteForce(data, n);
    divide = divideConquer(data, n);
    if (!brute.valid || !divide.valid ||
        !distancesMatch(brute.distance, divide.distance)) {
      printf("random consistency       FAIL (sample %d: %.12g vs %.12g)\n",
             sample, brute.distance, divide.distance);
      return 1;
    }
  }
  printf("random consistency       PASS (100 data sets)\n");
  return 0;
}
static int runSelfTests(void) {
  static const Point minimum[] = {{0, 0, 0, 0}, {3, 4, 1, 0}};
  static const Point duplicates[] = {
      {2, 2, 0, 0}, {7, 9, 1, 0}, {2, 2, 2, 0}, {5, 1, 3, 0}};
  static const Point collinear[] = {
      {0, 0, 0, 0}, {2, 0, 1, 0}, {5, 0, 2, 0}, {6, 0, 3, 0}};
  static const Point sameX[] = {
      {4, 1, 0, 0}, {4, 5, 1, 0}, {4, 2.25, 2, 0}, {4, 2.5, 3, 0}};
  static const Point sameY[] = {
      {1, 8, 0, 0}, {3, 8, 1, 0}, {3.4, 8, 2, 0}, {10, 8, 3, 0}};
  int failures = 0;
  failures += runCase("minimum input (2)", minimum, 2, 5.0);
  failures += runCase("duplicate points", duplicates, 4, 0.0);
  failures += runCase("collinear points", collinear, 4, 1.0);
  failures += runCase("identical X coordinates", sameX, 4, 0.25);
  failures += runCase("identical Y coordinates", sameY, 4, 0.4);
  failures += runRandomConsistencyTest();
  printf("\n%d test group(s) failed.\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif

#ifndef CLOSEST_PAIR_TEST
static LRESULT CALLBACK proc(HWND hwnd, UINT m, WPARAM w, LPARAM l) {
  switch (m) {
  case WM_CREATE: {
    HFONT f = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    CreateWindowA("STATIC", "CLOSEST PAIR", WS_CHILD | WS_VISIBLE, 18, 20, 220,
                  26, hwnd, 0, 0, 0);
    CreateWindowA("STATIC", "Click the canvas to add points",
                  WS_CHILD | WS_VISIBLE, 18, 55, 220, 20, hwnd, 0, 0, 0);
    hCount = CreateWindowA("STATIC", "Points: 0 / 1000", WS_CHILD | WS_VISIBLE,
                           18, 88, 210, 20, hwnd, 0, 0, 0);
    CreateWindowA("STATIC", "Random points:", WS_CHILD | WS_VISIBLE, 18, 124,
                  110, 20, hwnd, 0, 0, 0);
    CreateWindowA("EDIT", "100", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                  130, 120, 72, 24, hwnd, (HMENU)101, 0, 0);
    CreateWindowA("BUTTON", "Generate", WS_CHILD | WS_VISIBLE, 18, 158, 184, 29,
                  hwnd, (HMENU)1, 0, 0);
    hRunBrute =
        CreateWindowA("BUTTON", "Run Brute Force", WS_CHILD | WS_VISIBLE, 18,
                      205, 184, 31, hwnd, (HMENU)2, 0, 0);
    hRunDivide =
        CreateWindowA("BUTTON", "Run Divide & Conquer", WS_CHILD | WS_VISIBLE,
                      18, 244, 184, 31, hwnd, (HMENU)3, 0, 0);
    hCompare = CreateWindowA("BUTTON", "Compare Both", WS_CHILD | WS_VISIBLE,
                             18, 283, 184, 31, hwnd, (HMENU)4, 0, 0);
    hClear = CreateWindowA("BUTTON", "Clear Canvas", WS_CHILD | WS_VISIBLE, 18,
                           330, 184, 29, hwnd, (HMENU)5, 0, 0);
    hStatus =
        CreateWindowA("STATIC", "Add points or generate a data set.",
                      WS_CHILD | WS_VISIBLE, 18, 385, 228, 85, hwnd, 0, 0, 0);
    SendMessage(hCount, WM_SETFONT, (WPARAM)f, TRUE);
    return 0;
  }
  case WM_COMMAND:
    switch (LOWORD(w)) {
    case 1:
      generatePoints(hwnd);
      break;
    case 2:
      run(BRUTE);
      break;
    case 3:
      run(DIVIDE);
      break;
    case 4:
      compare();
      break;
    case 5:
      pointCount = 0;
      closest.valid = 0;
      selected = NONE;
      bruteMs = divideMs = -1;
      updateCount();
      setStatus("Canvas cleared.");
      InvalidateRect(hwnd, NULL, TRUE);
      break;
    }
    return 0;
  case WM_LBUTTONDOWN: {
    int x = LOWORD(l), y = HIWORD(l);
    if (x > PANEL_W && pointCount < MAX_POINTS) {
      points[pointCount] = (Point){x, y, pointCount, 0};
      ++pointCount;
      closest.valid = 0;
      selected = NONE;
      updateCount();
      setStatus("Point added. Add another point or run an algorithm.");
      InvalidateRect(hwnd, NULL, FALSE);
    }
    return 0;
  }
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hwnd, &ps);
    RECT r;
    int i;
    HPEN pen;
    HGDIOBJ oldObject;
    GetClientRect(hwnd, &r);
    FillRect(dc, &r, (HBRUSH)(COLOR_WINDOW + 1));
    pen = CreatePen(PS_SOLID, 1, RGB(210, 215, 225));
    if (pen) {
      oldObject = SelectObject(dc, pen);
      MoveToEx(dc, PANEL_W, 0, NULL);
      LineTo(dc, PANEL_W, r.bottom);
      SelectObject(dc, oldObject);
      DeleteObject(pen);
    }
    for (i = 0; i < pointCount; i++) {
      HBRUSH b = CreateSolidBrush(RGB(45, 105, 210));
      if (b) {
        oldObject = SelectObject(dc, b);
        Ellipse(dc, (int)points[i].x - 4, (int)points[i].y - 4,
                (int)points[i].x + 5, (int)points[i].y + 5);
        SelectObject(dc, oldObject);
        DeleteObject(b);
      }
    }
    if (closest.valid) {
      pen = CreatePen(PS_SOLID, 3, RGB(230, 65, 65));
      if (pen) {
        oldObject = SelectObject(dc, pen);
        MoveToEx(dc, (int)closest.a.x, (int)closest.a.y, NULL);
        LineTo(dc, (int)closest.b.x, (int)closest.b.y);
        SelectObject(dc, oldObject);
        DeleteObject(pen);
      }
      for (i = 0; i < 2; i++) {
        Point p = i ? closest.b : closest.a;
        HBRUSH b = CreateSolidBrush(RGB(235, 55, 55));
        if (b) {
          oldObject = SelectObject(dc, b);
          Ellipse(dc, (int)p.x - 7, (int)p.y - 7, (int)p.x + 8, (int)p.y + 8);
          SelectObject(dc, oldObject);
          DeleteObject(b);
        }
      }
    }
    drawText(dc, PANEL_W + 18, 18, "2D POINT PLANE", RGB(40, 45, 55), 18);
    drawText(dc, PANEL_W + 18, 46, "Blue = input points     Red = closest pair",
             RGB(90, 95, 105), 14);
    EndPaint(hwnd, &ps);
    return 0;
  }
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, m, w, l);
}
int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR cmd, int show) {
  WNDCLASSA wc = {0};
  HWND hwnd;
  MSG msg;
  (void)p;
  (void)cmd;
  initTiming();
  srand((unsigned)(time(NULL) ^ GetTickCount()));
  wc.lpfnWndProc = proc;
  wc.hInstance = h;
  wc.hCursor = LoadCursor(NULL, IDC_CROSS);
  wc.lpszClassName = "ClosestPairApp";
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  RegisterClassA(&wc);
  hwnd = CreateWindowA(wc.lpszClassName,
                       "Closest Pair of Points | Divide and Conquer",
                       WS_OVERLAPPEDWINDOW | WS_VISIBLE, 120, 90, 1100, 650,
                       NULL, NULL, h, NULL);
  ShowWindow(hwnd, show);
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}
#else
int main(void) {
  srand(12345U);
  return runSelfTests();
}
#endif
