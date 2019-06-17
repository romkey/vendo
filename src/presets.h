typedef struct preset {
  const char* name;
  void (*preset)();  } preset_t;

extern preset_t presets[];

void preset_set(preset_t*);
bool preset_set(const char*);
