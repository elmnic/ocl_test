typedef struct Node {
	struct Node* parent;
	int cost;
	int position[2];
} Node;

//TODO: Skapa en vektor av noder som sedan används som en prioritetskö, dock genom att söka igenom hela listan varje gång istället.

//TODO: Söka upp hur allokera minne för enskild WORK ITEM.
//Alternativt lägga till parameter för maximal arraystorlek per WORK ITEM som de sedan har tillgång till i en gemensam array

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
	global int *map, 
	global int *inCol, 
	global int *inRow, 
	global int *output,
	global int *agents,
	global Node *openList,
	global Node *closedList,
	global int *maxArraySize)
{
	int sum = 0;
	int idx = get_global_id(0);
	int col = (*inCol);
	int row = (*inRow);
	int listIdx = idx * (*maxArraySize); // TODO: Kontrollera index för gemensam array


	/*
	// Simple test
	for (int i = 0; i < col; i++) 
	{
		sum += map[idx * row + i];
	}
	output[idx] = sum;
	*/

	// Start and end positions retrieved from agent-vector
	int2 start = { agents[idx * 2], agents[idx * 2 + 1]};
	int2 target = { 14, 0 }; // Top right

	// Heuristic per agent
	int h = heuristic(start, target, col, map);
	//printf("Heuristic: %i", h);

	openList[listIdx].parent = 5;
	openList[listIdx].cost = idx;
	openList[listIdx].position[0] = idx;
	openList[listIdx].position[1] = idx + 1;

	printf("Parent: %#010x", openList[listIdx].parent);
	printf("Cost: %i", openList[listIdx].cost);
	printf("Position: (%i, %i)", openList[listIdx].position[0], openList[listIdx].position[1]);


 };