# mcbetacpp

c++ port project aimed at minecraft beta 1.7.3 parity.

the codebase is being pushed forward toward beta 1.7.3 from an older beta baseline. beta 1.8 / adventure update and anything later are out of scope.

## building

you need cmake 3.14+ and a working c++ compiler. third-party code is vendored under `external/`, so the normal build is:

```bash
cmake -S . -B build
cmake --build build --config Debug --target McBetaCpp
```

cmake sends runtime output to `bin/`. with the debug command above, the executable ends up at `bin/Debug/McBetaCpp.exe` on the current windows build tree.

## resources

assets live under `resource/` so you can be lazy