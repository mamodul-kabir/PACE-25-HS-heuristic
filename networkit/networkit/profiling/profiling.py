#
# file: profiling.py
# author: Mark Erb
#

from networkit import *
import networkit as nk

import os as os
import sys, traceback
import timeit
import configparser
from pathlib import PurePosixPath, PureWindowsPath

from . import multiprocessing_helper
from . import stat
from . import plot
from ..support import MissingDependencyError


import collections
import math
import fnmatch
import random
try:
	from IPython.core.display import *
except ImportError:
	have_ipython = False
else:
	have_ipython = True

# colors
colors = {
	"green" : (0.003, 0.474, 0.435),
	"red" : (0.501, 0, 0)
}

def getfilepath(filename):
	"""
	getfilepath(filename)

	Helper function the get complete filepath based on filename. Only used by profiler - no need to be called independently.

	Parameters
	----------
	filename : str
		Name of file.
	"""
	filepath = os.path.abspath(os.path.dirname(__file__)) + "/" + filename
	if sys.platform == "win32":
		return PureWindowsPath(filepath)
	else:
		return PurePosixPath(filepath)

def readfile(filename, removeWS=False):
	""" 
	readfile(filename, removeWS=False)

	Helper function for file-loading. Only used by profiler - no need to be called independently.
	
	Parameters
	----------
	filename : str
		Name of file.
	removeWS : bool, optional
		Indicates whether whitespace should be removed. Default: False
	"""
	with open(getfilepath(filename), "r") as file:
		result = file.read()
		if removeWS:
			result = " ".join(result.split())
		return result



try:
	__IPYTHON__

	def _initHeader(tag, docType, data):
		""" private helper function for ipython/jupyter-notebook hack: create content of extended header """
		result = """
			{
				var element = document.getElementById('NetworKit_""" + tag + """');
				if (element) {
					element.parentNode.removeChild(element);
				}
				element = document.createElement('""" + tag + """');
				element.type = 'text/""" + docType + """';
				element.innerHTML = '""" + data + """';
				element.setAttribute('id', 'NetworKit_""" + tag + """');
				document.head.appendChild(element);
			}
		"""
		return result


	def _initOverlay(name, data):
		""" private helper function for ipython/jupyter-notebook hack: create content of overlay """
		result = """
			{
				var element = document.getElementById('NetworKit_""" + name + """');
				if (element) {
					element.parentNode.removeChild(element);
				}
				element = document.createElement('div');
				element.innerHTML = '<div id="NetworKit_""" + name + """_Toolbar_Top"><div class="button icon-close" id="NetworKit_""" + name + """_Close" /></div>""" + data + """';
				element.setAttribute('id', 'NetworKit_""" + name + """');
				document.body.appendChild(element);
				document.getElementById('NetworKit_""" + name + """_Close').onclick = function (e) {
					document.getElementById('NetworKit_""" + name + """').style.display = 'none';
				}
			}
		"""
		return result

	# load headers/scripts
	display_html(
		HTML("""
			<script type="text/javascript">
			<!--
				""" + _initHeader("script", "javascript", readfile("html/profiling.js", True))  + """
				""" + _initHeader("style",  "css",        readfile("html/profiling.css", True)) + """
				""" + _initOverlay("Overlay", readfile("html/overlay.html", True)) + """
			-->
			</script>
		""")
	)
except NameError as e:
	pass


class Config:
	""" 
	Config()

	Object to control profiling.Profile behaviour."""

	def __init__(self):
		""" constructor: all options off """
		self.__options_Properties = {
			"Diameter": False,
			"EffectiveDiameter": False
		}
		self.__options_Measures = {
			"Centrality.Degree": False,
			"Centrality.CoreDecomposition": False,
			"Centrality.ClusteringCoefficient": False,
			"Centrality.PageRank": False,
			"Centrality.KPath": False,
			"Centrality.Katz": False,
			"Centrality.Betweenness": False,
			"Centrality.Closeness": False,
			"Partition.Communities": False,
			"Partition.ConnectedComponents": False,
			"Partition.CoreDecomposition": False
		}
		self.__options_MeasureCorrelations = {
			"Pearson": False,
			"Spearman": False,
			"Fechner": False
		}

	@classmethod
	def createConfig(cls, preset="default"):
		""" 
		createConfig(preset="default")
		
		Create a config object by selection a named preset of predefined options. 
		
		Supported values for preset:
		
		- complete
		- minimal
		- default
		

		Parameters
		----------
		preset : str, optional
			Preset to be used for generating the config values. Default: "default"
		"""
		result = Config()

		if preset == "complete":
			result.setProperty("Diameter")
			result.setProperty("EffectiveDiameter")
			result.setMeasure("Centrality.Degree"),
			result.setMeasure("Centrality.CoreDecomposition")
			result.setMeasure("Centrality.ClusteringCoefficient")
			result.setMeasure("Centrality.PageRank")
			result.setMeasure("Centrality.Katz")
			result.setMeasure("Centrality.Betweenness")
			result.setMeasure("Centrality.Closeness")
			result.setMeasure("Partition.Communities")
			result.setMeasure("Partition.ConnectedComponents")
			result.setMeasure("Partition.CoreDecomposition")
			result.setMeasureCorrelation("Spearman")

		elif preset == "minimal":
			result.setMeasure("Centrality.Degree")
			result.setMeasure("Partition.ConnectedComponents")

		elif preset == "default":
			result.setProperty("Diameter")
			result.setMeasure("Centrality.Degree")
			result.setMeasure("Centrality.ClusteringCoefficient")
			result.setMeasure("Centrality.PageRank")
			result.setMeasure("Centrality.Betweenness")
			result.setMeasure("Centrality.Katz")
			result.setMeasure("Centrality.CoreDecomposition")
			result.setMeasure("Partition.ConnectedComponents")
			result.setMeasure("Partition.Communities")
			result.setMeasure("Partition.CoreDecomposition")
			result.setMeasureCorrelation("Spearman")
		else:
			raise Error("no preset given")

		return result


	def setProperty(self, id, enabled=True):
		""" 
		setProperty(id, enabled=True)
		
		Enable/disable given property option.
		
		Supported values for id:

		- Diameter
		- EffectiveDiameter

		Parameters
		----------
		id : str
			Property value to set.
		enabled : bool, optional
			Indicates whether property id should be enabled or not. Default: True
		"""
		if id in self.__options_Properties:
			self.__options_Properties[id] = enabled


	def getProperty(self, id):
		""" 
		setProperty(id)
		
		return state of given property option
		
		Supported values for id:

		- Diameter
		- EffectiveDiameter

		Returns
		-------
		bool
			State of property id.
		"""
		return self.__options_Properties[id]


	def setMeasure(self, id, enabled=True):
		""" 
		setMeasure(id, enabled=True)
		
		Enable/disable given measure option.
		
		Supported values for id:

		- Centrality.Degree
		- Centrality.CoreDecomposition
		- Centrality.ClusteringCoefficient
		- Centrality.PageRank
		- Centrality.KPath
		- Centrality.Katz
		- Centrality.Betweenness
		- Centrality.Closeness
		- Partition.Communities
		- Partition.ConnectedComponents
		- Partition.CoreDecomposition

		Parameters
		----------
		id : str
			Measure value to set.
		enabled : bool, optional
			Indicates whether measure id should be enabled or not. Default: True
		"""
		if id in self.__options_Measures:
			self.__options_Measures[id] = enabled


	def getMeasure(self, id):
		""" 
		getMeasure(id)
		
		Return state of given measure option
		
		Supported values for id:

		- Centrality.Degree
		- Centrality.CoreDecomposition
		- Centrality.ClusteringCoefficient
		- Centrality.PageRank
		- Centrality.KPath
		- Centrality.Katz
		- Centrality.Betweenness
		- Centrality.Closeness
		- Partition.Communities
		- Partition.ConnectedComponents
		- Partition.CoreDecomposition

		Returns
		-------
		bool
			State of measure id.
		"""
		return self.__options_Measures[id]


	def setMeasureCorrelation(self, id, enabled=True):
		""" 
		setMeasureCorrelation(id, enabled=True)
		
		Enable/disable given measure correlation option.
		
		Supported values for id:

		- Pearson
		- Spearman
		- Fechner

		Parameters
		----------
		id : str
			Measure correlation value to set.
		enabled : bool, optional
			Indicates whether measure correlation id should be enabled or not. Default: True
		"""
		if id in self.__options_MeasureCorrelations:
			self.__options_MeasureCorrelations[id] = enabled


	def getMeasureCorrelation(self, id):
		""" 
		getMeasureCorrelation(id)
		
		Return state of given correlation (measure) option
		
		Supported values for id:

		- Pearson
		- Spearman
		- Fechner

		Returns
		-------
		bool
			State of measure correlation id.
		"""
		"""  """
		return self.__options_MeasureCorrelations[id]



class Profile:
	""" 
	Profile(G, token=object())
	
	Automated network profiling class. Don't call the constructor directly.
	Use networkit.profile.profile.Profile.create to instantiate a profile.
	"""

	__TOKEN = object()	# see __init__(): prevent this class from being instanced directly
	__pageCount = 0
	__token = ""
	__verbose = False
	__verboseLevel = 0
	__verboseFilename = ""
	__parallel = multiprocessing_helper.numberOfProcessors()


	def __init__(self, G, token=object()):
		if token is not self.__TOKEN:
			raise ValueError("call create(G) to create an instance")
		self.__G = G
		self.__properties = {}
		self.__measures = collections.OrderedDict()
		self.__correlations = {}

		self.__token = ''.join(random.choice('0123456789abcdef') for n in range(16))



	@classmethod
	def create(cls, G, preset="default", config=None):
		""" 
		create(G, preset="default", config=None)

		Creates a profile object

		Parameters
		----------
		G : networkit.Graph
			Graph to profile.
		preset: str
			Name of preset configuration: "complete", "minimal", "default"
		Config: networkit.profile.profile.Config
			Config object to control some aspects of the generation behaviour. Default: None
	
		Returns
		-------
		networkit.profile.profile.Profile
			Resulting profiling object.
		"""

		# if no custom config is given, use a preconfigured config according to preset name
		if not config:
			config = Config.createConfig(preset)

		result = cls(G, cls.__TOKEN)
		# TODO: use copy constructor instead
		result.__config = config

		nk.setNumberOfThreads(result.__parallel)

		def funcScores(instance):
			""" returns node scores """
			return instance.scores()

		def funcSizes(instance):
			""" returns partition subset sizes """
			return sorted(instance.getPartition().subsetSizes())

		if G.isDirected():
			classConnectedComponents = components.StronglyConnectedComponents
		else:
			classConnectedComponents = components.ConnectedComponents

		# internal unique name | category name | display name |
		# compute correlation within same category | value function for measures | display name (axis) | class name of measure | parameter of constructor
		for parameter in [
			("Centrality.Degree",					"Node Centrality",	"Degree",
				True,	funcScores,	"Score",				centrality.DegreeCentrality, 			(G, )),
			("Centrality.CoreDecomposition",		"Node Centrality",	"k-Core Decomposition",
				True,	funcScores,	"Score",				centrality.CoreDecomposition, 			(G, )),
			("Centrality.ClusteringCoefficient",	"Node Centrality",	"Local Clustering Coefficient",
				True,	funcScores,	"Score",				centrality.LocalClusteringCoefficient,	(G, )),
			("Centrality.PageRank", 				"Node Centrality",	"PageRank",
				True,	funcScores,	"Score",				centrality.PageRank, 					(G, )),
			("Centrality.KPath", 					"Node Centrality",	"k-Path Centrality",
				True,	funcScores,	"Score",				centrality.KPathCentrality,				(G, )),
			("Centrality.Katz",						"Node Centrality",	"Katz Centrality",
				True,	funcScores,	"Score",				centrality.KatzCentrality,				(G, )),
			("Centrality.Betweenness", 				"Node Centrality",	"Betweenness",
				True,	funcScores,	"Score",				centrality.EstimateBetweenness,			(G, 10, True)),
			("Centrality.Closeness",				"Node Centrality",	"Closeness",
				True,	funcScores,	"Score",				centrality.ApproxCloseness,				(G, min(10, G.numberOfNodes()), True)),
			("Partition.Communities", 				"Partition",		"Communities",
				False,	funcSizes,	"Nodes per Community",	community.PLM,			 				(G, )),
			("Partition.ConnectedComponents", 		"Partition",		"Connected Components",
				False,	funcSizes,	"Nodes per Component",	classConnectedComponents,				(G, )),
			("Partition.CoreDecomposition", 		"Partition",		"k-Core Decomposition",
				False,	funcSizes,	"Nodes per Shell",		centrality.CoreDecomposition, 			(G, ))
		]: result.__addMeasure(parameter)

		if cls.__verbose:
			timerAll = timeit.default_timer()
		result.__loadProperties()
		result.__loadMeasures()
		if cls.__verbose:
			if cls.__verboseLevel < 1:
				print("")
			print("\ntotal time (measures + stats + correlations): {:.2F} s".format(timerAll - timeit.default_timer()))
			print("total speed: {:.1F} edges/s".format(G.numberOfEdges() / (timerAll - timeit.default_timer())))
		return result;


	@classmethod
	def setVerbose(cls, verbose=False, level=0, filename=""):
		"""
		setVerbose(verbose=False, level=0, filename="")
		
		Set verbose behaviour of all public methods.

		Parameters
		----------
		verbose: bool, optional
		Enable/disable display verbose. Default: False
		level: int, optional
			Set level of verbose (0, 1). Default: False
		filename: str, optional
			Enable/disable additional logfile support to given file. Default: ""
		"""
		cls.__verbose = verbose
		cls.__verboseLevel = level
		cls.__verboseFilename = filename


	@classmethod
	def getVerbose(cls):
		""" 
		getVerbose()
		
		Returns verbose settings.
		
		Returns
		-------
		tuple(bool, int, str)
			Tuple with three entries -> (verbose, verboseLevel, filename).
		"""
		return (cls.__verbose, cls.__verboseLevel, cls.__verboseFilename)


	@classmethod
	def setParallel(cls, parallel):
		"""
		setParallel(parallel)

		Set the number of parallel threads/processes to use.
		
		Parameters
		----------
		parallel : int
			Number of threads to use for algorithms used during profiling.
		"""
		if (parallel < 1):
			raise ValueError("parallel < 1");
		cls.__parallel = parallel


	@classmethod
	def getParallel(cls):
		"""
		getParallel()

		Returns the number of parallel threads/processes to use
		
		Returns
		-------
		int
			Number of threads to use for algorithms used during profiling.
		"""
		return cls.__parallel


	def getStat(self, measure):
		""" 
		getStat(measure)

		Returns an directory of all computed measure values.

		Supported values for measure:

		- Centrality.Degree
		- Centrality.CoreDecomposition
		- Centrality.ClusteringCoefficient
		- Centrality.PageRank
		- Centrality.KPath
		- Centrality.Katz
		- Centrality.Betweenness
		- Centrality.Closeness
		- Partition.Communities
		- Partition.ConnectedComponents
		- Partition.CoreDecomposition

		Parameters
		----------
		measure : str
			Measure name.

		Returns
		-------
		dict(float)
			Dictionary with computed measure values.
		"""
		return self.__measures[measure]["stat"]


	def getCategory(self, measure):
		""" 
		getCategory(measure)

		Returns the category of an given measure.
		
		Supported values for measure:

		- Centrality.Degree
		- Centrality.CoreDecomposition
		- Centrality.ClusteringCoefficient
		- Centrality.PageRank
		- Centrality.KPath
		- Centrality.Katz
		- Centrality.Betweenness
		- Centrality.Closeness
		- Partition.Communities
		- Partition.ConnectedComponents
		- Partition.CoreDecomposition

		Parameters
		----------
		measure : str
			Measure name.

		Returns
		-------
		str
			Category of measure.
		"""
		return self.__measures[measure]["category"]


	def getElapsedTime(self, measure):
		""" 
		getElapsedTime(measure)

		Returns the elapsed computation times of an given measure.
		
		Supported values for measure:

		- Centrality.Degree
		- Centrality.CoreDecomposition
		- Centrality.ClusteringCoefficient
		- Centrality.PageRank
		- Centrality.KPath
		- Centrality.Katz
		- Centrality.Betweenness
		- Centrality.Closeness
		- Partition.Communities
		- Partition.ConnectedComponents
		- Partition.CoreDecomposition

		Parameters
		----------
		measure : str
			Measure name.

		Returns
		-------
		float
			Elapsed time for computing measure.
		"""
		return self.__measures[measure]["time"]


	def output(self, outputType, directory, filename='graph', style="light", color=colors["green"], parallel=False):
		""" 
		output(outputType, directory, filename='graph', style="light", color=colors["green"], parallel=False)
		
		Outputs a computed profile to disk.

		Parameters
		----------
		outputType: str
			Profile output format ("HTML", "LaTeX").
		directory: str
			Directory to write.
		filename: str, optional
			Enable/disable additional logfile support to given file. Default: ""
		style: str, optional
			Style of generated output. Default: "light"
		color: tuple(float, float, float), optional
			Mainly used color of given style. Given in RGB values between 0.0 and 1.0. Default: "green" -> (0.003, 0.474, 0.435)
		parallel: bool, optional
			Run some additional parts of the generation in parallel (experimental). Default: False
		"""

		# TODO: type -> enum
		options_type = ["HTML", "LaTeX", None]
		for o in options_type:
			if o is None:
				raise ValueError("unknown output type: options are " + str(options_type[0:len(options_type)-1]))
			if o == outputType:
				break

		result = self.__format(
			outputType = outputType,
			directory = directory,
			filename = filename,
			style = style,
			color = color,
			pageIndex = 0,
			parallel = parallel,
			token = self.__token,
		)

		if outputType == "HTML":
			js = readfile("html/profiling.js", False).replace("\\\\", "\\")
			css = readfile("html/profiling.css", False).replace("\\\\", "\\")
			result = """
				<!DOCTYPE HTML>
				<html>
					<head>
						<meta charset="utf-8">
						<style>
							""" + css + """
						</style>
						<script type="text/x-mathjax-config">
							MathJax.Hub.Config({tex2jax: {inlineMath: [['$','$']]}});
						</script>
						<script type="text/javascript" src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
						<script>
							""" + js + """
						</script>
					</head>
					<body>
						""" + result + """
					</body>
				</html>
			"""
			filename += "html"
		elif outputType == "LaTeX":
			result = result
			if directory[-1] == "/":
				directory += filename[:-1]
			else:
				directory += "/" + filename[:-1]
			filename += "tex"
		else:
			raise Error("unknown output type")

		with open(directory + "/" + filename, 'w') as file:
			file.write(result)


	def show(self, style="light", color=colors["green"], parallel=False):
		""" 
		show(style="light", color=colors["green"], parallel=False)

		Display computed profile.

		Parameters
		----------
		style: str, optional
			Style of generated output. Default: "light"
		color: tuple(float, float, float), optional
			Mainly used color of given style. Given in RGB values between 0.0 and 1.0. Default: "green" -> (0.003, 0.474, 0.435)
		parallel: bool, optional
			Run some additional parts of the generation in parallel (experimental). Default: False
		"""
		if not have_ipython:
			raise MissingDependencyError("IPython")
		try:
			__IPYTHON__
		except:
			raise RuntimeError("this function cannot be used outside ipython notebook")

		result = self.__format(
			outputType = "HTML",
			directory = "",
			filename = "",
			style = style,
			color = color,
			token = self.__token,
			pageIndex = self.__pageCount,
			parallel = parallel
		)

		display_html(HTML(result))
		self.__pageCount = self.__pageCount + 1


	def __format(self, outputType, directory, filename, style, color, token, pageIndex, parallel):
		""" layouts the profile	"""
		confParser = configparser.ConfigParser()
		confParser.read(getfilepath("description/descriptions.txt"))
		theme = plot.Theme()
		theme.set(style, color)

		# TODO: refactor and use decorator design pattern instead if an 3rd format is supported
		if outputType == "HTML":
			plottype = "SVG"
			options = []
			templateProfile = readfile("html/profile.html", False)
			templateMeasure = readfile("html/measure.html", False)
		elif outputType == "LaTeX":
			plottype = "PDF"
			if directory[-1] == "/":
				output_dir = directory + filename[:-1]
			else:
				output_dir = directory + "/" + filename[:-1]
			if not os.path.isdir(output_dir):
				os.mkdir(output_dir)
			options = [output_dir, filename]
			templateProfile = readfile("latex/profile.tex", False)
			templateMeasure = readfile("latex/measure.tex", False)

		# generate plots
		pool = multiprocessing_helper.ThreadPool(self.__parallel, parallel)
		for name in self.__measures:
			category = self.__measures[name]["category"]
			pool.put(
				plot.Measure(plottype, options, name, (
					0,
					self.__measures[name]["stat"],
					category,
					self.__measures[name]["name"],
					self.__measures[name]["label"],
					theme
				))
			)
			pool.put(
				plot.Measure(plottype, options, name, (
					1,
					self.__measures[name]["stat"],
					category,
					self.__measures[name]["name"],
					self.__measures[name]["label"],
					theme
				))
			)
			if category == "Partition":
				pool.put(
					plot.Measure(plottype, options, name, (
						2,
						self.__measures[name]["stat"],
						category,
						self.__measures[name]["name"],
						self.__measures[name]["label"],
						theme
					))
				)
		for category in self.__correlations:
			keyBList = []
			for keyA in self.__measures:
				if self.__measures[keyA]["category"] == category and self.__measures[keyA]["correlate"]:
					keyBList.append(keyA)
					for keyB in keyBList:
						if keyA != keyB:
							try:
								value = self.__correlations[category][keyA][keyB]
								keyFirst = keyA
								keySecond = keyB
							except:
								value = self.__correlations[category][keyB][keyA]
								keyFirst = keyB
								keySecond = keyA
							pool.put(
								plot.Scatter(plottype, options, keyFirst, (
									keySecond,
									self.__measures[keyFirst]["name"],
									self.__measures[keySecond]["name"],
									self.__measures[keyFirst]["label"],
									self.__measures[keySecond]["label"],
									self.__measures[keyFirst]["stat"],
									self.__measures[keySecond]["stat"],
									value["stat"],
									theme
								))
							)
		while pool.numberOfTasks() > 0:
			(taskType, name, data) = pool.get()
			try:
				category = self.__measures[name]["category"]

				if taskType == "Plot.Measure":
					(index, image) = data
					self.verbosePrint("Plot.Measure", level=1)
					self.__measures[name]["image"][index] = image

				elif taskType == "Plot.Scatter":
					(nameB, image) = data
					self.verbosePrint("Plot.Scatter (" + name + " <-> " + nameB + ")", level=1)
					self.__correlations[category][name][nameB]["image"] = image
			except Exception as e:
				self.verbosePrint("Error (Post Processing): " + taskType + " - " + name, level=-1)
				self.verbosePrint(str(e), level=-1)
		pool.join()

		# outline results
		results = {}
		for category in self.__correlations:
			results[category] = {}
			results[category]["Correlations"] = {}
			results[category]["Correlations"]["HeatMaps"] = ""
			results[category]["Correlations"]["ScatterPlots"] = ""
			results[category]["Measures"] = ""
			results[category]["Overview"] = ""

			def funcHeatMap(category, correlationName):
				result = ""
				keyBList = []

				if outputType == "HTML":
					result += "<div class=\"SubCategory HeatTable\" data-title=\"" + correlationName + "\">"
					for keyA in self.__measures:
						if self.__measures[keyA]["category"] == category and self.__measures[keyA]["correlate"]:
							keyBList.append(keyA)
							for keyB in keyBList:
								try:
									value = self.__correlations[category][keyA][keyB]
								except:
									value = self.__correlations[category][keyB][keyA]
								nameA = self.__measures[keyA]["name"]
								nameB = self.__measures[keyB]["name"]
								result += "<div class=\"HeatCell\" title=\"" + nameB + " - " + nameA + "\" data-heat=\"{:+.3F}\"></div>".format(value["stat"]["Value"][correlationName])
							result += "<div class=\"HeatCellName\">" + nameB + "</div><br>"
					result += "</div>"
				elif outputType == "LaTeX":
					i = 0
					n = len(self.__measures)
					result += "\\subsection{" + correlationName + "}\n"
					result += "\\begin{tabular}{" + ("l" * (n+1)) +"}"
					for keyA in self.__measures:
						if self.__measures[keyA]["category"] == category and self.__measures[keyA]["correlate"]:
							keyBList.append(keyA)
							for keyB in keyBList:
								try:
									value = self.__correlations[category][keyA][keyB]
								except:
									value = self.__correlations[category][keyB][keyA]
								nameA = self.__measures[keyA]["name"]
								nameB = self.__measures[keyB]["name"]
								result += "\\cellcolor{"
								result += "red" if value["stat"]["Value"][correlationName] > 0 else "blue"
								result += "!" + str(abs(value["stat"]["Value"][correlationName]) * 100)
								result += "}"
								result += "{:+.3F} & ".format(value["stat"]["Value"][correlationName])
							result += "\\multicolumn{" + str(n-i) + "}" + "{l}{" + nameB + "} \\\\"
						i += 1
					result += "\\end{tabular}"
				return result
			if self.__config.getMeasureCorrelation("Pearson"):
				results[category]["Correlations"]["HeatMaps"] += funcHeatMap(category, "Pearson's Correlation Coefficient")
			if self.__config.getMeasureCorrelation("Spearman"):
				results[category]["Correlations"]["HeatMaps"] += funcHeatMap(category, "Spearman's Rank Correlation Coefficient")
			if self.__config.getMeasureCorrelation("Fechner"):
				results[category]["Correlations"]["HeatMaps"] += funcHeatMap(category, "Fechner's Correlation Coefficient")

			def funcScatterPlot(category):
				result = ""

				keyBList = []
				for keyA in self.__measures:
					if self.__measures[keyA]["category"] == category and self.__measures[keyA]["correlate"]:
						keyBList.append(keyA)
						for keyB in keyBList:
							if keyA != keyB:
								try:
									value = self.__correlations[category][keyA][keyB]
								except:
									value = self.__correlations[category][keyB][keyA]
#								if outputType == "HTML":
#									result += "<div class=\"Thumbnail_ScatterPlot\" data-title=\"" + keyB + "\" data-title-second=\"" + keyA + "\"><img src=\"data:image/svg+xml;utf8," + value["image"] + "\" /></div>"
#								elif outputType == "LaTeX":
#									result += "\n\\includegraphics[width=0.5\\textwidth]{{\"" + value["image"] + "\"}.pdf}"
				return result
			results[category]["Correlations"]["ScatterPlots"] += funcScatterPlot(category)

		# group results to fit templates (measure)
		for key in self.__measures:
			measure = self.__measures[key]
			name = measure["name"]
			category = measure["category"]
			image = measure["image"]
			stat = measure["stat"]
			assortativity = measure["assortativity"]
			centralization = measure["centralization"]
			algorithm = measure["class"].__name__

			description = "N/A"
			try:
				description = confParser.get("kernel descriptions", key)
			except:
				print("couldn't read description for\t{}".format(key))
				pass

			extentions = ""
			try:
				if outputType == "HTML":
					extentions = "<div class=\"PartitionPie\"><img src=\"data:image/svg+xml;utf8," + image[2] + "\" /></div>"
				elif outputType == "LaTeX":
					extentions = "\n\\includegraphics[width=0.5\\textwidth]{{\"" + image[2] + "\"}.pdf}"

			except:
				pass

			results[category]["Measures"] += self.__formatMeasureTemplate(
				templateMeasure,
				pageIndex,
				key,
				name,
				image,
				stat,
				assortativity,
				centralization,
				extentions,
				description,
				algorithm
			)
			if outputType == "HTML":
				results[category]["Overview"] += "<div class=\"Thumbnail_Overview\" data-title=\"" + name + "\"><a href=\"#NetworKit_Page_" + str(pageIndex) + "_" + key + "\"><img src=\"data:image/svg+xml;utf8," + image[1] + "\" /></a></div>"
			elif outputType == "LaTeX":
				results[category]["Overview"] += "\n\\includegraphics[width=0.5\\textwidth]{{\"" + image[1] + "\"}.pdf}\n"

		result = self.__formatProfileTemplate(
			templateProfile,
			token,
			pageIndex,
			results
		)
		return result


	def __formatMeasureTemplate(self, template, pageIndex, key, name, image, stat, assortativity, centralization, extentions, description, algorithm):
		""" format measure template - all function parameters, are available for the template """
		result = template.format(**locals())
		return result


	def __formatProfileTemplate(self, template, token, pageIndex, results):
		""" format profile template - all function parameters, are available for the template """
		properties = self.__properties
		result = template.format(**locals())
		return result


	def __addMeasure(self, args):
		""" add a measure if it is enabled in the given config object """
		(measureKey, measureCategory, measureName, correlate, getter, label, measureClass, parameters) = args
		if self.__config.getMeasure(measureKey):
			measure = {}
			measure["name"] = measureName
			measure["category"] = measureCategory
			measure["correlate"] = correlate
			measure["getter"] = getter
			measure["label"] = label
			measure["class"] = measureClass
			measure["parameters"] = parameters
			measure["data"] = {}
			measure["image"] = {}
			self.__measures[measureKey] = measure
		try:
			self.__correlations[measureCategory]
		except:
			self.__correlations[measureCategory] = {}


	def __loadProperties(self):
		""" calculate the network properties """
		self.__properties["Nodes"] = self.__G.numberOfNodes()
		self.__properties["Edges"] = self.__G.numberOfEdges()
		self.__properties["Density"] = nk.graphtools.density(self.__G)
		self.__properties["Directed"] = self.__G.isDirected()
		self.__properties["Weighted"] = self.__G.isWeighted()
		self.__properties["Self Loops"] = self.__G.numberOfSelfLoops()

		if self.__config.getProperty("Diameter"):
			try:
				timerInstance = timeit.default_timer()
				self.verbosePrint("Diameter: ", end="")
				diam = distance.Diameter(self.__G, distance.DiameterAlgo.ESTIMATED_RANGE, error = 0.1)
				diameter = diam.run().getDiameter()
				elapsedMain = timeit.default_timer() - timeit.default_timer()
				self.verbosePrint("{:.2F} s".format(elapsedMain))
				self.verbosePrint("")
			except Exception as e:
				self.verbosePrint("Diameter raised exception: {}".format(e))
				diameter = "N/A"
		else:
			diameter = "N/A"
		self.__properties["Diameter Range"] = diameter

		if self.__config.getProperty("EffectiveDiameter"):
			try:
				timerInstance = timeit.default_timer()
				self.verbosePrint("EffectiveDiameter: ", end="")
				diam = distance.EffectiveDiameterApproximation(self.__G)
				diameter = diam.run().getEffectiveDiameter()
				elapsedMain = timeit.default_timer() - timerInstance
				self.verbosePrint("{:.2F} s".format(elapsedMain))
				self.verbosePrint("")
			except:
				self.verbosePrint("EffectiveDiameter raised exception")
				diameter = "N/A"
		else:
			diameter = "N/A"
		self.__properties["Effective Diameter"] = diameter


		timerInstance = timeit.default_timer()
		self.verbosePrint("Connected Components: ", end="")
		try:
			if self.__G.isDirected():
				cc = components.StronglyConnectedComponents(self.__G)
			else:
				cc = components.ConnectedComponents(self.__G)
			cc.run()
			num = cc.numberOfComponents()
		except:
			self.verbosePrint("ConnectedComponents raised exception")
			num = "N/A"
		elapsedMain = timeit.default_timer() - timerInstance
		self.verbosePrint("{:.2F} s".format(elapsedMain))
		self.verbosePrint("")
		self.__properties["Connected Components"] = num


	def __loadMeasures(self):
		""" calculate the network measures and stats """
		pool = multiprocessing_helper.ThreadPool(self.__parallel, False)

		failed_measures = list()

		for name, measure in self.__measures.items():
			self.verbosePrint(name + ": ", end="")
			try:
				instance = measure["class"](*measure["parameters"])
			except Exception as e:
				self.verbosePrint("(removed)\n>> " + str(e))
				failed_measures.append(name)
				continue

			# run algorithm and get result
			timerInstance = timeit.default_timer()
			instance.run()
			measure["data"]["sample"] = measure["getter"](instance)
			#self.verbosePrint("{0} called on {1}".format(measure["getter"], instance))
			elapsedMain = timeit.default_timer() - timerInstance
			self.verbosePrint("{:.2F} s".format(elapsedMain))

			self.verbosePrint("    Sort: ", end="")
			timerPostSort = timeit.default_timer()
			measure["data"]["sorted"] = sorted(measure["data"]["sample"])
			elapsedPostSort = timeit.default_timer() - timerPostSort
			self.verbosePrint("{:.2F} s".format(elapsedPostSort))

			self.verbosePrint("    Rank: ", end="")
			timerPostRank = timeit.default_timer()
			measure["data"]["ranked"] = helpers.ranked(measure["data"]["sample"])
			elapsedPostRank = timeit.default_timer() - timerPostRank
			self.verbosePrint("{:.2F} s".format(elapsedPostRank))

			if self.__measures[name]["category"] == "Node Centrality":
				self.verbosePrint("    Assortativity: ", end="")
				timerPostAssortativity = timeit.default_timer()
				assortativity = nk.correlation.Assortativity(self.__G, measure["data"]["sample"])
				assortativity.run()
				measure["assortativity"] = assortativity.getCoefficient()
				elapsedPostAssortativity = timeit.default_timer() - timerPostAssortativity
				self.verbosePrint("{:.2F} s".format(elapsedPostAssortativity))
			else:
				measure["assortativity"] = float("nan")

			if self.__measures[name]["category"] == "Node Centrality":
				self.verbosePrint("    Centralization: ", end="")
				timerPostCentralization = timeit.default_timer()
				try:
					measure["centralization"] = instance.centralization()
				except:
					self.verbosePrint("Centrality.centralization not properly defined for {0}. ".format(name), level=0, end="")
					measure["centralization"] = float("nan")
				elapsedPostCentralization =  timeit.default_timer() - timerPostCentralization
				self.verbosePrint("{:.2F} s".format(elapsedPostCentralization))
			else:
				measure["centralization"] = float("nan")

			measure["time"] = (
				elapsedMain,
				elapsedPostSort,
				elapsedPostRank,
				elapsedPostAssortativity,
				elapsedPostCentralization
			)

		self.verbosePrint("")

		for name in failed_measures:
			del self.__measures[name]

		for name in self.__measures:
			# the fix below avoids a cake-diagram for connected graphs,
			# as there is no other other way to include connected components for now,
			# the detailed output for connected graphs is accepted
			#if len(self.__measures[name]["data"]["sample"]) <= 1:	# for case connected graph...
			#	del self.__measures[name]
			#else:
			category = self.__measures[name]["category"]
			pool.put(
				stat.Stat(name, (
					self.__measures[name]["data"]["sample"],
					self.__measures[name]["data"]["sorted"],
					self.__measures[name]["data"]["ranked"],
					category == "Partition"
				))
			)

		while pool.numberOfTasks() > 0:
			(jobType, name, data) = pool.get()

			try:
				category = self.__measures[name]["category"]

				if jobType == "Stat":
					self.__measures[name]["stat"] = data
					self.verbosePrint("Stat: " + name, level=1)
					if self.__measures[name]["correlate"]:
						for key in self.__correlations[category]:
							self.__correlations[category][key][name] = {}
							self.__correlations[category][key][name]["stat"] = {}
							pool.put(
								stat.Correlation(key, (
									name,
									self.__measures[key]["data"]["sample"],
									self.__measures[key]["data"]["ranked"],
									self.__measures[key]["stat"],
									self.__measures[name]["data"]["sample"],
									self.__measures[name]["data"]["ranked"],
									self.__measures[name]["stat"]
								))
							)
						self.__correlations[category][name] = {}
						self.__correlations[category][name][name] = {}
						self.__correlations[category][name][name]["stat"] = {}
						self.__correlations[category][name][name]["stat"]["Value"] = {
							"Spearman's Rank Correlation Coefficient": 1,
							"Pearson's Correlation Coefficient": 1,
							"Fechner's Correlation Coefficient": 1
						}
						self.__correlations[category][name][name]["image"] = ""

				elif jobType == "Correlation":
					(nameB, correlation) = data
					self.verbosePrint("Correlation: " + name + " <-> " + nameB, level=1)
					self.__correlations[category][name][nameB]["stat"] = correlation
			except Exception as e:
				self.verbosePrint("Error (Post Processing): " + jobType + " - " + name, level=-1)
				self.verbosePrint(str(e), level=-1)

		pool.join()


	@classmethod
	def verbosePrint(cls, text="", end="\n", level=0):
		""" 
		verbosePrint(text="", end="\\n", level=0)

		Print for verbose output.
		
		Parameters
		----------
		text : str, optional
			Text to add to the verbose print. Default: ""
		end : str, optional
			Marker for line break. Default: "\\n"
		level : int, optional
			Level for verbose printing. Default: 0
		"""
		if cls.__verboseLevel >= level:
			text = text + end
		else:
			text = "."

		if cls.__verbose or level < 0:
			print(text, end="", flush=True)

		if cls.__verboseFilename != "":
			with open(cls.__verboseFilename, 'a+') as file:
				file.write(text)


def walk(inputDir, outputDir, graphFormat, filePattern="*",  preset="default", config=None, outputType="HTML", style="light", color=colors["green"], recursive=False, parallel=False):
	""" 
	walk(inputDir, outputDir, graphFormat, filePattern="`*`",  preset="default", config=None, outputType="HTML", style="light", color=colors["green"], recursive=False, parallel=False)

	Tests all files of a directory for the given conditions and generates a profile when matching.

	Parameters
	----------
	inputDir : str
		The directory to search.
	outputDir : str
		Directory to write the generated profiles.
	graphFormat : networkit.graphio.Format
		Format of input graphs.
	filePattern : str, optional
		Specify accepted file names, e.g.: `*.METIS.graph`. Default: `*`
	preset : str, optional
		Config preset ("minimal", "default", "full"). Default: "default"
	Config: networkit.profile.profile.Config
		Config object to control some aspects of the generation behaviour. Default: None	
	outputType: str, optional
		Profile output format ("HTML", "LaTeX"). Default: "HTML"
	style: str, optional
		Style of generated output. Default: "light"
	color: tuple(float, float, float), optional
		Mainly used color of given style. Given in RGB values between 0.0 and 1.0. Default: "green" -> (0.003, 0.474, 0.435)
	recursive : bool, optional
		Also search in subfolders for matching files. Default: False
	parallel: bool, optional
		Run some additional parts of the generation in parallel (experimental). Default: False
	"""

	# if no custom config is given, use a preconfigured config according to preset name
	if not config:
		config = Config.createConfig(preset)

	if not os.path.isdir(outputDir):
		os.mkdir(outputDir)

	for (dirpath, dirnames, filenames) in os.walk(inputDir):
		for filename in filenames:
			file = dirpath + "/" + filename
			if fnmatch.fnmatch(filename, filePattern):
				Profile.verbosePrint("\n[ " + file + " ]")
				try:
					G = nk.readGraph(file, graphFormat)
					try:
						pf = Profile.create(
							G,
							config = config
						)
						Profile.verbosePrint("");
						pf.output(
							outputType = outputType,
							directory = outputDir,
							style = style,
							color = color,
							parallel = parallel
						)
					except Exception as e:
						Profile.verbosePrint("=> an error occured: {0} of type {1}".format(e, type(e)))
						Profile.verbosePrint(traceback.format_exc())
				except:
					Profile.verbosePrint("could not read {0}".format(file))
				Profile.verbosePrint("\n")
			else:
				Profile.verbosePrint("skipping {0} as it does not match filePattern".format(file))
		if not recursive:
			break
	print("Done")
