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
TEST_OPERATOR_PREFIX = 'TEST_OPERATOR_'
TEST_MODIFIES_NONE_PREFIX = 'TEST_MODIFIES_NONE_'
TEST_USES_NONE_PREFIX = 'TEST_USES_NONE_'
TEST_BASIC_PREFIX = 'TEST_BASIC_'
TEST_PARSER_KEYWORDS_RECOGNITION_PREFIX = 'TEST_PARSER_KEYWORDS_RECOGNITION_'

TEST_NESTED_COND_BASIC_PREFIX = 'TEST_NESTED_COND_BASIC_'
TEST_NESTED_COND_PARENT_PARENTT_INVALID_PREFIX = 'TEST_NESTED_COND_PARENT_PARENTT_INVALID_'
TEST_NESTED_COND_PARENT_VALID_PREFIX = 'TEST_NESTED_COND_PARENT_VALID_'
TEST_NESTED_COND_PARENTT_VALID_PREFIX = 'TEST_NESTED_COND_PARENTT_VALID_'

TEST_NESTED_COND_FOLLOWS_FOLLOWST_INVALID_PREFIX = 'TEST_NESTED_COND_FOLLOWS_FOLLOWST_INVALID_'
TEST_NESTED_COND_FOLLOWS_VALID_PREFIX = 'TEST_NESTED_COND_FOLLOWS_VALID_'
TEST_NESTED_COND_FOLLOWST_VALID_PREFIX = 'TEST_NESTED_COND_FOLLOWST_VALID_'

TEST_OPERATORS_N_PATTERNS_BASIC_PREFIX = 'TEST_OPERATORS_N_PATTERNS_BASIC_'
TEST_OPERATORS_N_PATTERNS_PATTERN_ASSIGN_INVALID_PREFIX = 'TEST_OPERATORS_N_PATTERNS_PATTERN_ASSIGN_INVALID_'
TEST_OPERATORS_N_PATTERNS_PATTERN_ASSIGN_VALID_PREFIX = 'TEST_OPERATORS_N_PATTERNS_PATTERN_ASSIGN_VALID_'

TEST_OPERATORS_N_PATTERNS_USES_INVALID_PREFIX = 'TEST_OPERATORS_N_PATTERNS_USES_INVALID_'
TEST_OPERATORS_N_PATTERNS_USES_VALID_PREFIX = 'TEST_OPERATORS_N_PATTERNS_USES_VALID_'
TEST_OPERATORS_N_PATTERNS_MODIFIES_INVALID_PREFIX = 'TEST_OPERATORS_N_PATTERNS_MODIFIES_INVALID_'
TEST_OPERATORS_N_PATTERNS_MODIFIES_VALID_PREFIX = 'TEST_OPERATORS_N_PATTERNS_MODIFIES_VALID_'

TEST_NESTED_COND_NEXT_VALID_PREFIX = 'TEST_NESTED_COND_NEXT_VALID_'
TEST_NESTED_COND_NEXT_INVALID_PREFIX = 'TEST_NESTED_COND_NEXT_INVALID_'
TEST_NESTED_COND_NEXTT_VALID_PREFIX = 'TEST_NESTED_COND_NEXTT_VALID_'
TEST_NESTED_COND_NEXTT_INVALID_PREFIX = 'TEST_NESTED_COND_NEXTT_INVALID_'

TEST_TWO_CLAUSE_FOLLOWS_VALID_PREFIX = "TEST_TWO_CLAUSE_FOLLOWS_VALID_"
TEST_TWO_CLAUSE_FOLLOWST_VALID_PREFIX = "TEST_TWO_CLAUSE_FOLLOWST_VALID_"
TEST_TWO_CLAUSE_PARENT_VALID_PREFIX = "TEST_TWO_CLAUSE_PARENT_VALID_"
TEST_TWO_CLAUSE_PARENTT_VALID_PREFIX = "TEST_TWO_CLAUSE_PARENTT_VALID_"
TEST_TWO_CLAUSE_USES_VALID_PREFIX = "TEST_TWO_CLAUSE_USES_VALID_"
TEST_TWO_CLAUSE_MODIFIES_VALID_PREFIX = "TEST_TWO_CLAUSE_MODIFIES_VALID_"
TEST_TWO_CLAUSE_NEXT_VALID_PREFIX = "TEST_TWO_CLAUSE_NEXT_VALID_"
TEST_TWO_CLAUSE_NEXTT_VALID_PREFIX = "TEST_TWO_CLAUSE_NEXTT_VALID_"
TEST_TWO_CLAUSE_BOOLEAN_VALID_PREFIX = "TEST_TWO_CLAUSE_BOOLEAN_VALID_"
TEST_SINGLE_CLAUSE_BOOLEAN_VALID_PREFIX = "TEST_SINGLE_CLAUSE_BOOLEAN_VALID_"

TEST_CALLS_TWO_PROC_VALID_PREFIX = "TEST_CALLS_TWO_PROC_VALID_"
TEST_CALLS_TWO_PROC_INVALID_PREFIX = "TEST_CALLS_TWO_PROC_INVALID_"
TEST_CALLST_TWO_PROC_VALID_PREFIX = "TEST_CALLST_TWO_PROC_VALID_"
TEST_CALLST_TWO_PROC_INVALID_PREFIX = "TEST_CALLST_TWO_PROC_INVALID_"
TEST_CALLS_CALLST_LINEAR_VALID_PREFIX = "TEST_CALLS_CALLST_LINEAR_VALID_"
TEST_CALLS_CALLST_DAG_VALID_PREFIX = "TEST_CALLS_CALLST_DAG_VALID_"

TEST_AFFECTS_CODE5_PREFIX = "TEST_AFFECTS_CODE5_"
TEST_AFFECTS_SIMPLE_VALID_PREFIX = "TEST_AFFECTS_SIMPLE_VALID_"
TEST_AFFECTS_SIMPLE_INVALID_PREFIX = "TEST_AFFECTST_SIMPLE_INVALID_"
TEST_AFFECTST_SIMPLE_VALID_PREFIX = "TEST_AFFECTST_SIMPLE_VALID_"
TEST_AFFECTST_SIMPLE_INVALID_PREFIX = "TEST_AFFECTST_SIMPLE_INVALID_"
TEST_AFFECTS_NESTED_COND_PREFIX = "TEST_AFFECTS_NESTED_COND_"

TEST_TUPLES_SINGLE_CLAUSE_VALID_PREFIX = "TEST_TUPLES_SINGLE_CLAUSE_VALID_"
TEST_TUPLES_MULTI_CLAUSE_SIMPLE_VALID_PREFIX = "TEST_TUPLES_MULTI_CLAUSE_SIMPLE_VALID_"


SIMPLE_SOURCE_QPL_QUERY_PAIRS = (
        [('trivial_source', 'trivial_queries')]
        + formatter(TEST_OPERATOR_PREFIX, [('source', 'queries')])
        + formatter(TEST_USES_NONE_PREFIX, [('source', 'queries')])
        + formatter(TEST_MODIFIES_NONE_PREFIX, [('source', 'queries')])
        + formatter(TEST_BASIC_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_BASIC_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_PARENT_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_PARENTT_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_PARENT_PARENTT_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_FOLLOWS_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_FOLLOWST_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_FOLLOWS_FOLLOWST_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_OPERATORS_N_PATTERNS_BASIC_PREFIX, [('source', 'queries')])
        + formatter(TEST_OPERATORS_N_PATTERNS_PATTERN_ASSIGN_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_OPERATORS_N_PATTERNS_PATTERN_ASSIGN_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_OPERATORS_N_PATTERNS_USES_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_OPERATORS_N_PATTERNS_USES_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_OPERATORS_N_PATTERNS_MODIFIES_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_OPERATORS_N_PATTERNS_MODIFIES_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_PARSER_KEYWORDS_RECOGNITION_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_NEXT_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_NEXT_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_NEXTT_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_NESTED_COND_NEXTT_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TWO_CLAUSE_FOLLOWS_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TWO_CLAUSE_FOLLOWST_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TWO_CLAUSE_PARENT_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TWO_CLAUSE_PARENTT_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TWO_CLAUSE_USES_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TWO_CLAUSE_MODIFIES_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TWO_CLAUSE_NEXT_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TWO_CLAUSE_NEXTT_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TWO_CLAUSE_BOOLEAN_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_SINGLE_CLAUSE_BOOLEAN_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_CALLS_TWO_PROC_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_CALLS_TWO_PROC_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_CALLST_TWO_PROC_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_CALLST_TWO_PROC_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_CALLS_CALLST_LINEAR_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_CALLS_CALLST_DAG_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_AFFECTS_CODE5_PREFIX, [('source', 'queries')])
        + formatter(TEST_AFFECTS_SIMPLE_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_AFFECTS_SIMPLE_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_AFFECTST_SIMPLE_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_AFFECTST_SIMPLE_INVALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_AFFECTS_NESTED_COND_PREFIX, [('source', 'queries')])
        + formatter(TEST_TUPLES_SINGLE_CLAUSE_VALID_PREFIX, [('source', 'queries')])
        + formatter(TEST_TUPLES_MULTI_CLAUSE_SIMPLE_VALID_PREFIX, [('source', 'queries')])
)

SIMPLE_SOURCE_QPL_QUERY_FAILURE_PAIRS = (
        formatter('failure_bee_movie_script', [('source', 'queries')])
        + formatter('failure_procedures_with_same_name', [('source', 'queries')])
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
