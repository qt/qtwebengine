import sys, ast, os, argparse

class Gyp(object):
    def __init__(self, fileName, gypVariables):
        self.fileName = fileName
        self.gypVariables = gypVariables
        with open(fileName, "r") as f:
            self.variables = ast.literal_eval(f.read())

    def target(self, name):
        for t in self.variables["targets"]:
            if t["target_name"] == name:
                return t;

        for condition in self.variables["conditions"] or []:
            check = condition[0]
            vars = condition[1]
            try:
                if eval(check, None, self.gypVariables):
                    for t in vars["targets"] or []:
                        if t["target_name"] == name:
                            return t;
            except: pass
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
        path = path.replace("<(DEPTH)", "")
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

def addDependencies(gyp, proFile, gypTarget, variables):
    for dep in gypTarget.get("dependencies") or []:
        target = None
        baseDir = None
        for key, value in variables.iteritems():
            name = "<(" + key + ")"
            replacement = value
            dep = dep.replace(name, replacement)
        if ".gyp:" in dep:
            targetFileName = dep[:dep.index(":")]
            fileName = os.path.dirname(gyp.fileName) + "/" + targetFileName
            subDep = dep[dep.index(":") + 1:]
            target = Gyp(fileName, variables).target(subDep)
            baseDir = os.path.relpath(os.path.dirname(fileName), os.path.dirname(gyp.fileName))
        else:
            target = gyp.target(dep)
        if not target:
            return
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

        addDependencies(gyp, proFile, target, variables)

optionParser = argparse.ArgumentParser()
optionParser.add_argument("--gyp-var", action="append")
optionParser.add_argument("input")
optionParser.add_argument("mainTarget")
optionParser.add_argument("output")
config = optionParser.parse_args()

variables = {}
for var in config.gyp_var or []:
    key, value = var.split("=")
    variables[key] = value

gyp = Gyp(config.input, variables)

mainTarget = gyp.target(config.mainTarget)

pro = ProFile()

print(mainTarget)
pro.addSources(mainTarget["sources"])

addDependencies(gyp, pro, mainTarget, variables)

target_defaults = gyp.target_defaults()
pro.addDefines(target_defaults["defines"])
if "include_dirs" in target_defaults:
    for path in target_defaults["include_dirs"]:
        pro.addInclude(os.path.relpath(os.path.dirname(gyp.fileName) + "/" + path, os.path.dirname(config.output)))

with open(config.output, "w") as f:
    f.write(pro.generate())
