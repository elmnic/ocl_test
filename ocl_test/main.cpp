#include <iostream>
#include <CL/cl2.hpp>
#include <fstream>
#include "Map.h"


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

int main()
{
	Map map;
	cl_int status;

	// Get platforms
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0)
	{
		std::cout << "No platforms found.";
		exit(1);
	}

	// Default platform
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

	// Default devices
	cl::Device default_device = all_devices[0];
	std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

	// Create context
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
	std::vector<std::vector<int>> _map = map.mMap;
	int _width = map.getWidth();
	int _height = map.getHeight();
	std::vector<int> _1Dmap(_height * _width);

	// Convert 2D map into 1D map
	for (int i = 0; i < _height; i++)
	{
		for (int j = 0; j < _width; j++)
		{
			_1Dmap[i * _width + j] = _map[i][j];
		}
	}

	cl::vector<int> _output(_width);
	std::cout << "map size: " << sizeof(_map) << "\n1d size: " << sizeof(_1Dmap) << " -- " << _1Dmap.size() << "\nint size: " << sizeof(int) << "\n";

	// Buffers
	cl::Buffer buffer_map(context,    CL_MEM_READ_ONLY , sizeof(int) * _1Dmap.size());
	cl::Buffer buffer_width(context,  CL_MEM_READ_ONLY , sizeof(int));
	cl::Buffer buffer_height(context, CL_MEM_READ_ONLY , sizeof(int));
	cl::Buffer buffer_output(context, CL_MEM_WRITE_ONLY, sizeof(int) * _output.size());

	// Write input buffers to device
	cl::CommandQueue queue(context, default_device);
	status = queue.enqueueWriteBuffer(buffer_map,    CL_TRUE, 0, sizeof(int) * _1Dmap.size(), _1Dmap.data()); // Input
	status = queue.enqueueWriteBuffer(buffer_width,  CL_TRUE, 0, sizeof(int),                 &_width); // Width
	status = queue.enqueueWriteBuffer(buffer_height, CL_TRUE, 0, sizeof(int),                 &_height); // Heigh

	// Set arguments
	cl::Kernel find_path(program, "find_path");
	find_path.setArg(0, buffer_map);
	find_path.setArg(1, buffer_width);
	find_path.setArg(2, buffer_height);
	find_path.setArg(3, buffer_output);

	// Set work items and execute
	status = queue.enqueueNDRangeKernel(find_path, 0, cl::NDRange(_output.size()));
	status = queue.finish();
	std::cout << "Status finish: " << status << "\n";

	// Read output buffer from device
	status = queue.enqueueReadBuffer(buffer_output, CL_TRUE, 0, sizeof(int) * _output.size(), _output.data(), NULL, NULL); // Output

	std::cout << "Status read: " << status << "\n";
	for (int i = 0; i < _height; i++)
	{
		std::cout << _output[i] << " ";
	}


	system("pause");
	return 0;
}