#! /bin/env python3

import os
import sys

import argparse
import configparser
import logging as log

import pycparser

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

def instrument(cfile, func, wantedfunc):
    log.info("Analyzing %s" % cfile)
    if func not in ERR_CPP:
        ast = pycparser.parse_file(cfile, use_cpp=True, cpp_args=CPP_OPTS)
    else:
        ast = pycparser.parse_file(cfile, use_cpp=True, cpp_args=ERR_CPP_OPTS)

    if wantedfunc != 'y':
        func = wantedfunc
    vf = FunctionParameter(cfile, func)
    vf.visit(ast)
    
def main(config):

    folders = config.sections()
    instr = [("../%s/%s.c" % (folder, file), file, config.get(folder, file)) for folder in folders for file in config.options(folder)]
    
    for cfile, func, wantedfunc in instr:
        if not os.path.isfile(cfile):
            print("%s not found!" % cfile)
            continue
        instrument(cfile, func, wantedfunc)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Libcstapper - systemtap uprobes instrumentation")
    parser.add_argument("-c", "--config", nargs=1, type=str, help="Configuration file with list of functions to instrument")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")
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

    main(config)

