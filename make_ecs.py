from os import listdir
from os.path import isfile, join

print("> Generating EntityComponentSystem files...")

path = "src"

onlyfiles = [f for f in listdir(path) if isfile(join(path, f))]
srcList = []
components = []

for f in onlyfiles:
    if f.endswith(".h"):
        srcList.append(f)

# find //@Component
for filename in srcList:
    f = open(join(path, filename), "r")
    lines = f.readlines()
    f.close()
    for lid, l in enumerate(lines):
        if l == "//@Component\n":
            compLine = lines[lid+1][:-1] # line minus \n
            words = compLine.split(" ")
            if words[0] == "struct":
                print(words[1])
                components.append(words[1])
            else:
                print("Component formating error, file %s (line: %d" % (filename, lid))
                
f_ecsh_in = open("src/ecs.h.in", "r")
f_ecscpp_in = open("src/ecs.cpp.in", "r")

# generate ecs.h
lines = f_ecsh_in.readlines();

for lid, l in enumerate(lines):
    if l == "%%ENUM_COMPONENT%%\n":
        str = ""
        for cid, c in enumerate(components):
            str += "        %s = (u64(1) << %d),\n" % (c[1:], cid)
        lines[lid] = str
        
    elif l == "%%ARRAY_COMPONENT%%\n":
        str = ""
        for cid, c in enumerate(components):
            str += "    ArraySparse<%s> comp_%s;\n" % (c, c[1:])
        lines[lid] = str
        
    elif l == "%%ADD_COMPONENT%%\n":
        str = ""
        for cid, c in enumerate(components):
            str += "    ADD_FUNC(%s)\n" % (c[1:])
        lines[lid] = str
        
    elif l == "%%GETTER_COMPONENT%%\n":
        str = ""
        for cid, c in enumerate(components):
            str += "    GETTER_FUNC(%s)\n" % (c[1:])
        lines[lid] = str
        
ecshContent = "".join(lines)

# generate ecs.cpp
lines = f_ecscpp_in.readlines();

for lid, l in enumerate(lines):
    if l == "%%NAMES_COMPONENT%%\n":
        str = ""
        for cid, c in enumerate(components):
            str += "    \"%s\",\n" % (c[1:])
        lines[lid] = str
        
    elif l == "%%UPDATE_COMPONENT%%\n":
        str = ""
        for cid, c in enumerate(components):
            str += "    UPDATE_FUNC(%s);\n" % (c[1:])
        lines[lid] = str
        
    elif l == "%%ON_DELETE_COMPONENT%%\n":
        str = ""
        for cid, c in enumerate(components):
            str += "    DELETE_FUNC(%s);\n" % (c[1:])
        lines[lid] = str
        
    elif l == "%%REMOVE_COMPONENT%%\n":
        str = ""
        for cid, c in enumerate(components):
            str += "            if(compBits & ComponentBit::%s) {\n" % (c[1:])
            str += "                comp_%s.removeById(i);\n            }\n" % (c[1:])
        lines[lid] = str
        
ecscppContent = "".join(lines)

f_ecsh = open("build/src/ecs.h", "w")
f_ecsh.write(ecshContent)
f_ecsh.close()

f_ecscpp = open("build/src/ecs.cpp", "w")
f_ecscpp.write(ecscppContent)
f_ecscpp.close()

print("> Done.")