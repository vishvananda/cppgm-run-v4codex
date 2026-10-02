// VALIDATION: compile-pass
// N3485: [temp.inst]/1,5; [conv]/4.
template<class T> struct Handle {
    T value;
    explicit operator bool() const { return value != 0; }
};

// The reference does not instantiate Handle<int>; conversion lookup must.
bool present(const Handle<int>& handle)
{
    if (handle) return true;
    return false;
}

int main()
{
    Handle<int> empty{0}, full{7};
    return present(empty) || !present(full);
}
