#ifndef TRIANGULATE_H
#define TRIANGULATE_H

#include <stdint.h>
#include <string>
#include <string.h>
#include <iostream>

// polygon: (x,y,z),(x,y,z),...
void triangulate(uint32_t in_size, float* polygon, uint32_t** out_size, float** out_triangles);

#ifdef TRIANGULATE_IMPLEMENTATION

void copy_vec(float* polygon, uint32_t index, float* x, float* y, float* z) {
  memcpy(x, polygon + 3*index, sizeof(float));
  memcpy(y, polygon + 3*index + 1, sizeof(float));
  memcpy(z, polygon + 3*index + 2, sizeof(float));
}

void get_vec(float* polygon, uint32_t index, float** x, float** y, float** z) {
  *x = polygon + 3*index;
  *y = polygon + 3*index + 1;
  *z = polygon + 3*index + 2;
}

void triangulate_ring_no_holes(uint32_t num_vertices, float* ring, uint32_t** out_num_vertices, float** out_triangles) {
  std::cout << "Triangulating ring of size " << num_vertices << std::endl;

  if (num_vertices == 3) {
    *out_num_vertices = (uint32_t*)malloc(sizeof(uint32_t));
    *out_triangles = (float*)malloc(9*sizeof(float));

    **out_num_vertices = 3;
    memcpy(*out_triangles, ring, 9*sizeof(float));
  }
  else if (num_vertices == 4) {
    *out_num_vertices = (uint32_t*)malloc(sizeof(uint32_t));
    *out_triangles = (float*)malloc(18*sizeof(float));

    **out_num_vertices = 6;
    memcpy(*out_triangles, ring, 9*sizeof(float));
    memcpy(*out_triangles+9, ring+6, 6*sizeof(float));
    memcpy(*out_triangles+15, ring, 3*sizeof(float));
  }
  else {
    // TODO
  }
}

void triangulate(uint32_t num_vertices, float* polygon, uint32_t** out_size, float** out_triangles) {
  triangulate_ring_no_holes(num_vertices-1, polygon, out_size, out_triangles);

  /*
  float *x0, *y0, *z0, *x1, *y1, *z1 = 0;
  uint32_t s0 = 0;
  get_vec(polygon, 0, &x0, &y0, &z0);
  for(uint32_t i = 1; i < num_vertices; i++) {
    get_vec(polygon, i, &x1, &y1, &z1);
    if (*x0 == *x1 && *y0 == *y1 && *z0 == *z1) {
      // TODO: ring-size: i-s0, ring: polygon+s0*3
      // triangulate_ring_no_holes(i-s0, polygon+s0*3, out_size, out_triangles);

      if (i < num_vertices-4) {
        s0 = ++i;
        get_vec(polygon, i, &x0, &y0, &z0);
      }
    }
  }
  */
}

#endif // TRIANGULATE_IMPLEMENTATION
#endif // TRIANGULATE_H
