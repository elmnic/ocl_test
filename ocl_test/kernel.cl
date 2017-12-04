void kernel find_path(
	global int *_map, 
	global int *_width, 
	global int *_height, 
	global int *_output)
{
	int sum = 0;
	int idx = get_global_id(0);
	int width = (*_width);
	int height = (*_height);

	//printf("value: %d", _map[14]);
	for (int i = 0; i < height; i++) 
	{
		printf("%i", _map[idx * width + height]);
		sum += _map[idx * width + height];
	}
 };