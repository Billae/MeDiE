from collections import defaultdict
import os

def import_data(filePath):
    result = defaultdict(list)
    with open(filePath) as file:
        for line in file:
            words = line.split()
            type = words[0]  # first element
            key = words[1][2:]  # second element minus t=
            job = words[2][2:]  # third element minus J=
            result[job].append((type, key))
    return result

def export_data(data, dirPath):
    os.makedirs(dirPath)
    for job , type_key_list in data.items():
        jobPath = dirPath + '/' + job
        with open(jobPath, 'w') as file:
            for type, key in type_key_list:
                line = "{t},{k}\n".format(t=type, k=key)
                file.write(line)

path = "./cl_dump_sirene"
outPath = "./jobs"
data = import_data(path)
export_data(data, outPath)