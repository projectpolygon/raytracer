"""
Splits an image up into slabs, splits them across slaves using hypercube,
then recombines the final image after all tasks have been completed
"""

from os import path
from sys import exit as sys_exit
from threading import Thread
from time import sleep
from typing import List
from json import dumps as json_dumps, loads as json_loads
from pathlib import Path

from common.task import Task
from master.master import HyperMaster, JobInfo
from master_app_ex.create_slabs import create_slabs
from master_app_ex.combine import combine_slabs

def byID(task : Task):
	return task.task_id

image_width = 1080
image_height = 720
slab_dim = 100

if __name__ == "__main__":
    # Step 1: Initialise HyperMaster
	master: HyperMaster = HyperMaster()
	
	# Step 2: Initialise Job
	job: JobInfo = JobInfo()
	job.job_path = f'{path.dirname(path.abspath(__file__))}/job'
	job.file_names = ["render.sh"]
	master.init_job(job)
	
	# Step 3: Create a working directory to save intermediate images
	cwd = str(Path.cwd().resolve())
	working_dir = cwd + f'/working_dir/{job.job_id}'
	print(working_dir)
	Path.mkdir(Path(working_dir), parents=True, exist_ok=True)

	# Step 4: Break the scene up into a set of slabs
	slab_coordinates = create_slabs(image_width, image_height, slab_dim)

	# Step 5: Convert each "slab" into a task
	task_list : List = []
	with open(job.job_path + "/scene.json", 'r') as json_file:
		parse_json = json_loads(json_file.read())
		print(parse_json["image_height"])
		PROGRAM = "./render.sh"
		i = 0
		for slab in slab_coordinates:
			parse_json["slab_startx"] = slab[0][0]
			parse_json["slab_endx"] = slab[1][0]
			parse_json["slab_starty"] = slab[0][1]
			parse_json["slab_endy"] = slab[1][1]
			parse_json["output_file"] = "job/" + str(master.job.job_id) + f"/output_{i}.bmp"

			ARGS = [f'payload_{i}.txt', f'output_{i}.txt']
			PAYLOAD = str.encode(json_dumps(parse_json))

			task : Task = Task(i, PROGRAM, ARGS, PAYLOAD, \
				f"output_{i}.bmp", f"payload_{i}.txt")

			task_list.append(task)
			i += 1

		
    # Step 6: Load Tasks
	master.load_tasks(task_list)

	# Step 7: Start Master Server
	master_thread = Thread(name='hypermaster_server_thread', target=master.start_server)
	master_thread.setDaemon(True)
	master_thread.start()

	try:
		# Step 8: Wait for job to be completed
		while not master.is_job_done():
			master.print_status()
			sleep(10)

        # Step 9: Sort the completed tasks
		completed_tasks: List[Task] = master.get_completed_tasks()
		completed_tasks.sort(key=byID)

		# Step 10: Create image files for the tasks
		i = 0
		task_filenames = []
		for task in completed_tasks:
			output_filename = working_dir + "/" + f"output_{i}.bmp"
			i += 1
			with open(output_filename, "wb") as output:
				output.write(task.payload)
				task_filenames.append(output_filename)
				output.flush()

		# Step 11: Combine the images using slab recombination
		combine_slabs(task_filenames, image_width, image_height)

		sys_exit(0)

	except KeyboardInterrupt:
		# call master.exit
		print("exiting")
		sys_exit(0)
