from os import listdir
from os.path import isfile, join
from subprocess import call

path = "assets"

objFiles = [f for f in listdir(path) if isfile(join(path, f)) and f.endswith(".obj")]

for f in objFiles:
    print("> " + f)
    name = f[:-4]
    
    cmd = ('"ignore/geometryc.exe" -f "' + path + '/%s.obj" -o "build/assets/%s.mesh" --ccw') % (name, name);
    call(cmd)
    