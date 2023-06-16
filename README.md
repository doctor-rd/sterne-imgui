# sterne-imgui
[Sterne](https://doctor-rd.github.io/sterne-imgui/)

#### Building
`$ git clone --recurse-submodules -b wasm https://github.com/doctor-rd/sterne-imgui.git`\
`$ cd sterne-imgui`

###### For your local machine
`$ make`\
`$ ./sterne`

###### For Web
`$ make -f Makefile.emscripten`\
`$ cd web`\
`$ python3 -m http.server`
