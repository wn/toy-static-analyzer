#!/usr/bin/python

# This file is written for Python 3
# It could be made portable and directory agnostic if desired in the future.
# Execute this file in Team24/Test24/ directory.
# Usage: python <this script's filepath> <autotester executable filepath>
# e.g. usage: python ../../autotester_ci.py ../Code24/cmake-build-debug/src/autotester/autotester
# e.g. Mac CI usage: python ../../autotester_ci.py ../../build/src/autotester/autotester
# e.g. Windows CI usage: python ../Code24/autotester_ci.py ../../build/src/autotester/<Debug | Release>/autotester.exe

import os
import sys
from collections import namedtuple

# Edit this to change what source and query file pairs to run.
SIMPLE_SOURCE_QPL_QUERY_PAIRS = [('trivial_source', 'Sample_queries')]
# Named tuple(s)
AutotesterParameters = namedtuple('AutotesterParameters', ['source_filepath', 'query_filepath', 'output_filepath'])
# Dev constants
WINDOWS_OS_NAME = 'nt'
SUCCESS_EXIT_CODE = 0


def log(x):
    """
    Helper function to add more descriptive prefix to print statements for debugging
    """
    return print(f'autotester_ci: {x}')


def autotester_parameters_to_run_from(source_query_pairs):
    return [
        AutotesterParameters(source_filepath=f'./{source_filename}.txt', query_filepath=f'./{query_filename}.txt',
                             output_filepath=f'./output_{source_filename}_{query_filename}.xml') for
        source_filename, query_filename in
        source_query_pairs]


def execute_autotester(autotester_filepath, autotester_parameters):
    command = f'{autotester_filepath} {autotester_parameters.source_filepath} {autotester_parameters.query_filepath} {autotester_parameters.output_filepath}'
    # Replace '/' with '\' as Windows uses '\' as separators between directories and files.
    # e.g. unix/unix_file.txt vs windows\windows_file.txt
    if os.name == WINDOWS_OS_NAME:
        command = command.replace('/', '\\')

    log(f'Executing command: {command}')
    exit_code = os.system(command)
    if exit_code != SUCCESS_EXIT_CODE:
        raise RuntimeError(f'Autotester execution failed with exit code: {exit_code}')
    # TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/70):
    # Read and check output of xml file here


def main():
    args = sys.argv[1:]
    if len(args) != 1:
        raise AssertionError(
            'Only the one argument after the script is expected. E.g. python3 <script_name> <autotester_file_path>')

    autotester_filepath = args[0]
    log(f'Received autotester filepath: {autotester_filepath}')

    autotester_parameters_list = autotester_parameters_to_run_from(source_query_pairs=SIMPLE_SOURCE_QPL_QUERY_PAIRS)
    log(f'Autotester parameters: {autotester_parameters_list}')

    for autotester_parameters in autotester_parameters_list:
        execute_autotester(autotester_filepath=autotester_filepath, autotester_parameters=autotester_parameters)


if __name__ == "__main__":
    main()
