import os
import sys
import subprocess
import tempfile

if __name__ == '__main__':
    if not os.path.exists('cachesim'):
        print('No cachesim')
        sys.exit()
    trace = ['trace' + str(i) for i in range(6)]
    cache = ['lfu', 'lru', 'lruv', 'test', 'test1', 'test2']
    wrong = 0
    for t in trace:
        for c in cache:
            print('Testing trace: [' + t + '] with cache: [' + c + ']', end='')
            with tempfile.TemporaryFile() as stdout, tempfile.TemporaryFile() as stderr:
                subprocess.call(['valgrind', '--leak-check=full', '-q', './cachesim', 'test/' + c + '_definition', 'test/test_' + t], stdout=stdout, stderr=stderr)
                stdout.seek(0)
                with open('test/test_' + t + '_' + c, 'rb') as std:
                    if std.read() == stdout.read():
                        print(' PASS', end='')
                    else:
                        print(' NG', end='')
                        wrong = 1
                    if stderr.tell():
                        print(' LEAK')
                        wrong = 1
                    else:
                        print(' CLEAN')
    if not wrong:
        print('Test passed!')
