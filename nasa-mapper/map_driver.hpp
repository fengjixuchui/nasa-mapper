#pragma once
#include <cstddef>
#include <cstdint>
#include <map>

namespace mapper
{
	enum class mapper_error
	{
		error_success = 0x000,			// everything is good!
		image_invalid = 0x111,			// the driver your trying to map is invalid (are you importing things that arent in ntoskrnl?)
		load_error = 0x222,				// unable to load signed driver into the kernel (are you running as admin?)
		unload_error = 0x333,			// unable to unload signed driver from kernel (are all handles to this driver closes?)
		piddb_fail = 0x444,				// piddb cache clearing failed... (are you using this code below windows 10?)
		init_failed = 0x555,			// setting up library dependancies failed!
		failed_to_create_proc = 0x777	// was unable to create a new process to inject driver into! (RuntimeBroker.exe)
	};

	/// <summary>
	/// map a driver only into your current process...
	/// </summary>
	/// <param name="drv_image">base address of driver buffer</param>
	/// <param name="image_size">size of the driver buffer</param>
	/// <param name="entry_data">data to be sent to the entry point of the driver...</param>
	/// <returns>status of the driver being mapped, and base address of the driver...</returns>
	std::pair<mapper_error, void*> map_driver(std::uint8_t* drv_image, std::size_t image_size, void** entry_data);
}