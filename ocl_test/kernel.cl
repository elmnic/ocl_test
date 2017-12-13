/*struct Node {
	Node* mParent;
	int cost;
	int2 position;
} Node;*/

int heuristic(
	int2 start, 
	int2 target, 
	int columns, 
	global int *map) 
{
	int xStart = start[0]; 
	int yStart = start[1];
	
	int xTarget = target[0]; 
	int yTarget = target[1];

	// Retrieve index in one-dimensional array
	int startIdx = yStart * columns + xStart;
	int targetIdx = yTarget * columns + xTarget;

	printf("Start at: (%i,%i). : %i", xStart, yStart, map[startIdx]);
	printf("Target at: (%i,%i). : %i", xTarget, yTarget, map[targetIdx]);

	// Manhattan distance between start and target
	int manhattan = abs(xStart - xTarget) + abs(yStart - yTarget);
	return manhattan;
}

void kernel find_path(
	global int *_map, 
	global int *inCol, 
	global int *inRow, 
	global int *_output/*,
	global int *agents*/)
{
	int sum = 0;
	int idx = get_global_id(0);
	int col = (*inCol);
	int row = (*inRow);

	for (int i = 0; i < col; i++) 
	{
		if (idx == 0) 
		{
			printf("%i", _map[idx * row + i]);
		}
		sum += _map[idx * row + i];
	}
	_output[idx] = sum;

	// Start and end positions
	int2 start = { col / 2 + 1, 0 };
	int2 target = { 0, 6 };

	int h = heuristic(start, target, col, _map);
	printf("Heuristic: %i", h);

 };