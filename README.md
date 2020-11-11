
## How to build
```
git clone https://github.com/LoopTactics/TacticsDSL.git tacticsdsl
cd tacticsdsl
git submodule update --init --recursive 
cmake ./
make -j4
make check
```

## Deps
- llvm-9 (```apt-get install llvm-9-dev```)
- cmake >= 3.16
- gcc/++

