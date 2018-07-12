from os import listdir
from os.path import isfile, join
from subprocess import call

path = "src"

onlyfiles = [f for f in listdir(path) if isfile(join(path, f))]
scList = []

for f in onlyfiles:
    if f.endswith(".sc") and not f.endswith(".def.sc"):
        scList.append(f)

for f in scList:
    print("> " + f)
    name = f[:-3]
    type = "vertex"
    typeMini = "vs"
    
    if name.startswith("fs"):
        type = "fragment"
        typeMini = "ps"
        
    cmd = 'ignore/shaderc.exe -f "src/%s.sc" -o "build/%s.dx11" --type %s --platform windows -varyingdef "src/varying.def.sc" -p %s_5_0 --debug --O 0' % (name, name, type, typeMini);
    call(cmd)
    