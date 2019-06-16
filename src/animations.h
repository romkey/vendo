#pragma once

typedef struct animation {
  const char* name;
  unsigned (*animation)();
} animation_t;

extern animation_t animations[];

extern animation_t *current_animation;

void animation_set(animation_t *);
animation_t *animation_lookup(const char*);
void animation_speed(int);
void animation_start();
void animation_stop();
