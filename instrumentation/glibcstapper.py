#! /bin/env python3

import os
import sys

import argparse
import configparser

import pycparser

class ColorPrint:

    @staticmethod
    def print_fail(message, end = '\n'):
        sys.stderr.write('\x1b[1;31m' + message.strip() + '\x1b[0m' + end)

    @staticmethod
    def print_pass(message, end = '\n'):
        sys.stdout.write('\x1b[1;32m' + message.strip() + '\x1b[0m' + end)

    @staticmethod
    def print_warn(message, end = '\n'):
        sys.stderr.write('\x1b[1;33m' + message.strip() + '\x1b[0m' + end)

    @staticmethod
    def print_info(message, end = '\n'):
        sys.stdout.write('\x1b[1;34m' + message.strip() + '\x1b[0m' + end)

    @staticmethod
    def print_bold(message, end = '\n'):
        sys.stdout.write('\x1b[1;37m' + message.strip() + '\x1b[0m' + end)

CPP_OPTS = [
                "-Ifake_libc_include",
                # "-I..",
                # "-I../include",
                # "-I../sysdeps/generic",
                "-Dlibc_hidden_builtin_def(x)=",
                "-Dlibc_hidden_def(x)=",
                "-Dweak_alias(x, y)=",
                "-D_LIBC=",
           ]

class FunctionParameter(pycparser.c_ast.NodeVisitor):

    def visit_FuncDef(self, node):
        #node.decl.type.args.params
        print (node.decl.name)
        for params in (node.decl.type.args.params):
            print (params.name)

def instrument(cfile, func):
    print ("Analyzing %s" % cfile)
    ast = pycparser.parse_file(cfile, use_cpp=True, cpp_args=CPP_OPTS)

    vf = FunctionParameter()
    vf.visit(ast)
    
# def instrument(cfile, func):
    # print ("Analyzing %s" % cfile)

    # content = None
    # with open(cfile, "r") as f:
        # content = f.readlines()

    # if not content:
        # print ("Cannot open %s" % cfile)
        # return

    # idx = -1
    # for i in range(len(content)):
        # if func+" (" in content[i] or func.upper()+" (" in content[i]:
            # ColorPrint.print_info(content[i])
            # idx = i
            # break
    # if idx == -1:
        # ColorPrint.print_fail("Cannot find the right function. Inspect manually!")

def main(config):

    folders = config.sections()
    instr = [("../%s/%s.c" % (folder, file), file) for folder in folders for file in config.options(folder)]
    
    for cfile, func in instr:
        if not os.path.isfile(cfile):
            print("%s not found!" % cfile)
            continue
        instrument(cfile, func)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Libcstapper - systemtap uprobes instrumentation")
    parser.add_argument("-c", "--config", nargs=1, type=str, help="Configuration file with list of functions to instrument")
    args = parser.parse_args()
    config_file = args.config[0]

    if not config_file:
        parser.print_help()
        exit(1)
    
    config = configparser.ConfigParser()
    config.read(config_file)

    main(config)

