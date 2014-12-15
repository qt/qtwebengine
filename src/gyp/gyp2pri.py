import sys, ast, os

class Gyp(object):
    def __init__(self, fileName):
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

    def addSources(self, sources):
        for source in sources:
            self.addSource(source)

    def addDefine(self, define):
        self.defines.append(define)

    def addDefines(self, defines):
        for macro in defines:
            self.addDefine(macro)

    def addConfig(self, cfg):
        self.config.append(cfg)

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
    if gyp.target(dep)["target_name"] == "javascript":
        continue
    if gyp.target(dep)["target_name"] == "jsapi":
        continue
    t = gyp.target(dep)
    pro.addSources(t["sources"])
    
    if "conditions" in t:
        for condition in t["conditions"]:
            if condition[0] == "OS==\"win\"":
                scope = ProFileSection("win32")
                scope.addSources(condition[1]["sources"])
                pro.addScope(scope)

pro.addDefines(gyp.target_defaults()["defines"])

with open(sys.argv[3], "w") as f:
    f.write(pro.generate())
