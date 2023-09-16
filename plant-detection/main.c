#include "planted.h"
#include <math.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stdio.h>

int main() {
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
    planted_detect(&image);
    char str[80];
    sprintf(str, "../test/mask-%d.png", i);
    int success = stbi_write_png(str, image.width, image.height, image.channels,
                                 image.data, 0);
    stbi_image_free(image.data);
  }

  return 0;
}
