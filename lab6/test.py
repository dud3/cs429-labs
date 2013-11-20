import os
import sys
import subprocess

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
            output = None
            err = None
            print(subprocess.call(['valgrind', './cachesim', 'test/' + c + '_definition', 'test/test_' + t], stdout=output, stderr=err))
            # answer = os.popen('./cachesim test/' + c + '_definition test/test_' + t)
            # output = answer.readlines()
            std = open('test/test_' + t + '_' + c).readlines()
            if output != std:
                print(' NG')
                wrong = 1
            else:
                print(' Pass')
    if not wrong:
        print('Test passed!')

