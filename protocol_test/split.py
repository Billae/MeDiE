#! /bin/env python
#

from argparse import ArgumentParser, FileType
from collections import defaultdict

HEADERS = ("operation", "key", "jobid")

def partname(filename, index):
    try:
        filename, suffix = filename.rsplit('.', 1)
    except ValueError:
        return "%s-%i" % (filename, index)
    return "%s-%i.%s" % (filename, index, suffix)

def _main(source, step):
    try:
        line = next(source)
    except StopIteration:
        return
    start, line = line.split(',', maxsplit=1)
    start = float(start)

    i = 0
    while True:
        with open(partname(source.name, i), 'w') as stream:
            print(','.join(HEADERS), file=stream)

            print(line, end='', file=stream)

            while True:
                try:
                    line = next(source)
                except StopIteration:
                    return

                timestamp, line = line.split(',', maxsplit=1)
                timestamp = float(timestamp)
                if timestamp >= start + step:
                    start = timestamp
                    break
                else:
                    print(line, end='', file=stream)
        i+= 1

def main(args):
    for stream in args.path:
        stream.readline()
        _main(stream, args.step)

def parser():
    _parser = ArgumentParser()
    _parser.add_argument("path", nargs='+', type=FileType('r'))
    _parser.add_argument("step", type=int)
    return _parser

if __name__ == "__main__":
    main(parser().parse_args())
