void kernel find_path(
	global int *_map, 
	global int *_width, 
	global int *_height, 
	global int *_output)
{
	int sum = 0;
	const int idx = get_global_id(0);
	const int width = (*_width);
	const int height = (*_height);

	for (int i = 0; i < width; i++) 
	{
		sum += _map[idx * height + i];
	}
	_output[idx] = sum;

 };