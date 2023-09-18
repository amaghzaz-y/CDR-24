#pragma once
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define max3(a, b, c) max(max(a, b), c)
#define min3(a, b, c) min(min(a, b), c)
#define bool int
#define true 1
#define false 0

struct RGB {
  float r;
  float g;
  float b;
};
struct HSV {
  float h;
  float s;
  float v;
};

struct Image {
  unsigned char *data;
  int width;
  int height;
  int channels;
};

struct Plant {
  int x;
  int y;
};

const struct HSV LOWER = {45, 15, 15};
const struct HSV UPPER = {
    120,
    60,
    45,
};

const struct RGB BLACK = {0, 0, 0};
const struct RGB WHITE = {255, 255, 255};
const float _WHITE = 255;
const float _BLACK = 0;

// fast distance evaluation of plant to plant based on window size
// returns true if the point a close to point b
bool planted_is_same_plant(struct Plant *a, struct Plant *b, int window_size) {
  int d_x = a->x - b->x;
  int d_y = a->y - b->y;
  if (d_x < 0)
    d_x = d_x * -1;
  if (d_y < 0)
    d_y = d_y * -1;
  if (d_x <= window_size && d_y <= window_size) {
    return true;
  }
  return false;
}

void planted_rgb_to_hsv(struct RGB *rgb, struct HSV *hsv) {
  rgb->r = rgb->r / 255;
  rgb->g = rgb->g / 255;
  rgb->b = rgb->b / 255;
  float maxc = max3(rgb->r, rgb->g, rgb->b);
  float minc = min3(rgb->r, rgb->g, rgb->b);
  float hue = 0.0;
  if (maxc == rgb->r) {
    hue = (rgb->g - rgb->b) / (maxc - minc);
  }
  if (maxc == rgb->g) {
    hue = 2.0 + (rgb->b - rgb->r) / (maxc - minc);
  }
  if (maxc == rgb->b) {
    hue = 4.0 + (rgb->r - rgb->g) / (maxc - minc);
  }
  int value = maxc * 100;
  hue = hue * 60;
  if (hue < 0)
    hue = hue + 360;
  hsv->h = hue;
  hsv->s = ((maxc - minc) / maxc) * 100;
  hsv->v = maxc * 100;
  return;
}

// returns true if the desired color is detected
bool planted_apply_mask(struct RGB *rgb, struct HSV *hsv) {
  if (hsv->s > LOWER.s && hsv->s < UPPER.s) {
    if (hsv->v > LOWER.v && hsv->v < UPPER.v) {
      if (hsv->h > LOWER.h && hsv->h < UPPER.h) {
        return true;
      }
    }
  }
  return false;
}

// window_size = 10 yields the best results for 240p image
// return total plants pieces detected
int planted_sliding_window(struct Image *image, struct Plant *plants,
                           int window_size) {
  struct RGB avg = {0, 0, 0};
  struct RGB sum = {0, 0, 0};
  struct HSV hsv = {0, 0, 0};
  plants[0].x = 0;
  plants[0].y = 0;
  int index = 0;
  for (int by = 0; by < image->height; by += window_size) {
    for (int bx = 0; bx < image->width; bx += window_size) {
      sum.r = _BLACK;
      sum.g = _BLACK;
      sum.b = _BLACK;
      for (int y = by; y < by + window_size; y++) {
        for (int x = bx; x < bx + window_size; x++) {
          int i = y * image->width * image->channels + x * image->channels + 0;
          int j = y * image->width * image->channels + x * image->channels + 1;
          int k = y * image->width * image->channels + x * image->channels + 2;
          sum.r += (int)image->data[i];
          sum.g += (int)image->data[j];
          sum.b += (int)image->data[k];
        }
      }
      avg.r = sum.r / (window_size * window_size);
      avg.b = sum.b / (window_size * window_size);
      avg.g = sum.g / (window_size * window_size);
      planted_rgb_to_hsv(&avg, &hsv);
      // color detected
      if (planted_apply_mask(&avg, &hsv)) {
        for (int y = by; y < by + window_size; y++) {
          for (int x = bx; x < bx + window_size; x++) {
            int i =
                y * image->width * image->channels + x * image->channels + 0;
            int j =
                y * image->width * image->channels + x * image->channels + 1;
            int k =
                y * image->width * image->channels + x * image->channels + 2;
            image->data[i] = _WHITE;
            image->data[j] = _BLACK;
            image->data[k] = _BLACK;
          }
        }
        struct Plant plant = {(bx + window_size) / 2, (by + window_size) / 2};
        int last_index = index - 1;
        // index is 0 if index is 0
        if (last_index < 0)
          last_index = 0;

        // calculate the average x,y which is the center of the plant
        if (planted_is_same_plant(&plant, &plants[last_index], window_size)) {
          plants[index].x = (plants[index].x + plants[last_index].x) / 2;
          plants[index].y = (plants[index].y + plants[last_index].y) / 2;
        } else {
          plants[index] = plant;
          index += 1;
        };
      }
    }
  }
  return index;
}

// int planted_get_plants_xy(struct Image *image, struct Plant *plants,
//                           int window_size) {
//   int total = planted_sliding_window(image, plants, window_size);
//   int num = 0;
//   for (int i = 0; i < total; i++) {
//     for (int e = 0; e < total; e++) {
//       if (plants[i].x == plants[e].x && plants[i].y == plants[e].y) {
//         plants[e].x = 0;
//         plants[e].y = 0;
//         continue;
//       }
//       if (plants[e].x == 0 && plants[e].y == 0) {
//         continue;
//       }
//       if (planted_is_same_plant(&plants[i], &plants[e], window_size)) {
//         plants[i].x = (plants[i].x + plants[e].x) / 2;
//         plants[i].y = (plants[i].y + plants[e].y) / 2;
//         num += 1;
//       }
//     }
//   }
//   return num;
// }

void planted_full(struct Image *image) {
  struct RGB rgb;
  struct HSV hsv;
  for (int y = 0; y < image->height; y++) {
    for (int x = 0; x < image->width; x++) {
      int i = y * image->width * image->channels + x * image->channels + 0;
      int j = y * image->width * image->channels + x * image->channels + 1;
      int k = y * image->width * image->channels + x * image->channels + 2;
      rgb.r = image->data[i];
      rgb.g = image->data[j];
      rgb.b = image->data[k];
      planted_rgb_to_hsv(&rgb, &hsv);
      planted_apply_mask(&rgb, &hsv);
      image->data[i] = rgb.r;
      image->data[j] = rgb.g;
      image->data[k] = rgb.b;
    }
  }
  return;
}