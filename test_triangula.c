
#include <stdio.h>
#include <stdlib.h>

//-------------------- Función área 2D --------------------
double area2D(double x0,double y0,double x1,double y1,double x2,double y2){
    return (x1-x0)*(y2-y0) - (y1-y0)*(x2-x0);
}


//-------------------- Triangulación de banda --------------------
int triangula_banda_area(
    const double *x1,const double *y1,int n1,const int *idx1,
    const double *x2,const double *y2,int n2,const int *idx2,
    int *triangles
){
    int t=0;
    int i1=0, i2=0;

    while(i1<n1-1 || i2<n2-1){
        printf("i1=%d n1=%d i2=%d n2=%d\n", i1, n1, i2, n2);
        if(i1 == n1-1){
            triangles[t++] = idx1[i1];
            triangles[t++] = idx2[i2];
            triangles[t++] = idx2[i2+1];
            i2++;
            continue;
        }
        if(i2 == n2-1){
            triangles[t++] = idx1[i1];
            triangles[t++] = idx1[i1+1];
            triangles[t++] = idx2[i2];
            i1++;
            continue;
        }

        double area1 = area2D(x1[i1],y1[i1], x1[i1+1],y1[i1+1], x2[i2],y2[i2]);
        double area2 = area2D(x1[i1],y1[i1], x2[i2],y2[i2], x2[i2+1],y2[i2+1]);
        printf("area1=%f area2=%f\n", area1, area2);

        if(area1<0 && area2<0) return 0;
        else if(area1>0 && area2<=0){
            triangles[t++] = idx1[i1];
            triangles[t++] = idx1[i1+1];
            triangles[t++] = idx2[i2];
            i1++;
        }
        else if(area2>0 && area1<=0){
            triangles[t++] = idx1[i1];
            triangles[t++] = idx2[i2];
            triangles[t++] = idx2[i2+1];
            i2++;
        }
        else{
            if(area1<area2){
                triangles[t++] = idx1[i1];
                triangles[t++] = idx1[i1+1];
                triangles[t++] = idx2[i2];
                i1++;
            }else{
                triangles[t++] = idx1[i1];
                triangles[t++] = idx2[i2];
                triangles[t++] = idx2[i2+1];
                i2++;
            }
        }
    }

    return 1;
}

//-------------------- Test --------------------
int main(){
    // Polígono 1: 4 puntos (cuadrado)
    double x1[] = {0,1,1,2};
    double y1[] = {0,0,1,2};
    int idx1[] = {0,1,2,3};
    int n1 = 4;

    // Polígono 2: 3 puntos (triángulo)
    double x2[] = {0.2,0.8,0.5};
    double y2[] = {2,2,2.5};
    int idx2[] = {4,5,6};
    int n2 = 3;

    int num_tri = n1 + n2 - 2;
    int *triangles = (int*)malloc(sizeof(int)*3*num_tri);

    int res = triangula_banda_area(x1,y1,n1,idx1, x2,y2,n2,idx2, triangles);

    if(res==0){
        printf("Error: triangulación no válida\n");
        free(triangles);
        return 1;
    }

    printf("Triángulos generados (%d):\n", num_tri);
    for(int i=0;i<num_tri;i++){
        printf("Tri %d: %d %d %d\n", i, triangles[3*i], triangles[3*i+1], triangles[3*i+2]);
    }

    free(triangles);
    return 0;
}