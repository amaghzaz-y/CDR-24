#include "planted.h"
#include <math.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stdio.h>

int main() {
  const char *filename = "../test/4.jpg";
  struct Image image;
  image.data =
      stbi_load(filename, &image.width, &image.height, &image.channels, 3);
  if (image.data == NULL) {
    printf("couldnt load image \n");
    exit(0);
  }
  planted_detect(&image);
  int success = stbi_write_png("../output.png", image.width, image.height,
                               image.channels, image.data, 0);
  stbi_image_free(image.data);
  return 0;
}
