# external imports
import os

def exportNodeValues(values, fpath, column_name):
	""" 
	exportNodeValues(values, fpath, column_name)

	This method exports node values (e.g. community information, betwenness centrality values) 
	to a CSV file. The values can then be imported to Gephi.
	
	Parameters
	----------
	values : list(float) or networkit.Partition
		Python list or Partition object that contains the values to be exported.
	fpath : str
		The file including its path to be written to.
	column_name : str
		Name of the column as description for the data.
	"""
	f = open(fpath, "w+")
	f.write("id,{0}\n".format(column_name))
	for i in range(0,len(values)):
		f.write("{0},{1}\n".format(i,values[i]))
	f.close()
