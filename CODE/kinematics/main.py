import pygame
#import pygame.draw_py
import pygame_gui
import numpy as np


# Robot parameters
c0 = np.array([400, 550])
W = 40
H = 40
S = 10

pygame.init()

pygame.display.set_caption('snakebot kinematics demo')
window_surface = pygame.display.set_mode((800, 600))

background = pygame.Surface((800, 600))
background.fill(pygame.Color('#000000'))

manager = pygame_gui.UIManager((800, 600))

clock = pygame.time.Clock()
is_running = True

dispacement_slider = pygame_gui.elements.UIHorizontalSlider(pygame.Rect((0, 0), (200, 40)), start_value=0, value_range=(-10, 10), click_increment=0.1)

def get_rotation_matrix(theta):
	return np.array([[np.cos(theta), -np.sin(theta)], [np.sin(theta), np.cos(theta)]])

def get_center_points(x):
	center_points = [c0]
	left_points = [c0 + np.array([-W, 0])]
	right_points = [c0 + np.array([W, 0])]
	angles = [0]
	#THETA = x / W 
	#THETA = np.sqrt((1 - x**2) / W**2)
	#THETA = (x - 1 + np.sqrt(H**2 + 1 - 2*x)) / W*H
	THETA = x * (np.abs(x) + H) / (W * H)
	print(x, THETA)
	for s in range(S):
		if s < S // 2:
			angles.append(angles[-1] + THETA)
		else:
			angles.append(angles[-1] - THETA)
		center_points.append(center_points[-1] + H * np.matmul(get_rotation_matrix(angles[-1]), np.array([0, -1])))
		left_points.append(center_points[-1] + W * np.matmul(get_rotation_matrix(angles[-1]), np.array([-1, 0])))
		right_points.append(center_points[-1] + W * np.matmul(get_rotation_matrix(angles[-1]), np.array([1, 0])))
	return center_points, left_points, right_points

while is_running:
	time_delta = clock.tick(60)/1000.0
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			is_running = False

		manager.process_events(event)

	manager.update(time_delta)
	window_surface.blit(background, (0, 0))
	manager.draw_ui(window_surface)

	# Draw the body points
	center_points, left_points, right_points = get_center_points(dispacement_slider.get_current_value())
	for point in center_points:
		pygame.draw.circle(window_surface, [0, 255, 0], center=point, radius=5)
	for point in left_points:
		pygame.draw.circle(window_surface, [255, 0, 0], center=point, radius=5)
	for point in right_points:
		pygame.draw.circle(window_surface, [255, 0, 0], center=point, radius=5)
	# Draw the lines: x,y
	mid_point = center_points[S // 2]
	z = mid_point - center_points[0]
	d = np.linalg.norm(z)
	x = [z[0], 0]
	y = [0, z[1]]
	pygame.draw.line(window_surface, [0, 0, 255], start_pos=center_points[0], end_pos=center_points[0] + x)
	pygame.draw.line(window_surface, [0, 0, 255], start_pos=center_points[0] + x, end_pos=center_points[0] + x + y)
	pygame.draw.line(window_surface, [0, 0, 255], start_pos=center_points[0], end_pos=center_points[0] + z)
	#print(f'|x|: {np.linalg.norm(x)}, |y: {np.linalg.norm(y)}, |z: {np.linalg.norm(z)}')

	pygame.display.update()