[[noreturn]] void stop() noexcept {__builtin_abort();} int f(){try{stop();}catch(...){}} int main(){}
