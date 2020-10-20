#include "map_driver.hpp"
#include "mapper_ctx/mapper_ctx.hpp"
#include "kernel_ctx/kernel_ctx.h"

namespace mapper
{
	std::pair<mapper_error, void*> map_driver(std::uint8_t* drv_image, std::size_t image_size, void** entry_data)
	{
		std::vector<std::uint8_t> drv_buffer(drv_image, image_size + drv_image);
		if (!drv_buffer.size())
			return { mapper_error::image_invalid, nullptr };

		if (!physmeme::load_drv())
			return { mapper_error::load_error, nullptr };

		physmeme::kernel_ctx kernel;

		// after we setup the kernel_ctx we dont need any driver loaded anymore...
		if (!physmeme::unload_drv())
			return { mapper_error::unload_error, nullptr };

		// clear piddb cache of the loaded vuln driver...
		if (!kernel.clear_piddb_cache(physmeme::drv_key, util::get_file_header((void*)raw_driver)->TimeDateStamp))
			return { mapper_error::piddb_fail, nullptr };

		// start a runtime broker suspended...
		const auto runtime_broker_pid = util::start_runtime_broker();

		if (!runtime_broker_pid)
			return { mapper_error::failed_to_create_proc, nullptr };	

		physmeme::mem_ctx my_proc(kernel, GetCurrentProcessId());
		physmeme::mem_ctx runtime_broker(kernel, runtime_broker_pid);
		physmeme::mapper_ctx mapper(my_proc, runtime_broker);

		// allocate the driver in the suspended runtime broker and expose it to this process...
		const auto [drv_base, drv_entry] = mapper.map(drv_buffer);
		if (!drv_base || !drv_entry)
			return { mapper_error::init_failed, nullptr };

		mapper.call_entry(drv_entry, entry_data);

		// mem_ctx destructors need to be called before kernel_ctx destructors...
		my_proc.~mem_ctx();
		runtime_broker.~mem_ctx();
		return { mapper_error::error_success, drv_base };
	}
}