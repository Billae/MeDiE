#! /bin/env python
#

from argparse import ArgumentParser, FileType

def _main(stream):
    futur_created = set()
    to_create = set()
    for line in stream:
        timestamp, operation, key, jobid = line.split(',')

        if operation == 'create':
            futur_created.add(key)

        if key in futur_created:
            continue

        to_create.add(int(key, 16))

    print('\n'.join(("create,%x,-1" % key for key in to_create)))

HEADERS = ("timestamp", "operation", "key", "jobid")

def main(args):
    print("operation,key,jobid")
    for stream in args.path:
        stream.readline()
        _main(stream)

def parser():
    _parser = ArgumentParser()
    _parser.add_argument("path", nargs='+', type=FileType('r'))
    return _parser

if __name__ == "__main__":
    main(parser().parse_args())
