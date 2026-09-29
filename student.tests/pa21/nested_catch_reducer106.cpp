int destroyed;
struct Guard { ~Guard() { ++destroyed; } };

int miss() {
  try {
    Guard guard;
    try { throw 7; } catch (long) { return 2; }
  } catch (int value) { return value != 7 || destroyed != 1; }
  return 3;
}

int active_handler() {
  destroyed = 0;
  try {
    try { throw 1; }
    catch (...) {
      Guard guard;
      try { throw 7L; } catch (int) { return 2; }
    }
  } catch (long value) { return value != 7 || destroyed != 1; }
  return 3;
}

int main() { return miss() || active_handler(); }
