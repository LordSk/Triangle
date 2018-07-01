@echo off

REM use vs_4_0+ for dx11
echo test_shader: start...
"ignore/shaderc.exe" -f "src/vs_test.sc" -o "build/vs_test.dx11" --type vertex --platform windows -varyingdef "src/varying.def.sc" -p vs_5_0 --debug --O 0
"ignore/shaderc.exe" -f "src/fs_test.sc" -o "build/fs_test.dx11" --type fragment --platform windows -varyingdef "src/varying.def.sc" -p ps_5_0 --debug --O 0
"ignore/shaderc.exe" -f "src/vs_test.sc" -o "build/vs_test.glsl" --type vertex --platform linux -varyingdef "src/varying.def.sc"
"ignore/shaderc.exe" -f "src/fs_test.sc" -o "build/fs_test.glsl" --type fragment --platform linux -varyingdef "src/varying.def.sc"
echo test_shader: done.

echo vertex_shading: start...
"ignore/shaderc.exe" -f "src/vs_vertex_shading.sc" -o "build/vs_vertex_shading.dx11" --type vertex --platform windows -varyingdef "src/varying.def.sc" -p vs_5_0 --debug --O 0
"ignore/shaderc.exe" -f "src/fs_vertex_shading.sc" -o "build/fs_vertex_shading.dx11" --type fragment --platform windows -varyingdef "src/varying.def.sc" -p ps_5_0 --debug --O 0
"ignore/shaderc.exe" -f "src/vs_vertex_shading.sc" -o "build/vs_vertex_shading.glsl" --type vertex --platform linux -varyingdef "src/varying.def.sc"
"ignore/shaderc.exe" -f "src/fs_vertex_shading.sc" -o "build/fs_vertex_shading.glsl" --type fragment --platform linux -varyingdef "src/varying.def.sc"
echo vertex_shading: done.

echo vertex_shading_color: start...
"ignore/shaderc.exe" -f "src/vs_vertex_shading.sc" -o "build/vs_vertex_shading.dx11" --type vertex --platform windows -varyingdef "src/varying.def.sc" -p vs_5_0 --debug --O 0
"ignore/shaderc.exe" -f "src/fs_vertex_shading_color.sc" -o "build/fs_vertex_shading_color.dx11" --type fragment --platform windows -varyingdef "src/varying.def.sc" -p ps_5_0 --debug --O 0
"ignore/shaderc.exe" -f "src/vs_vertex_shading.sc" -o "build/vs_vertex_shading.glsl" --type vertex --platform linux -varyingdef "src/varying.def.sc"
"ignore/shaderc.exe" -f "src/fs_vertex_shading_color.sc" -o "build/fs_vertex_shading_color.glsl" --type fragment --platform linux -varyingdef "src/varying.def.sc"
echo vertex_shading_color: done.

echo dbg_color: start...
"ignore/shaderc.exe" -f "src/vs_dbg_color.sc" -o "build/vs_dbg_color.dx11" --type vertex --platform windows -varyingdef "src/varying.def.sc" -p vs_5_0 --debug --O 0
"ignore/shaderc.exe" -f "src/fs_dbg_color.sc" -o "build/fs_dbg_color.dx11" --type fragment --platform windows -varyingdef "src/varying.def.sc" -p ps_5_0 --debug --O 0
"ignore/shaderc.exe" -f "src/vs_dbg_color.sc" -o "build/vs_dbg_color.glsl" --type vertex --platform linux -varyingdef "src/varying.def.sc"
"ignore/shaderc.exe" -f "src/fs_dbg_color.sc" -o "build/fs_dbg_color.glsl" --type fragment --platform linux -varyingdef "src/varying.def.sc"
echo dbg_color: done.