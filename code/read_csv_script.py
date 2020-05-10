import pandas as pd
import csv

with open('output.csv', newline='') as oldFile:
    with open('output1.csv', 'w') as newFile:
        reader = csv.reader(oldFile)
        writer = csv.writer(newFile)
        writer.writerow(['Array_Size', 'Pattern_Tested', 'Time', 'Threads'])
        for row in reader:
            writer.writerow(row)

df = pd.read_csv(r'output1.csv')

sequentialList = []
parallelList = []
mergeList = []

patterns_by_mean = df.groupby(
    ['Pattern_Tested', 'Array_Size']).mean().reset_index()

patterns_by_mean.to_csv(r'output2.csv')
