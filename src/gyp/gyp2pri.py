import sys, ast, os

class Gyp(object):
    def __init__(self, fileName):
        self.fileName = fileName
        with open(fileName, "r") as f:
            self.variables = ast.literal_eval(f.read())

    def target(self, name):
        for t in self.variables["targets"]:
            if t["target_name"] == name:
                return t;
        return None

    def target_defaults(self):
        return self.variables["target_defaults"]

class ProFileSection(object):
    sourceExtensions = [ ".cpp", ".cc", ".c" ]
    headerExtensions = [ ".h", ".hh" ]
    skippingExtensions = [ ".rc" ]
    skippingFiles = [ "makefile" ]

    def __init__(self, scope):
        self.sources = []
        self.headers = []
        self.defines = []
        self.includes = []
        self.config = []
        self.scope = scope

    def addSource(self, fileName):
        extension = os.path.splitext(fileName)[1]
        baseName = os.path.basename(fileName)
        if extension in ProFile.headerExtensions:
            self.headers.append(fileName)
        elif extension in ProFile.sourceExtensions:
            self.sources.append(fileName)
        elif baseName in ProFile.skippingFiles:
            return
        elif extension in ProFile.skippingExtensions:
            return
        else:
            raise Exception("Unknown source %s" % fileName)

    def addSources(self, sources, baseDirectory = None):
        for source in sources:
            path = source
            if baseDirectory:
                path = baseDirectory + "/" + path
            self.addSource(path)

    def addDefine(self, define):
        self.defines.append(define)

    def addDefines(self, defines):
        for macro in defines:
            self.addDefine(macro)

    def addConfig(self, cfg):
        self.config.append(cfg)

    def addInclude(self, path):
        self.includes.append("$$PWD/" + path)

    def generate(self):
        result = ""
        if self.defines:
            result += "DEFINES += \\\n    "
            result += " \\\n    ".join(self.defines)
            result += "\n\n"
        if self.config:
            result += "CONFIG += \\\n    "
            result += " \\\n    ".join(self.config)
            result += "\n\n"
        if self.includes:
            result += "INCLUDEPATH += \\\n    "
            result += " \\\n    ".join(self.includes)
            result += "\n\n"
        result += "SOURCES += \\\n    "
        result += " \\\n    ".join(self.sources)
        result += "\n\n"
        result += "HEADERS += \\\n    "
        result += " \\\n    ".join(self.headers)
        result += "\n\n"
        return result

class ProFile(ProFileSection):
    def __init__(self):
        ProFileSection.__init__(self, "")
        self.scopes = []

    def addScope(self, section):
        self.scopes.append(section)

    def generate(self):
        result = "# This is a generated file, do not edit!\n"
        result += ProFileSection.generate(self)
        for section in self.scopes:
            result += section.scope + " {\n"
            result += section.generate()
            result += "\n}\n"
        return result

gyp = Gyp(sys.argv[1])

mainTarget = gyp.target(sys.argv[2])

pro = ProFile()

pro.addSources(mainTarget["sources"])

for dep in mainTarget["dependencies"]:
    target = None
    baseDir = None
    if ".gyp:" in dep:
        fileName = os.path.dirname(gyp.fileName) + "/" + dep[:dep.index(":")]
        subDep = dep[dep.index(":") + 1:]
        target = Gyp(fileName).target(subDep)
        baseDir = os.path.relpath(os.path.dirname(fileName), os.path.dirname(gyp.fileName))
    else:
        target = gyp.target(dep)
    if target["target_name"] == "javascript":
        continue
    if target["target_name"] == "jsapi":
        continue
    pro.addSources(target["sources"], baseDir)
    
    if "conditions" in target:
        for condition in target["conditions"]:
            if condition[0] == "OS==\"win\"":
                scope = ProFileSection("win32")
                scope.addSources(condition[1]["sources"])
                pro.addScope(scope)

target_defaults = gyp.target_defaults()
pro.addDefines(target_defaults["defines"])
if "include_dirs" in target_defaults:
    for path in target_defaults["include_dirs"]:
        pro.addInclude(os.path.relpath(os.path.dirname(gyp.fileName) + "/" + path, os.path.dirname(sys.argv[3])))

with open(sys.argv[3], "w") as f:
    f.write(pro.generate())
