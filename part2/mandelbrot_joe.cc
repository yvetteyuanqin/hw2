/**
 *  \file mandelbrot_joe.cc
 *
 *  \brief Implement your parallel mandelbrot set in this file.
 */
//Yuan Qin
//Lai Man Tang


#include <iostream>
#include <cstdlib>
#include <mpi.h>
#include "render.hh"

using namespace std;

#define WIDTH 1000
#define HEIGHT 1000

int
mandelbrot(double x, double y) {
    int maxit = 511;
    double cx = x;
    double cy = y;
    double newx, newy;
    
    int it = 0;
    for (it = 0; it < maxit && (x*x + y*y) < 4; ++it) {
        newx = x*x - y*y + cx;
        newy = 2*x*y + cy;
        x = newx;
        y = newy;
    }
    return it;
}


int
main (int argc, char* argv[])
{
    double minX = -2.1;
    double maxX = 0.7;
    double minY = -1.25;
    double maxY = 1.25;
    
    int height, width;
    if (argc == 3) {
        height = atoi (argv[1]);
        width = atoi (argv[2]);
        assert (height > 0 && width > 0);
    } else {
        fprintf (stderr, "usage: %s <height> <width>\n", argv[0]);
        fprintf (stderr, "where <height> and <width> are the dimensions of the image.\n");
        return -1;
    }
    
    double it = (maxY - minY)/height;
    double jt = (maxX - minX)/width;
    double x, y;
    
    
    /*MPI section*/
    int p, P, N;
    double *recvdata ;
    //double *sentdata ;

    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &p);
    MPI_Comm_size(MPI_COMM_WORLD, &P);
    
    N = height/P;
    double Jtime = MPI_Wtime();
    
    if(p == 0){
        recvdata = (double*)malloc(height *width *sizeof(double));
    }
    double *sentdata =(double*)malloc(N *width *sizeof(double));
    
    y = minY + p*N * it;
    int count =0;
    for (int i = p*N; i < (p+1)*N; i++) {
        x = minX;
        
        for (int j = 0; j < width; j++) {
            sentdata[count+j] = mandelbrot(x,y)/512.0;
            x += jt;
        }
        count+=width;
        y += it;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(sentdata,N*width,MPI_DOUBLE,recvdata+ p*N, N*width, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    if(p == 0){
        printf("Joe's algorithm time: %lf :",MPI_Wtime()-Jtime);
        /*Calculating Joe's algorithm*/
        
        //rendering image
        gil::rgb8_image_t img(height, width);
        auto img_view = gil::view(img);
        
        
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                img_view(j, i) = render(recvdata[j+i*width]);
            }
        }
        
        y = minY + P * N * it;
        for (int i = P * N; i < height; i++) {
            x = minX;
            for (int j = 0; j < width; j++) {
                img_view(j, i) = render(mandelbrot(x, y) / 512.0);
                x += jt;
            }
            y += it;
            
        }

        
        
        gil::png_write_view("mandelbrot_joe.png", const_view(img));
        
    }
    

    MPI_Finalize();
    return 0;
}

/* eof */
