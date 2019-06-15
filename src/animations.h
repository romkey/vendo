extern struct animation {
  const char* name;
  unsigned (*animation)();
} animations[];

extern struct animation *current_animation;
