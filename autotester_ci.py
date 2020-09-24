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
from collections import namedtuple, Counter


def formatter(prefix, source_qpl_query_pairs):
    return [(prefix + s, prefix + q) for s, q in source_qpl_query_pairs]


# Edit this to change what source and query file pairs to run.
TEST_OPERATOR_PREFIX = 'TEST-OPERATOR_'
TEST_MODIFIES_NONE_PREFIX = 'TEST-MODIFIES-NONE_'
TEST_USES_NONE_PREFIX = 'TEST-USES-NONE_'
TEST_BASIC_PREFIX = 'TEST-BASIC_'
TEST_NESTED_COND_PREFIX = 'TEST-NESTED-COND_'
TEST_OPERATORS_N_PATTERNS_PREFIX = 'TEST-OPERATORS-N-PATTERNS_'
TEST_PARSER_KEYWORDS_RECOGNITION = 'TEST-PARSER-KEYWORDS-RECOGNITION_'
SIMPLE_SOURCE_QPL_QUERY_PAIRS = (
        [('trivial_source', 'trivial_queries')]
        + formatter(TEST_OPERATOR_PREFIX, [('source', 'queries')])
        + formatter(TEST_USES_NONE_PREFIX, [('source', 'queries')])
        + formatter(TEST_MODIFIES_NONE_PREFIX, [('source', 'queries')])
        + formatter(TEST_BASIC_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_PREFIX, [('source', 'queries')])
        + formatter(TEST_OPERATORS_N_PATTERNS_PREFIX, [('source', 'queries')])
        + formatter(TEST_PARSER_KEYWORDS_RECOGNITION, [('source', 'queries')])
)

# Edit this to change what source and query file pairs should run with failures
SIMPLE_SOURCE_QPL_QUERY_FAILURE_PAIRS = (
    [('failure_bee_movie_script', 'trivial_queries'), ('failure_procedures_with_same_name', 'trivial_queries')]
)

# Named tuple(s)
AutotesterParameters = namedtuple('AutotesterParameters', ['source_filepath', 'query_filepath', 'output_filepath'])
# Dev constants
WINDOWS_OS_NAME = 'nt'
SUCCESS_EXIT_CODE = 0
# Output XML constants
# Whenever a test fails, the output xml will enclose the reason for failure in between `<failed> </failed>` tags.
FAILURE_CLOSING_TAG = '</failed>'
# Whenever a test passes, the output xml will have a `<passed/>` tag.
SUCCESS_TAG = '<passed/>'


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


def execute_autotester(autotester_filepath, autotester_parameters, expect_failure=False):
    command = f'{autotester_filepath} {autotester_parameters.source_filepath} {autotester_parameters.query_filepath} {autotester_parameters.output_filepath}'
    # Replace '/' with '\' as Windows uses '\' as separators between directories and files.
    # e.g. unix/unix_file.txt vs windows\windows_file.txt
    if os.name == WINDOWS_OS_NAME:
        command = command.replace('/', '\\')

    log(f'Executing command: {command}')
    exit_code = os.system(command)
    if expect_failure:
        if exit_code == SUCCESS_EXIT_CODE:
            raise RuntimeError(f'Autotester execution succeeded but failure was expected')
        log(f"Autotester failed successfully with {command} (-: !")
        return

    if exit_code != SUCCESS_EXIT_CODE:
        raise RuntimeError(f'Autotester execution failed with exit code: {exit_code}')
    check_output_xml(autotester_parameters.output_filepath)
    log(f"Autotester passed successfully with {command} (-: !")


def check_output_xml(output_xml):
    with open(output_xml) as f:
        output = f.read()
        # Analyze chunks of the output file.
        text_chunks = Counter(output.split())
        passed_test_cases = text_chunks[SUCCESS_TAG]
        failed_test_cases = text_chunks[FAILURE_CLOSING_TAG]
        # TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/135):
        # Provide more descriptive test summary on test cases failed and reason for failure.
        test_summary = f'Test passed:{passed_test_cases}\nTest failed:{failed_test_cases}'
        if failed_test_cases > 0:
            raise RuntimeError(f'Some test cases failed.\n{test_summary}')
        # TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/134):
        # Check if any of the queries exceed the 5s limit.
        log(test_summary)


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

    autotester_failure_parameters_list = autotester_parameters_to_run_from(
        source_query_pairs=SIMPLE_SOURCE_QPL_QUERY_FAILURE_PAIRS)
    log(f'Autotester expected failure parameters: {autotester_parameters_list}')

    for autotester_parameters in autotester_failure_parameters_list:
        execute_autotester(autotester_filepath=autotester_filepath, autotester_parameters=autotester_parameters,
                           expect_failure=True)


if __name__ == "__main__":
    main()
