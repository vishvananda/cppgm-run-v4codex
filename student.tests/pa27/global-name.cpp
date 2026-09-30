// Reduced global-namespace variable ABI boundary. Compile separately from user.
int g = 7;
int* address_of_g() { return &g; }
