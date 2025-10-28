                                                                                         
#include <stdio.h>
#include <math.h>

/*------------------------------------------------------------
   Rutina principal: genera una curva paralela a la derecha
   Entradas:
      x[], y[]  -> puntos de la curva original
      n         -> número de puntos
      h         -> desplazamiento
      x0[], y0[]-> arrays de salida
      nmax      -> tamaño máximo de los arrays de salida
   Devuelve:
      número de puntos generados (m)
      Si hay overflow, devuelve 0
------------------------------------------------------------*/
int offset_curve(double *x, double *y, int n, double h,
                 double *x0, double *y0, int nmax)
{
    int i, m = 0;
    double dx1, dy1, dx2, dy2, nx, ny, len, bx, by;
    double ax, ay;

    if (n < 2) return 0;

    /* --- normal inicial --- */
    dx1 = x[1] - x[0];
    dy1 = y[1] - y[0];
    len = sqrt(dx1 * dx1 + dy1 * dy1);
    if (len == 0) return 0;
    nx =  dy1 / len;
    ny = -dx1 / len;
    ax = x[0] + h * nx;
    ay = y[0] + h * ny;
    x0[m] = ax;
    y0[m] = ay;
    m++;

    for (i = 0; i < n - 1; i++) {
        /* --- dirección del tramo actual --- */
        dx1 = x[i+1] - x[i];
        dy1 = y[i+1] - y[i];
        len = sqrt(dx1 * dx1 + dy1 * dy1);
        if (len == 0) continue;
        nx =  dy1 / len;
        ny = -dx1 / len;

        /* --- dirección del siguiente tramo --- */
        if (i < n - 2) {
            dx2 = x[i+2] - x[i+1];
            dy2 = y[i+2] - y[i+1];
            len = sqrt(dx2 * dx2 + dy2 * dy2);
            if (len == 0) { dx2 = dx1; dy2 = dy1; }
            nx = (dy1 / sqrt(dx1*dx1 + dy1*dy1)
                + dy2 / sqrt(dx2*dx2 + dy2*dy2)) * 0.5;
            ny = (-dx1 / sqrt(dx1*dx1 + dy1*dy1)
                - dx2 / sqrt(dx2*dx2 + dy2*dy2)) * 0.5;
            len = sqrt(nx * nx + ny * ny);
            if (len != 0) { nx /= len; ny /= len; }
        }

        /* --- punto desplazado B --- */
        bx = x[i+1] + h * nx;
        by = y[i+1] + h * ny;

        /* --- comprobar intersección de (P_i,B) y (P_{i+1},A) --- */
        double x1 = x[i],   y1 = y[i];
        double x2 = bx,     y2 = by;
        double x3 = x[i+1], y3 = y[i+1];
        double x4 = ax,     y4 = ay;

        double den = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
        int intersect = 0;
        if (fabs(den) > 1e-12) {
            double t = ((x1-x3)*(y3-y4) - (y1-y3)*(x3-x4)) / den;
            double u = ((x1-x3)*(y1-y2) - (y1-y3)*(x1-x2)) / den;
            if (t >= 0 && t <= 1 && u >= 0 && u <= 1) intersect = 1;
        }

        if (!intersect) {
            if (m >= nmax) return 0; /* overflow */
            x0[m] = bx;
            y0[m] = by;
            m++;
            ax = bx;
            ay = by;
        }
    }

    return m;
}

/*------------------------------------------------------------
   Programa principal de prueba
------------------------------------------------------------*/
int main(void)
{
    /* Curva de prueba: un codo */
    double x[] = {0.0, 1.0, 1.0, 2.0};
    double y[] = {0.0, 0.0, 1.0, 1.0};
    int n = 4;
    double h = 0.2;

    double x0[100], y0[100];
    int m;

    m = offset_curve(x, y, n, h, x0, y0, 100);
    if (m == 0) {
        printf("Error o overflow.\n");
        return 1;
    }

    printf("Curva desplazada (%d puntos):\n", m);
    for (int i = 0; i < m; i++)
        printf("%2d: (%.4f, %.4f)\n", i, x0[i], y0[i]);

    return 0;
}
