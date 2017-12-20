#include<stdio.h>
#include<math.h>
double power(double x, double y)
{
    double buf=1;
    while(y>0)
    {
        buf= buf*x;
        y--;
    }
    return buf;
}
double ln(double x)
{
    double buf=0;
    x=x-1;
    int n=1;
    while(n<21)
    {
        if(n%2==0)
            buf= buf-(power(x,n)/n);
        else
            buf= buf+(power(x,n)/n);
        n++;
    }
    return buf;
}
double expdev(double lambda) {
    double dummy;
    do
        dummy= (double) rand() / RAND_MAX;
    while (dummy == 0.0);
    return -ln(dummy) / lambda;
}
