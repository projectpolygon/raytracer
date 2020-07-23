"""
An example app that imports master and prepares the tasks
"""

from os import path
from sys import exit as sys_exit
from threading import Thread
from time import sleep
from typing import List

from common.task import Task
from master.master import HyperMaster, JobInfo

def create_slabs(image_width : int, image_height : int, slab_dims : int) -> List: 

	slabs : List = []

	x = 0
	x_dim = slab_dims
	y = 0
	y_dim = slab_dims

	while y < image_height:
		while x < image_width:
			new_slab : List = [[0,0],[0,0]]
			
			# x-coordinates
			new_slab[0][0] = x
			new_slab[1][0] = x + x_dim
			x += slab_dims

			new_slab[0][1] = y
			new_slab[1][1] = y + y_dim

			slabs.append(new_slab)
		# Increment y coordinate
		x = 0
		y += y_dim 
	
	# correct out-of-bounds slabs
	for slab in slabs:
		if slab[1][0] > image_width:
			slab[1][0] = image_width
		if slab[1][1] > image_height:
			slab[1][1] = image_height
	
	return slabs

if __name__ == "__main__":
	print(create_slabs(1080, 720, 50)) 
