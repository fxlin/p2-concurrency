#!/usr/bin/env python

'''
prerequisite: pip3 install bokeh

this script expects a list of log files, the names of which are defined in @progs below.

This script parses the log files and produces a series of plots in HTML: res-0.html, res-1.html, ...
Each HTML includes additional data lines from a new log file.

Example command:
$ ls *.txt
trace-pmla.txt  trace-pml.txt  trace-pm.txt  trace-p.txt  trace-.txt

$ ~/p2-concurrency/scripts/plot.py

$ ls *.html
res-0.html  res-2.html  res-4.html  
res-1.html  res-3.html  res.html

'''

from bokeh.plotting import figure, show, output_file
#from bokeh.io import output_notebook
from bokeh.io import export_png
from bokeh.layouts import row, column

MAX_CORE = 9 # we will not plot anything beyond this

# return: a list of dicts
def parse_trace(f):
	print(f)
	f=open(f)
	lines=f.readlines()
	res = []

	for l in lines:
		if not l.startswith('test='):
			continue
		ldict = {}
		l=l.replace('\n', '')
		tokens=l.split(' ')
		for t in tokens:
			tt = t.split('=')
			ldict[tt[0]] = tt[1]
		res.append(ldict)
	return res

# the scalability plot. there's only one

pp = figure(title=f"1M iterations")
pp.xaxis.axis_label = 'threadNum'
pp.yaxis.axis_label = 'tput (M ops/sec)'

pp_colors = ['red', 'darksalmon', 'green', 'blue', 'darkblue', 'dodgerblue']    # for multiple exps

# the list of log files to parse

'''
progs = [
	"trace-biglock",
	"trace-malloc",
	"trace-nomalloc-nosteal",
	"trace-nomalloc-steal",
	"trace-nomalloc-steal-padding"
]
'''

progs = [
	"trace-",
	"trace-p",
	"trace-pm",
	"trace-pml",
	"trace-pmla"
]


# corresponding to the progs above
legend_names = [
	"baseline (biglock)",
	"-p (partlists)",
	"-pm (partlists,nomalloc)",
	"-pml (partlists,nomalloc,nostraggler)",
	"-pmla (partlists,nomalloc,nostraggler,nofalsesharing)"
]

for progid, prog in enumerate(progs):
	# for perf
	tr = []
	tput = []

	res = parse_trace(f"{prog}.txt")

	for idx, exp in enumerate(res):
		if exp == []:
			continue

		expname = exp['test']

		threadNum = int(exp['threadNum'])
		if threadNum >= MAX_CORE:
			continue
		#iterations = int(l['iterations'])
		#t = float(l['numOperation']) / float(l['runTime(ns)']) * 1e3  # Mops/sec
		t = float(exp['tput(Mops)'])

		tr.append(threadNum)
		tput.append(t)
	# perf plot: add line
	pp.line(tr, tput, line_color=pp_colors[progid], legend_label=f'{legend_names[progid]}')
	pp.circle(tr, tput, fill_alpha=0.6, line_color='black', fill_color='black')

	#show(column(row(ps), pp))
	
	output_file(f"res-{progid}.html", title="res")
	#export_png(pp, filename="res.png")

	# final touch...
	pp.legend.location = "top_left"

	show(pp)

output_file("res.html", title="res")
#export_png(pp, filename="res.png")

# final touch...
pp.legend.location = "top_left"
pp.legend.click_policy="hide"

show(pp)
