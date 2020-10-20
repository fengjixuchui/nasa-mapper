#include "../mem_ctx/mem_ctx.hpp"
#include "../pe_image/pe_image.h"
#include "../direct.h"

#define PML4_MAP_INDEX 70
namespace physmeme
{
	class mapper_ctx
	{
	public:
		explicit mapper_ctx
		(
			physmeme::mem_ctx& map_into, 
			physmeme::mem_ctx& map_from
		);
		std::pair<void*, void*> map(std::vector<std::uint8_t>& raw_image);
		bool call_entry(void* drv_entry, void** hook_handler) const;
	private:
		std::pair<void*, void*> allocate_driver(std::vector<std::uint8_t>& raw_image);
		void make_kernel_access(void* drv_base);
		physmeme::mem_ctx map_into;
		physmeme::mem_ctx map_from;
	};
}