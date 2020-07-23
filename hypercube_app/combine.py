import sys
from PIL import Image

def combine_slabs(image_names, image_width, image_height):

	images = []

	for name in image_names:
		images.append(Image.open(name))

	print("IMAGES OPENED")

	new_im = Image.new('RGB', (1080, 720))

	x_offset = 0
	y_offset = 720 - 100
	for im in images:
		if x_offset >= 1080:
			y_offset -= im.size[1]
			x_offset = 0
		new_im.paste(im, (x_offset,y_offset))
		x_offset += im.size[0]

	new_im.save('render.jpg')
