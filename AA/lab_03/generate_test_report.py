import subprocess
import json
import datetime
import os
import xml.etree.ElementTree as ET


def compile_and_run_tests():
    base_flags = ['-std=c++17', '-Wall', '-Wextra']
    include_dirs = ['-I./code', '-I./unit_tests']

    result = subprocess.run(
        ['g++'] + base_flags + include_dirs + [
            '-c', './code/algorithms.cpp', '-o', 'algorithms.o'
        ],
        capture_output=True, text=True
    )

    if result.returncode != 0:
        print(f"Error compiling algorithms.cpp: {result.stderr}")
        return None, 1

    result = subprocess.run(
        ['g++'] + base_flags + include_dirs + [
            '-c', './unit_tests/tests.cpp', '-o', 'tests.o'
        ],
        capture_output=True, text=True
    )

    if result.returncode != 0:
        print(f"Error compiling tests.cpp: {result.stderr}")
        return None, 1

    result = subprocess.run([
        'g++', '-o', 'test_runner',
        'algorithms.o', 'tests.o',
        '-lgtest', '-lgtest_main', '-lpthread'
    ], capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Linking error: {result.stderr}")
        return None, 1

    result = subprocess.run(
        ['./test_runner', '--gtest_output=xml:test_results.xml'],
        capture_output=True, text=True
    )

    return result.stdout, result.returncode


def parse_test_results():
    try:
        tree = ET.parse('test_results.xml')
        root = tree.getroot()

        tests = int(root.attrib.get('tests', 0))
        failures = int(root.attrib.get('failures', 0))
        passed = tests - failures

        return passed, failures
    except (ET.ParseError, FileNotFoundError) as e:
        print(f"Error parsing test results: {e}")
        return 0, 12


def main():
    os.makedirs('ready', exist_ok=True)

    output, return_code = compile_and_run_tests()

    if output is None:
        passed, failed = 0, 12
    else:
        passed, failed = parse_test_results()

    now = datetime.datetime.now(datetime.timezone.utc).astimezone()
    timestamp = now.strftime("%Y-%m-%dT%H:%M:%S:%z")

    total_tests = passed + failed
    coverage = passed / total_tests if total_tests > 0 else 0

    report = {
        "timestamp": timestamp,
        "coverage": round(coverage, 2),
        "passed": passed,
        "failed": failed
    }

    with open('ready/stud-unit-test-report.json', 'w') as f:
        json.dump(report, f, indent=4)

    for file in ['test_runner', 'algorithms.o', 'tests.o', 'test_results.xml']:
        if os.path.exists(file):
            os.remove(file)

    print(f"Test report generated: {passed} passed, {failed} failed")
    print(f"Coverage: {coverage:.2%}")


if __name__ == '__main__':
    main()
