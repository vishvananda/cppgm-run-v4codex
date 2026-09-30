template<int Bias> struct Mixer {
    long value;
    long combine(double first, long integer, double second) {
        return value + (long)first + integer + (long)second + Bias;
    }
    long dormant() { return missing(*this); }
};
int main(int argc, char**) {
    Mixer<7> mixer;
    mixer.value = argc;
    long result = mixer.combine(1.5, 20, 2.5);
    return result != argc + 30;
}
