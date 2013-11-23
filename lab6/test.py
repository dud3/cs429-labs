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
    wrong = False
    fast = False
    for t in trace:
        for c in cache:
            with tempfile.TemporaryFile() as stdout, tempfile.TemporaryFile() as stderr:
                args = ['./cachesim', 'test/' + c + '_definition', 'test/test_' + t]
                if len(sys.argv) == 1:
                    args = ['valgrind', '--leak-check=full', '-q'] + args
                elif len(sys.argv) == 2 and sys.argv[1] == 'fast':
                    fast = True
                else:
                    print('Usage: %s [fast]' % sys.argv[0])
                    sys.exit(1)
                print('Testing trace: [' + t + '] with cache: [' + c + ']', end='')
                subprocess.call(args, stdout=stdout, stderr=stderr)
                stdout.seek(0)
                with open('test/test_' + t + '_' + c, 'rb') as std:
                    if std.read() == stdout.read():
                        print(' PASS', end='')
                    else:
                        print(' NG', end='')
                        wrong = True
                    if fast:
                        print(' NO MEMORY CHECK')
                    elif stderr.tell():
                        print(' LEAK')
                        wrong = True
                    else:
                        print(' CLEAN')
    if not wrong:
        print('All test passed!')
