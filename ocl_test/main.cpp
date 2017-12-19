#include <iostream>
#include <CL/cl2.hpp>
#include <fstream>
#include "Map.h"
#include "Agents.h"


typedef struct Node
{
	struct Node* mParent;
	int cost;
	int position[2];
} Node;



int convertToString(const char *filename, std::string& s)
{
	size_t size;
	char*  str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if (f.is_open())
	{
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size + 1];
		if (!str)
		{
			f.close();
			return 0;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
		s = str;
		delete[] str;
		return 0;
	}
	std::cout << "Error: failed to open file\n:" << filename << std::endl;
	return -1;
}

std::vector<int> to1D(std::vector<std::vector<int>> vector)
{
	int rows = vector.size();
	int cols = vector[0].size();

	std::vector<int> _1Dmap(rows * cols);

	// Convert 2D map into 1D map
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			_1Dmap[i * cols + j] = vector[i][j];
		}
	}
	return _1Dmap;
}

int main()
{
	Map map;
	Agents agents;
	cl_int status;

	// Get platforms
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0)
	{
		std::cout << "No platforms found.";
		exit(1);
	}

	// Use first platform as default
	cl::Platform default_platform = all_platforms[0];
	std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";
	std::cout << "Vendor: " << default_platform.getInfo<CL_PLATFORM_VENDOR>() << "\n";
	std::cout << "Version: " << default_platform.getInfo<CL_PLATFORM_VERSION>() << "\n";
	
	// Get devices 
	std::vector<cl::Device> all_devices;
	default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	

	if (all_devices.size() == 0)
	{
		std::cout << "No devices found.";
		exit(1);
	}

	// Use first device as default
	cl::Device default_device = all_devices[0];
	std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";
	std::cout << "Global mem: " << default_device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>() << "\n";
	std::cout << "Local mem: " << default_device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() << "\n";
	std::cout << "Max comp units: " << default_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << "\n";
	std::cout << "Max work group: " << default_device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << "\n";
	std::cout << "Clock freq: " << default_device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << "\n";
	std::cout << "Max mem alloc: " << default_device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() << "\n";

	// Create context using default device
	cl::Context context(default_device);

	// Create kernel source code
	cl::Program::Sources sources;
	std::string kernel_code;

	// Retrieve kernel code from external file
	convertToString("kernel.cl", kernel_code);

	// Add kernel string to sources
	sources.push_back({ kernel_code.c_str(), kernel_code.length() });

	// Create program
	cl::Program program(context, sources);
	if (program.build({ default_device }) != CL_SUCCESS)
	{
		std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
		system("pause");
		exit(1);
	}

	// Declare variables
	int cols = map.getWidth();
	int rows = map.getHeight();
	std::vector<int> output(agents.nrOfAgents());
	int maxArraySize = cols * rows;

	// Convert 2D map into 1D map
	std::vector<int> _1Dmap = to1D(map.mMap);
	std::vector<int> _1Dagents = to1D(agents.agent_positions);

	// Buffers
	cl::Buffer buffer_map(context,        CL_MEM_READ_ONLY , sizeof(int) * _1Dmap.size());
	cl::Buffer buffer_col(context,        CL_MEM_READ_ONLY , sizeof(int));
	cl::Buffer buffer_row(context,        CL_MEM_READ_ONLY , sizeof(int));
	cl::Buffer buffer_output(context,     CL_MEM_WRITE_ONLY, sizeof(int) * output.size());
	cl::Buffer buffer_agents(context,     CL_MEM_READ_ONLY,  sizeof(int) * _1Dagents.size());
	cl::Buffer buffer_openList(context,   CL_MEM_READ_WRITE, sizeof(Node));
	cl::Buffer buffer_closedList(context, CL_MEM_READ_WRITE, sizeof(Node));
	cl::Buffer buffer_maxArrSize(context, CL_MEM_READ_ONLY, sizeof(int));

	// Write input buffers to device
	cl::CommandQueue queue(context, default_device);
	status = queue.enqueueWriteBuffer(buffer_map,        CL_TRUE, 0, sizeof(int) * _1Dmap.size(), _1Dmap.data()); // Input
	status = queue.enqueueWriteBuffer(buffer_col,        CL_TRUE, 0, sizeof(int),                        &cols); // Width
	status = queue.enqueueWriteBuffer(buffer_row,        CL_TRUE, 0, sizeof(int),                        &rows); // Heigh
	status = queue.enqueueWriteBuffer(buffer_agents,     CL_TRUE, 0, sizeof(int) * _1Dagents.size(), _1Dagents.data()); // Agent start positions
	status = queue.enqueueWriteBuffer(buffer_openList,   CL_TRUE, 0, sizeof(Node) * maxArraySize * agents.nrOfAgents(), NULL); // Memory for openList
	status = queue.enqueueWriteBuffer(buffer_closedList, CL_TRUE, 0, sizeof(Node) * maxArraySize * agents.nrOfAgents(), NULL); // Memory for closedList
	status = queue.enqueueWriteBuffer(buffer_maxArrSize, CL_TRUE, 0, sizeof(int),                &maxArraySize); // Max space per WI in local array per WG


	std::cout << "Status enq Node: " << status << "\n";
	std::cout << "Size of Node * maxSize * WI: " << sizeof(Node) * maxArraySize * agents.nrOfAgents() << "\n";

	// Set arguments
	cl::Kernel find_path(program, "find_path");
	find_path.setArg(0, buffer_map);
	find_path.setArg(1, buffer_col);
	find_path.setArg(2, buffer_row);
	find_path.setArg(3, buffer_output);
	find_path.setArg(4, buffer_agents);
	find_path.setArg(5, buffer_openList);
	find_path.setArg(6, buffer_closedList);
	find_path.setArg(7, buffer_maxArrSize);

	// Set work items and execute
	status = queue.enqueueNDRangeKernel(find_path, 0, cl::NDRange(agents.nrOfAgents()));
	status = queue.finish();
	std::cout << "Status finish: " << status << "\n";

	// Read output buffer from device
	status = queue.enqueueReadBuffer(buffer_output, CL_TRUE, 0, sizeof(int) * output.size(), output.data()); // Output

	std::cout << "Status read: " << status << "\n";

	/*for (int i = 0; i < rows; i++)
	{
		std::cout << output[i] << " ";
	}*/


	system("pause");
	return 0;
}