#! /bin/env python3

import os
import sys

import argparse
import configparser
import logging as log

import pycparser

MARKER = "/* STAPPED */\n"
STAP_HEADER = "#include <stap-probe.h>\n"

CPP_OPTS = [
                "-Ifake_libc_include",
                # "-I..",
                # "-I../include",
                # "-I../sysdeps/generic",
                "-D__attribute__(x)=",
                "-Dlibc_hidden_builtin_def(x)=",
                "-Dlibc_hidden_def(x)=",
                "-Dweak_alias(x, y)=",
                "-Dstrong_alias(x, y)=",
                "-Dinhibit_loop_to_libcall=",
                "-Dlibc_hidden_weak(x)=",
                "-D__THROW="
           ]

ERR_CPP = ["strdup"]
ERR_CPP_OPTS = CPP_OPTS + ["-D_LIBC"]

CPROTOS = {}
class CProto(object):
    def __init__(self, file, func, realfunc, args):
        self.file = file
        self.func = func
        self.realfunc = realfunc
        self.args = args
    def __str__(self):
        return "File: %s, function: %s, args: %s" % (self.file, self.realfunc, self.args)

class FunctionParameter(pycparser.c_ast.NodeVisitor):
    def __init__(self, file, func):
        self.file = file
        self.func = func
    
    def visit_FuncDef(self, node):
        name = node.decl.name
        if self.func[::-1] != name[::-1][:len(self.func)]:
            log.warning("Discarding function: %s in %s - Add it to config file if needed" % (name, self.file))
            return
        args = [ params.name for params in (node.decl.type.args.params)]
        CPROTOS[self.func] = CProto(self.file, self.func, name, args)
        log.info(CPROTOS[self.func])

def extract_cprotos(cfile, func):
    log.info("Analyzing %s" % cfile)
    if func not in ERR_CPP:
        ast = pycparser.parse_file(cfile, use_cpp=True, cpp_args=CPP_OPTS)
    else:
        ast = pycparser.parse_file(cfile, use_cpp=True, cpp_args=ERR_CPP_OPTS)

    vf = FunctionParameter(cfile, func)
    vf.visit(ast)
    
def add_probe(file, cproto):
    log.info("Instrumenting %s in %s" % (cproto.realfunc, file))

    fcontent = [ line for line in open(file, "r")]
    if fcontent[0] == MARKER:
        log.warning("File %s already instrumented" % file)
        return

    fcontent.insert(0, MARKER)
    fcontent.insert(1, STAP_HEADER)

    probe = "LIBC_PROBE( %s, %d" % (cproto.realfunc, len(cproto.args))
    for arg in cproto.args:
        probe += ", %s" % arg
    probe += " )\n"

    for i in range(len(fcontent)-1):
        if cproto.realfunc + " (" in fcontent[i].lower() and fcontent[i+1].strip() == "{":
            fcontent.insert(i+2, probe)
            break

    sys.stdout.writelines(fcontent)
    exit(1)

def del_probe(file, cproto):
    log.info("Cleaning %s in %s" % (cproto.realfunc, file))
    
    fcontent = [ line for line in open(file, "r")]
    if fcontent[0] != MARKER:
        log.warning("File %s not instrumented" % file)
        return
    
    del fcontent[0]
    if fcontent[0] == STAP_HEADER:
        del fcontent[0]

    for i in range(len(fcontent)):
        if "LIBC_PROBE" in fcontent[i]:
            del fcontent[i]
            break
    
    sys.stdout.writelines(fcontent)
    exit(1)

ADD_PROBE = 1
DEL_PROBE = 2

def main(config, action):

    folders = config.sections()
    instr = [("../%s/%s.c" % (folder, file), file, config.get(folder, file)) for folder in folders for file in config.options(folder)]
    
    for cfile, func, wantedfunc in instr:
        if not os.path.isfile(cfile):
            print("%s not found!" % cfile)
            continue
        if wantedfunc != "y":
            func = wantedfunc
        extract_cprotos(cfile, func)
        if action == ADD_PROBE:
            add_probe(cfile, CPROTOS[func])
        elif action == DEL_PROBE:
            del_probe(cfile, CPROTOS[func])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Libcstapper - systemtap uprobes instrumentation")
    parser.add_argument("-c", "--config", nargs=1, type=str, help="Configuration file with list of functions to instrument")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")
    parser.add_argument("-r", "--remove", action="store_true", help="Remove instrumentation")
    args = parser.parse_args()
    
    if args.verbose:
        log.basicConfig(format="%(levelname)s: %(message)s", level=log.DEBUG)
    else:
        log.basicConfig(format="%(levelname)s: %(message)s")
    
    config_file = args.config[0]
    log.info("Configuration file: %s" % config_file)

    if not config_file:
        parser.print_help()
        exit(1)
    
    config = configparser.ConfigParser()
    config.read(config_file)

    if args.remove:
        main(config, DEL_PROBE)
    else:
        main(config, ADD_PROBE)


