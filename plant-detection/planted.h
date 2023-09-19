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

// fast sqrt function to avoid importing math.h duhhh
int int_sqrt(int x) {
  int s, t;
  s = 1;
  t = x;
  while (s < t) {
    s <<= 1;
    t >>= 1;
  }
  do {
    t = s;
    s = (x / s + s) >> 1;
  } while (s < t);
  return t;
}

// fast distance evaluation of plant to plant based on window size
// returns true if the point a close to point b
bool planted_is_same_plant(struct Plant *a, struct Plant *b, int window_size) {
  // int d = int_sqrt(())
  int d_x = a->x - b->x;
  int d_y = a->y - b->y;
  if (d_x < 0)
    d_x = d_x * -1;
  if (d_y < 0)
    d_y = d_y * -1;
  int d = d_x + d_y;
  if (d <= window_size) {
    return true;
  }
  return false;
}

// in place rgb to hsv converter
// !IMPORTANT it changes the underlying rgb values;
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
// sliding window mask for fast iteration
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

// the function name says it all dumbass
void planted_swap(struct Plant *a, struct Plant *b) {
  struct Plant temp = *a;
  *a = *b;
  *b = temp;
}

// compare two plants depending on the distance from the origin
// using manhattan distance
bool planted_cmp(struct Plant *a, struct Plant *b) {
  int d_a = a->x + a->y;
  int d_b = b->x + b->y;
  if (d_a < 0)
    d_a *= -1;
  if (d_b < 0)
    d_b *= -1;
  if (d_a > d_b)
    return true;
  return false;
}

// bubble sort an array of plants
void planted_sort_plants(struct Plant plants[], int len) {
  bool swapped;
  for (int i = 0; i < len - 1; i++) {
    swapped = false;
    for (int j = 0; j < len - i - 1; j++) {
      if (planted_cmp(&plants[j], &plants[j + 1])) {
        planted_swap(&plants[j], &plants[j + 1]);
        swapped = true;
      }
    }
    if (!swapped)
      break;
  }
}

// decrease window_size for maximum accuracy
int planted_get_plants_xy(struct Image *image, struct Plant *plants,
                          int window_size) {
  int total = planted_sliding_window(image, plants, window_size);
  // we sort the plants to make it easier to merge points for the same plant
  planted_sort_plants(plants, total);
  struct Plant temp[total];
  int index = 0;
  int lock = false;
  temp[0].x = plants[0].x;
  temp[0].y = plants[0].y;
  // we average out values that belongs to the same plants in order to reduce
  // the
  for (int i = 0; i < total - 1; i++) {
    if (planted_is_same_plant(&plants[i], &plants[i + 1], window_size)) {
      temp[index].x = (plants[i].x + plants[i + 1].x) / 2;
      temp[index].y = (plants[i].y + plants[i + 1].y) / 2;
    } else {
      index += 1;
      temp[index].x = plants[i].x;
      temp[index].y = plants[i].y;
    }
  }
  for (int i = 0; i < index; i++) {
    plants[i].x = temp[i].x;
    plants[i].y = temp[i].y;
  }
  return index;
}

// full precision mask
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