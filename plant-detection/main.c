#include "planted.h"
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stdio.h>
#include <time.h>

int test() {
  const char *files[4] = {"../test/1.jpg", "../test/2.jpg", "../test/3.jpg",
                          "../test/4.jpg"};
  struct Image image;
  for (int i = 0; i < 4; i++) {
    image.data =
        stbi_load(files[i], &image.width, &image.height, &image.channels, 3);
    if (image.data == NULL) {
      printf("couldnt load image \n");
      exit(0);
    }
    clock_t start_time = clock();
    struct Plant plants[10000];
    int total = planted_get_plants_xy(&image, plants, 10);
    planted_set_crosshair(&image, plants, total, 4);
    clock_t end_time = clock();
    double elapsed_time =
        (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000;
    printf("elapsed time: %.1f ms - %d.png\n", elapsed_time, i);
    char str[80];
    sprintf(str, "../test/mask-%d.png", i);
    int success = stbi_write_png(str, image.width, image.height, image.channels,
                                 image.data, 0);
    stbi_image_free(image.data);
  }
  return 0;
}

int main() { test(); }