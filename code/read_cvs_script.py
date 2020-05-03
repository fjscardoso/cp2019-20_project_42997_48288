import pandas as pd
import matplotlib.pyplot as plt

array_runs = [10000, 100000, 1000000, 10000000, 100000000]

df = pd.read_csv(r'output.csv')

sequentialList = []
parallelList = []
mergeList = []
legend = ['Sequential', 'Parallel']
patterns = ['MAP', 'REDUCE']

patterns_by_mean = df.groupby(['Pattern_Tested', 'Array_Size']).mean().reset_index()
# ----MAP----
map_seq_row = patterns_by_mean.loc[patterns_by_mean.Pattern_Tested == 'testMapSequential']
map_par_row = patterns_by_mean.loc[patterns_by_mean.Pattern_Tested == 'testMap']
sequentialList.append(map_seq_row)
parallelList.append(map_par_row)


# ----REDUCE----
reduce_seq_row = patterns_by_mean.loc[patterns_by_mean.Pattern_Tested == 'testReduceSequential']
reduce_par_row = patterns_by_mean.loc[patterns_by_mean.Pattern_Tested == 'testReduce']
sequentialList.append(reduce_seq_row)
parallelList.append(reduce_par_row)

# ----GATHER----
gather_seq_row = patterns_by_mean.loc[patterns_by_mean.Pattern_Tested == 'testGatherSequential']
gather_par_row = patterns_by_mean.loc[patterns_by_mean.Pattern_Tested == 'testGather']
sequentialList.append(gather_seq_row)
parallelList.append(gather_par_row)

# # ----MAP----
# ax = plt.gca()
# result_map.plot(kind='line', x='Array_Size', y='Time_x', ax=ax, label='Parallel')
# result_map.plot(kind='line', x='Array_Size', y='Time_y', color='red', ax=ax, label='Sequential')
# ax.set_ylabel('Time (seconds)')  # Add a y-label to the axes.
# ax.set_xlabel('Array Size')  # Add a y-label to the axes.
# ax.set_title("MAP")  # Add a title to the axes.

for i in range(len(sequentialList)):
    result = pd.merge(sequentialList[i], parallelList[i], on=['Array_Size'])
    mergeList.append(result)

new_df = pd.DataFrame(mergeList)

new_df.to_csv(r'outpu1.csv')

# #MAP
# aux = mergeList[0]
# x_map = aux['Array_Size']
# y_map_seq = aux['Time_x']
# y_map_par = aux['Time_y']
# plt.subplot(2, 1, 1)
# ax=plt.gca()
# ax.get_xaxis().get_major_formatter().set_scientific(False)
# plt.ticklabel_format(useOffset=False)
# plt.xticks(x_map, array_runs)
# plt.plot(x_map,y_map_seq)
# plt.plot(x_map, y_map_par)
# plt.title('Map')
#
# #REDUCE
# aux = mergeList[1]
# x_map = aux['Array_Size'] #Array_Size
# y_map_seq = aux['Time_x']
# y_map_par = aux['Time_y']
# plt.subplot(2, 1, 2)
# ax=plt.gca()
# ax.get_xaxis().get_major_formatter().set_scientific(False)
# plt.ticklabel_format(useOffset=False)
# plt.plot(x_map,y_map_seq)
# plt.plot(x_map, y_map_par)
# plt.title('Reduce')


plt.show()
# print(patterns_by_mean)
# print(map)
# print(map_par_row)
# print(map_seq_row)
# print(map_par_row)
# print(result)
# plt.show()
# print(sequentialList)
# print(parallelList)
print(mergeList)
