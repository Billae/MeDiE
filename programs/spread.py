#! /bin/env python
#

from argparse import ArgumentParser, FileType
from collections import defaultdict

HEADERS = ("operation", "key")

class LazyOpener(dict):

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        for stream in self.values():
            stream.close()
        self.clear()

    def __missing__(self, key):
        self[key] = open(key ,'w')
        # print(','.join(HEADERS), file=self[key])
        return self[key]

def partname(filename, index):
    try:
        filename, suffix = filename.rsplit('.', 1)
    except ValueError:
        return "%s-%i" % (filename, index)
    return "%s-%i.%s" % (filename, index, suffix)

def _main(source, count):
    files = {partname(source.name, i): 0 for i in range(count)}
    key2file = defaultdict(lambda : min(files, key=lambda x: files[x]))
    with LazyOpener() as streams:
        for line in source:
            operation, key, _ = line.split(',')
            _file = key2file[key]
            print(','.join((operation, key)), file=streams[_file])
            files[_file] += 1

def main(args):
    for stream in args.path:
        stream.readline()
        _main(stream, args.count)

def parser():
    _parser = ArgumentParser()
    _parser.add_argument("path", nargs='+', type=FileType('r'))
    _parser.add_argument("count", type=int)
    return _parser

if __name__ == "__main__":
    main(parser().parse_args())
