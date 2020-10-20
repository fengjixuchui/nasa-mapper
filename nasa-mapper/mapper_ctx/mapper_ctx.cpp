#include "mapper_ctx.hpp"

namespace physmeme
{
	mapper_ctx::mapper_ctx
	(
		physmeme::mem_ctx& map_into,
		physmeme::mem_ctx& map_from
	)
		:
		map_into(map_into),
		map_from(map_from)
	{}

	std::pair<void*, void*> mapper_ctx::map(std::vector<std::uint8_t>& raw_image)
	{
		const auto [drv_alloc, drv_entry_addr] = allocate_driver(raw_image);
		auto [drv_ppml4e, drv_pml4e] = map_from.get_pml4e(drv_alloc);

		//
		// make the pde & pte's containing the driver user supervisor = false...
		//
		make_kernel_access(drv_alloc);

		//
		// removes the kernel memory from runtimebroker.exe
		//
		map_from.set_pml4e(drv_ppml4e, pml4e{ NULL });

		//
		// set new pml4e into specific process.
		//
		drv_pml4e.nx = false;
		drv_pml4e.user_supervisor = false;

		map_into.write_phys
		(
			reinterpret_cast<ppml4e*>(map_into.get_dirbase()) + PML4_MAP_INDEX,
			drv_pml4e
		);

		virt_addr_t new_addr = { reinterpret_cast<void*>(drv_alloc) };
		new_addr.pml4_index = PML4_MAP_INDEX;
		return { new_addr.value, drv_entry_addr };
	}

	bool mapper_ctx::call_entry(void* drv_entry, void** hook_handler) const
	{
		const auto result = map_into.k_ctx->syscall<NTSTATUS(__fastcall*)(void**)>(drv_entry, hook_handler);
		return !result;
	}

	std::pair<void*, void*> mapper_ctx::allocate_driver(std::vector<std::uint8_t>& raw_image)
	{
		const auto _get_module = [&](std::string_view name)
		{
			return util::get_module_base(name.data());
		};

		const auto _get_export_name = [&](const char* base, const char* name)
		{
			return reinterpret_cast<std::uintptr_t>(util::get_module_export(base, name));
		};

		physmeme::pe_image drv_image(raw_image);
		const auto process_handle =
			OpenProcess(
				PROCESS_ALL_ACCESS,
				FALSE,
				map_from.get_pid()
			);

		if (!process_handle)
			return {};

		drv_image.fix_imports(_get_module, _get_export_name);
		drv_image.map();

		const auto drv_alloc_base = 
			reinterpret_cast<std::uintptr_t>(
				direct::alloc_virtual_memory(
					process_handle,
					drv_image.size(),
					PAGE_READWRITE
				));

		if (!drv_alloc_base)
			return {};

		virt_addr_t new_addr = { reinterpret_cast<void*>(drv_alloc_base) };
		new_addr.pml4_index = PML4_MAP_INDEX;
		drv_image.relocate(reinterpret_cast<std::uintptr_t>(new_addr.value));

		//
		// dont write nt headers...
		//
		const bool result = direct::write_virtual_memory
		(
			process_handle,
			reinterpret_cast<void*>((std::uint64_t)drv_alloc_base + drv_image.header_size()),
			reinterpret_cast<void*>((std::uint64_t)drv_image.data() + drv_image.header_size()),
			drv_image.size() - drv_image.header_size()
		);

		if (!CloseHandle(process_handle))
			return { {}, {} };

		return
		{ 
			reinterpret_cast<void*>(drv_alloc_base),
			reinterpret_cast<void*>(drv_image.entry_point() + reinterpret_cast<std::uintptr_t>(new_addr.value)) 
		};
	}

	void mapper_ctx::make_kernel_access(void* drv_base)
	{
		const auto [ppdpte, pdpte] = map_from.get_pdpte(drv_base);
		auto ppdpte_phys = reinterpret_cast<void*>((reinterpret_cast<std::uint64_t>(ppdpte) >> 12) << 12); // 0 the last 12 bits...
		auto pdpt_mapping = reinterpret_cast<::ppdpte>(map_from.set_page(ppdpte_phys));

		// set pdptes to CPL0 access only and executable...
		for (auto pdpt_idx = 0u; pdpt_idx < 512; ++pdpt_idx)
		{
			if (pdpt_mapping[pdpt_idx].present)
			{
				pdpt_mapping[pdpt_idx].user_supervisor = false;
				pdpt_mapping[pdpt_idx].nx = false;

				auto pd_mapping = reinterpret_cast<ppde>(
					map_from.set_page(reinterpret_cast<void*>(
						pdpt_mapping[pdpt_idx].pfn << 12)));

				// set pdes to CPL0 access only and executable...
				for (auto pd_idx = 0u; pd_idx < 512; ++pd_idx)
				{
					if (pd_mapping[pd_idx].present)
					{
						pd_mapping[pd_idx].user_supervisor = false;
						pd_mapping[pd_idx].nx = false;

						auto pt_mapping = reinterpret_cast<ppte>(
							map_from.set_page(reinterpret_cast<void*>(
								pd_mapping[pd_idx].pfn << 12)));

						// set ptes to CPL0 access only and executable...
						for (auto pt_idx = 0u; pt_idx < 512; ++pt_idx)
						{
							if (pt_mapping[pt_idx].present)
							{
								pt_mapping[pt_idx].user_supervisor = false;
								pt_mapping[pt_idx].nx = false;
							}
						}

						// set page back to pd...
						pd_mapping = reinterpret_cast<ppde>(
							map_from.set_page(reinterpret_cast<void*>(
								pdpt_mapping[pdpt_idx].pfn << 12)));
					}
				}

				// set page back to pdpt...
				pdpt_mapping = reinterpret_cast<::ppdpte>(
					map_from.set_page(ppdpte_phys));
			}
		}
	}
}