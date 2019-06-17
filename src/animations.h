#pragma once

typedef struct animation {
  const char* name;
  unsigned (*animation)();
} animation_t;

extern animation_t animations[];

extern animation_t *current_animation;

animation_t *animation_lookup(const char*);
void animation_set(animation_t*);
bool animation_set(const char*);
void animation_speed(int);
void animation_start();
void animation_stop();
