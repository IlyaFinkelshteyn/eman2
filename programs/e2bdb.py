#!/usr/bin/env python

#
# Author: Steven Ludtke, 11/13/2008 (sludtke@bcm.edu)
# Copyright (c) 2000-2006 Baylor College of Medicine
#
# This software is issued under a joint BSD/GNU license. You may use the
# source code in this file under either license. However, note that the
# complete EMAN2 and SPARX software packages have some GPL dependencies,
# so you are responsible for compliance with the licenses of these packages
# if you opt to use BSD licensing. The warranty disclaimer below holds
# in either instance.
#
# This complete copyright notice must be included in any revised version of the
# source code. Additional authorship citations may be added, but existing
# author citations must be preserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  2111-1307 USA
#
#

# e2bdb.py  11/13/2008 Steven Ludtke
# This program allows manipulation and querying of the local database

from EMAN2 import *
from optparse import OptionParser
from math import *
import time
import os
import sys
import re

def main():
	global debug
	progname = os.path.basename(sys.argv[0])
	usage = """%prog [options] <path or db> ...
	
Various utilities related to BDB databases."""

	parser = OptionParser(usage=usage,version=EMANVERSION)

	parser.add_option("--cleanup","-c",action="store_true",default=False,help="This option will clean up the database cache so files can safely be moved or accessed on another computer via NFS.")
	parser.add_option("--force","-F",action="store_true",default=False,help="This will force an action that would normally fail due to failed checks.")
	parser.add_option("--delete",action="store_true",default=False,help="This will delete (or at least empty) the named database(s)")
	parser.add_option("--all","-a",action="store_true",help="List per-particle info",default=False)
	parser.add_option("--long","-l",action="store_true",help="Long listing",default=False)
	parser.add_option("--short","-s",action="store_true",help="Dense listing of names only",default=False)
	parser.add_option("--filt",type="string",help="Only include dictionary names containing the specified string",default=None)
	parser.add_option("--filtexclude",type="string",help="Exclude dictionary names containing the specified string",default=None)
	parser.add_option("--match",type="string",help="Only include dictionaries matching the provided Python regular expression",default=None)
	parser.add_option("--exclude",type="string",help="The name of a database containing a list of exclusion keys",default=None)

	parser.add_option("--makevstack",type="string",help="Creates a 'virtual' BDB stack with its own metadata, but the binary data taken from the (filtered) list of stacks",default=None)
	parser.add_option("--appendvstack",type="string",help="Appends to/creates a 'virtual' BDB stack with its own metadata, but the binary data taken from the (filtered) list of stacks",default=None)

	(options, args) = parser.parse_args()

	if options.cleanup : 
		db_cleanup(options.force)
		sys.exit(0)

	if options.all : options.long=1
	if len(args)==0 : args.append("bdb:.")
	
	logid=0
	if options.makevstack : 
		logid=E2init(sys.argv)
		vstack=db_open_dict(options.makevstack)
		vstackn=0
	elif options.appendvstack :
		logid=E2init(sys.argv)
		vstack=db_open_dict(options.appendvstack)
		vstackn=len(vstack)
	else : vstack=None
		
	
	for path in args:
		if path.lower()[:4]!="bdb:" : path="bdb:"+path
		if '#' in path :
			if len(args)>1 : print "\n",path,":"
			path,dbs=path.rsplit("#",1)
			path+="#"
			dbs=[dbs]
		else:
			if not '#' in path and path[-1]!='/' : path+='#'			
			if len(args)>1 : print "\n",path[:-1],":"
			dbs=db_list_dicts(path)
			
		
		dbs.sort()
		if options.filt:
			dbs=[db for db in dbs if options.filt in db]
			
		if options.filtexclude:
			dbs=[db for db in dbs if options.filtexclude not in db]

		if options.match!=None:
			dbs=[db for db in dbs if re.match(options.match,db)]
		
		if options.makevstack!=None or options.appendvstack!=None :
			
			for db in dbs:
				dct,keys=db_open_dict(path+db,with_keys=True)
				if dct==vstack : continue
				vals = keys
				if keys == None: vals = range(len(dct))
				for n in vals:
					try: d=dct.get(n,nodata=1).get_attr_dict()
					except:
						print "error reading ",db,n 
						continue
					d["data_path"]=dct.get_data_path(n)
					if d["data_path"]==None :
						print "error with data_path ",db,n
						continue
					vstack[vstackn]=d
					vstackn+=1
					if vstackn%100==0:
						try:
							print "\r  ",vstackn,"     ",
							sys.stdout.flush()
						except: pass	
				print "\r  ",vstackn,"     "

		try: maxname=max([len(s) for s in dbs])
		except: 
			print "Error reading ",path
			continue
			
		# long listing, one db per line
		if options.long :
			width=maxname+3
			fmt="%%-%ds %%-07d %%14s  %%s"%width
			fmt2="%%-%ds (not an image stack)"%width
			total=[0,0]
			for db in dbs:
				dct=db_open_dict(path+db)
				if options.all :
					for i in range(len(dct)):
						im=dct[i]
						print "%d. %d x %d x %d\tA/pix=%1.2f\tM=%1.4f\tS=%1.4f"%(i,im["nx"],im["ny"],im["nz"],im["apix_x"],im["mean"],im["sigma"]),
						try:
							print "\tdf=%1.3f\tB=%1.1f"%(im["ctf"].defocus,im["ctf"].bfactor)
						except: print " "
				
				first=EMData()
				try: 
					first.read_image(path+db,0,True)
					size=first.get_xsize()*first.get_ysize()*first.get_zsize()*len(dct)*4;
					total[0]+=len(dct)
					total[1]+=size
					print fmt%(db,len(dct),"%dx%dx%d"%(first.get_xsize(),first.get_ysize(),first.get_zsize()),human_size(size))
				except:
					print fmt2%db
			print fmt%("TOTAL",total[0],"",human_size(total[1]))

		elif options.short :
			for db in dbs:
				print path+db,
			print " "

		elif not options.makevstack and not options.appendvstack :
			# Nicely formatted 'ls' style display
			cols=int(floor(80.0/(maxname+3)))
			width=80/cols
			rows=int(ceil(float(len(dbs))/cols))
			
			fmt="%%-%ds"%width
			for r in range(rows):
				for c in range(cols):
					try: print fmt%dbs[r+c*rows],
					except: pass
				print " "

		if options.delete :
			if not options.force :
				print "You are requesting to delete the following databases:"
				for db in dbs:
					print db," ",
				if raw_input("\nAre you sure (y/n) ? ")[0].lower()!='y' :
					print "Aborted"
					sys.exit(1)
			
			for db in dbs: db_remove_dict(path+db)
			

	if logid : E2end(logid)

def db_cleanup(force=False):
	"""This is an important utility function to clean up the database environment so databases can safely be moved or used remotely
	from other machines. If working on a cluster, this routine should be called on any machine which has opened a database before that
	database is written to on another machine"""
	
	if(sys.platform == 'win32'):
		print "Database cleanup is not supported on windows machines"
		sys.exit(1)
		
	path="eman2db-%s"%os.getenv("USER","anyone")

	if not force :
		try:
			pipe=os.popen("lsof","r")
			op=[l.split() for l in pipe if path in l]
			ret=pipe.close()
			if ret!=None :
				pipe=os.popen("/usr/sbin/lsof","r")
				op=[l.split() for l in pipe if path in l]
				ret=pipe.close()
				if ret!=None: raise Exception
		except:
			print "Error : could not check for running EMAN2 jobs, please make sure the 'lsof' command is installed and functioning, or insure no EMAN2 commands are running and run e2bdb.py -cF"
			sys.exit(1)
		
		# someone is still using the cache
		if len(op)>0 :
			s=set()
			for i in op:
				s.add(i[1])
		
			print "These processes are actively using the cache. Please exit them and try again :"
			for i in s: 
				try: print os.popen("ps %s"%i,"r").readlines()[-1]
				except: print i
				
			return

	# ok, properly close the cache and delete it
	d=EMAN2DB()
	d.close()		# Properly 'close' the environment before we delete it
	
	os.system("rm -rf /tmp/%s"%path)
	print "Database cache removed. Now safe to access databases from another machine or delete existing databases"
	
	

def human_size(size):
	if size>1000000000: return "%1.2f gb"%(size/1000000000)
	elif size>1000000: return "%1.2f mb"%(size/1000000)
	else: return "%1.2f kb"%(size/1000)
	return str(size)
			
if __name__ == "__main__":
	main()
