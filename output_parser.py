import argparse
import glob
import os

def get_default_stat() -> dict:
    return {'merge_time': 0, 'coverage_calculation_time': 0, 'err_percent': 0}

def create_err_table(stats: dict):
    for audience_size in stats.keys():
        print(f'|{audience_size}|', end='')
        for value in stats[audience_size]:
            err = value['err_percent']
            print(f'{err}|', end='')
        print('')

def create_timings_table(stats_for_default: dict, stats_for_estimator: dict, key: str):
    assert(len(stats_for_default) == len(stats_for_estimator))

    i = 0
    for audience_size in stats_for_estimator.keys():
        print(f'|{audience_size}|', end='')
        print(f'{stats_for_default[i][key]}|', end='')
        for value in stats_for_estimator[audience_size]:
            err = value[key]
            print(f'{err}|', end='')
        print('')

        i += 1

def process_file(path: str) -> None:
    if not os.path.exists(path):
        return
    
    with open(path, 'r') as f:
        lines = f.readlines()
    
    stats_for_default = []
    stats_for_estimator = dict()

    curr_stat = get_default_stat()

    for line in lines:
        if line.find('merge') != -1:
            curr_stat['merge_time'] += int(line.split()[-1][:-2])
        elif line.find('coverage calculation') != -1:
            curr_stat['coverage_calculation_time'] = line.split()[-1][:-2]
        elif line.find('Baseline memory usage') != -1:
            stats_for_default.append(curr_stat)
            curr_stat = get_default_stat()
        elif line.find('error in %') != -1:
            tmp = line.split(':')[1].strip().split()
            curr_stat['err_percent'] = tmp[3]

            if tmp[1] not in stats_for_estimator:
                stats_for_estimator[tmp[1]] = []

            stats_for_estimator[tmp[1]].append(curr_stat)
            curr_stat = get_default_stat()
    
    create_err_table(stats_for_estimator)
    create_timings_table(stats_for_default, stats_for_estimator, 'merge_time')
    create_timings_table(stats_for_default, stats_for_estimator, 'coverage_calculation_time')


def process_directory(path: str) -> None:
    if not os.path.exists(path):
        return
    
    for file_path in glob.glob(os.path.join(path, '*')):
        print(file_path)
        process_file(file_path)
        print('---------------')
    



parser = argparse.ArgumentParser()
parser.add_argument('--dir', help='Path to directory with files to process', nargs='*')
args = parser.parse_args()

for path in args.dir:
    process_directory(path)