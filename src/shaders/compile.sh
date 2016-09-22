glslangValidator -V src/shaders/shader.vert -o src/shaders/vert.spv
glslangValidator -V src/shaders/shader.tesc -o src/shaders/tesc.spv
glslangValidator -V src/shaders/shader.tese -o src/shaders/tese.spv
#glslangValidator -V src/shaders/shader.geom -o src/shaders/geom.spv
glslangValidator -V src/shaders/shader.frag -o src/shaders/frag.spv
glslangValidator -V src/shaders/shader.comp -o src/shaders/comp.spv

xxd -i src/shaders/vert.spv > include/quavis/shaders.h
xxd -i src/shaders/tese.spv >> include/quavis/shaders.h
xxd -i src/shaders/tesc.spv >> include/quavis/shaders.h
#xxd -i src/shaders/geom.spv >> include/quavis/shaders.h
xxd -i src/shaders/frag.spv >> include/quavis/shaders.h
xxd -i src/shaders/comp.spv >> include/quavis/shaders.h
