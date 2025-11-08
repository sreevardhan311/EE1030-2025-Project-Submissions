#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\c_libs\pgm_io.h"

float* read_pgm(const char *path, int *rows, int *cols){
    FILE *f = fopen(path, "rb");
    if(!f){ fprintf(stderr,"Cannot open %s\n", path); exit(1); }

    char magic[3];
    if(fscanf(f, "%2s", magic) != 1 || strcmp(magic, "P5") != 0){
        fprintf(stderr,"%s is not a binary PGM (P5)\n", path);
        fclose(f); exit(1);
    }

    int w,h,maxv;
    fscanf(f, "%d %d %d", &w, &h, &maxv);
    fgetc(f);

    size_t count = (size_t)w*h;
    unsigned char *buf8 = (unsigned char*)malloc(count);
    fread(buf8,1,count,f);
    fclose(f);

    float *buf = (float*)malloc(count*sizeof(float));
    for(size_t i=0;i<count;i++) buf[i]=(float)buf8[i];
    free(buf8);

    *rows=h; *cols=w;
    return buf;
}

void write_pgm(const char *path, const float *buf, int rows, int cols){
    FILE *f = fopen(path, "wb");
    fprintf(f,"P5\n%d %d\n255\n", cols, rows);
    size_t count=(size_t)rows*cols;
    unsigned char *buf8=(unsigned char*)malloc(count);
    for(size_t i=0;i<count;i++){
        float v=buf[i]; if(v<0)v=0;if(v>255)v=255;
        buf8[i]=(unsigned char)(v+0.5f);
    }
    fwrite(buf8,1,count,f);
    fclose(f);
    free(buf8);
}
