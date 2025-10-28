#include <stdio.h>
#include <math.h>

/*------------------------------------------------------------
   Rutinas de procesamiento (por ahora vacías)
------------------------------------------------------------*/

void procesa_punto(double ax, double ay)
{
    /* Aquí se puede guardar el punto, imprimirlo, etc. */
    (void)ax; (void)ay; /* evita warning */
}

int procesa_cuadrilatero(int i0, int i1,
                         double ax, double ay,
                         double bx, double by)
{
    /* Aquí se podría construir un elemento cuadrilátero o acumular datos */
    (void)i0; (void)i1; (void)ax; (void)ay; (void)bx; (void)by;
    return 1; /* devolver 0 para anular la sustitución a=b */
}

void procesa_triangulo(int i0, int i1, double ax, double ay)
{
    /* Aquí se podría construir un triángulo degenerado o similar */
    (void)i0; (void)i1; (void)ax; (void)ay;
}

/*------------------------------------------------------------
   Genera una curva paralela a la derecha.
   La curva está definida por índices (1-based) sobre arrays globales x[], y[].
------------------------------------------------------------*/
void offset_curve(double *x, double *y, int *ind, int n, double h)
{
    int i;
    double dx1, dy1, dx2, dy2, nx, ny, len;
    double ax, ay, bx, by;

    if (n < 2) return;

    /* --- primer punto desplazado --- */
    int i0 = ind[0] - 1;
    int i1 = ind[1] - 1;
    dx1 = x[i1] - x[i0];
    dy1 = y[i1] - y[i0];
    len = sqrt(dx1*dx1 + dy1*dy1);
    if (len == 0) return;
    nx =  dy1 / len;
    ny = -dx1 / len;
    ax = x[i0] + h * nx;
    ay = y[i0] + h * ny;
    procesa_punto(ax, ay);

    /* --- procesamos tramo a tramo --- */
    for (i = 0; i < n - 1; i++) {
        i0 = ind[i] - 1;
        i1 = ind[i+1] - 1;

        dx1 = x[i1] - x[i0];
        dy1 = y[i1] - y[i0];
        len = sqrt(dx1*dx1 + dy1*dy1);
        if (len == 0) continue;

        /* bisectriz si hay un siguiente tramo */
        if (i < n - 2) {
            int i2 = ind[i+2] - 1;
            dx2 = x[i2] - x[i1];
            dy2 = y[i2] - y[i1];
            double len1 = sqrt(dx1*dx1 + dy1*dy1);
            double len2 = sqrt(dx2*dx2 + dy2*dy2);
            nx = (dy1/len1 + dy2/len2) * 0.5;
            ny = (-dx1/len1 - dx2/len2) * 0.5;
            len = sqrt(nx*nx + ny*ny);
            if (len != 0) { nx /= len; ny /= len; }
        } else {
            nx =  dy1 / len;
            ny = -dx1 / len;
        }

        bx = x[i1] + h * nx;
        by = y[i1] + h * ny;

        /* comprobar intersección de (P_i,B) y (P_{i+1},A) */
        double x1 = x[i0], y1 = y[i0];
        double x2 = bx,    y2 = by;
        double x3 = x[i1], y3 = y[i1];
        double x4 = ax,    y4 = ay;

        double den = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
        int intersect = 0;
        if (fabs(den) > 1e-12) {
            double t = ((x1-x3)*(y3-y4) - (y1-y3)*(x3-x4)) / den;
            double u = ((x1-x3)*(y1-y2) - (y1-y3)*(x1-x2)) / den;
            if (t >= 0 && t <= 1 && u >= 0 && u <= 1) intersect = 1;
        }

        if (!intersect) {
            int r = procesa_cuadrilatero(i0, i1, ax, ay, bx, by);
            if (r) { ax = bx; ay = by; }
        } else {
            procesa_triangulo(i0, i1, ax, ay);
        }
    }
}

/*------------------------------------------------------------
   Programa principal de prueba
------------------------------------------------------------*/
int main(void)
{
    double x[] = {0.0, 1.0, 1.0, 2.0};
    double y[] = {0.0, 0.0, 1.0, 1.0};
    int ind[] = {1, 2, 3, 4};
    int n = 4;
    double h = 0.2;

    offset_curve(x, y, ind, n, h);
    return 0;
}
