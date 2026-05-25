#include "mem_detect.h"
#include "../drivers/serial.h"

// 物理内存管理器实例
static pmm_t pmm;

// 页大小
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

// 将地址转换为页帧号
#define ADDR_TO_PFN(addr) ((uint32_t)(addr) >> PAGE_SHIFT)
#define PFN_TO_ADDR(pfn) ((void*)((uint32_t)(pfn) << PAGE_SHIFT))

// 打印内存映射表
void print_memory_map(void) {
    serial_putline("\n========== Memory Map ==========");
    serial_printf("Total entries: %d\n", g_memory_map->count);
    serial_putline("--------------------------------");
    
    for (uint32_t i = 0; i < g_memory_map->count; i++) {
        e820_entry_t *entry = &g_memory_map->entries[i];
        const char *type_str;
        
        switch (entry->type) {
            case E820_USABLE:   type_str = "Usable"; break;
            case E820_RESERVED: type_str = "Reserved"; break;
            case E820_ACPI:     type_str = "ACPI"; break;
            case E820_NVS:      type_str = "NVS"; break;
            case E820_UNUSABLE: type_str = "Unusable"; break;
            default:            type_str = "Unknown"; break;
        }
        
        serial_printf("[%d] 0x%x%08x - 0x%x%08x : %d MB (%s)\n",
            i,
            (uint32_t)(entry->base >> 32),
            (uint32_t)(entry->base),
            (uint32_t)((entry->base + entry->length) >> 32),
            (uint32_t)(entry->base + entry->length),
            (uint32_t)(entry->length / (1024 * 1024)),
            type_str
        );
    }
    serial_putline("================================\n");
}

// 获取总内存大小（MB）
uint32_t get_total_memory_mb(void) {
    uint64_t total = 0;
    
    for (uint32_t i = 0; i < g_memory_map->count; i++) {
        if (g_memory_map->entries[i].type == E820_USABLE) {
            total += g_memory_map->entries[i].length;
        }
    }
    
    return (uint32_t)(total / (1024 * 1024));
}

// 初始化物理内存管理器
void pmm_init(void) {
    serial_putline("Initializing Physical Memory Manager...");
    
    // 找到最大的可用内存区域作为管理范围
    uint64_t max_addr = 0;
    uint64_t usable_memory = 0;
    
    for (uint32_t i = 0; i < g_memory_map->count; i++) {
        e820_entry_t *entry = &g_memory_map->entries[i];
        if (entry->type == E820_USABLE) {
            usable_memory += entry->length;
            uint64_t end = entry->base + entry->length;
            if (end > max_addr) {
                max_addr = end;
            }
        }
    }
    
    // 从 1MB 开始管理（低于1MB的内存有特殊用途）
    pmm.start_pfn = ADDR_TO_PFN(0x100000);  // 256
    uint32_t max_pfn = ADDR_TO_PFN(max_addr);
    pmm.total_pages = max_pfn - pmm.start_pfn;
    
    // 计算位图大小（每页1bit）
    pmm.bitmap_size = (pmm.total_pages + 7) / 8;
    
    // 将位图放在内核后面（假设内核在 0x1200，最大 60KB）
    // 使用 0x20000 (128KB) 作为位图起始地址
    pmm.bitmap = (uint32_t*)0x20000;
    
    // 清空位图（0 = 空闲）
    for (uint32_t i = 0; i < pmm.bitmap_size / 4; i++) {
        pmm.bitmap[i] = 0;
    }
    
    // 标记已使用的内存（内核区域 0x1200 - 0x20000）
    uint32_t kernel_start_pfn = ADDR_TO_PFN(0x1200);
    uint32_t kernel_end_pfn = ADDR_TO_PFN(0x20000 + pmm.bitmap_size);

    for (uint32_t pfn = kernel_start_pfn; pfn < kernel_end_pfn && pfn < max_pfn; pfn++) {
        if (pfn >= pmm.start_pfn) {
            uint32_t idx = pfn - pmm.start_pfn;
            pmm.bitmap[idx / 32] |= (1 << (idx % 32));
        }
    }

    pmm.used_pages = (kernel_end_pfn > pmm.start_pfn) ? (kernel_end_pfn - pmm.start_pfn) : 0;
    pmm.free_pages = pmm.total_pages - pmm.used_pages;
    
    serial_printf("PMM initialized:\n");
    serial_printf("  Total memory: %d MB\n", get_total_memory_mb());
    serial_printf("  Total pages: %d (%d MB)\n", pmm.total_pages, pmm.total_pages * 4 / 1024);
    serial_printf("  Free pages: %d (%d MB)\n", pmm.free_pages, pmm.free_pages * 4 / 1024);
    serial_printf("  Used pages: %d (%d MB)\n", pmm.used_pages, pmm.used_pages * 4 / 1024);
    serial_printf("  Bitmap at: 0x%x, size: %d bytes\n", pmm.bitmap, pmm.bitmap_size);
}

// 分配一页物理内存
void* pmm_alloc_page(void) {
    if (pmm.free_pages == 0) {
        return 0;  // 内存不足
    }
    
    // 遍历位图找空闲页
    for (uint32_t i = 0; i < pmm.bitmap_size / 4; i++) {
        if (pmm.bitmap[i] != 0xFFFFFFFF) {  // 不是全满
            // 找第一个0位
            for (uint32_t j = 0; j < 32; j++) {
                if (!(pmm.bitmap[i] & (1 << j))) {
                    uint32_t pfn = pmm.start_pfn + i * 32 + j;
                    pmm.bitmap[i] |= (1 << j);  // 标记为已用
                    pmm.free_pages--;
                    pmm.used_pages++;
                    return PFN_TO_ADDR(pfn);
                }
            }
        }
    }
    
    return 0;  // 没找到
}

// 释放一页物理内存
void pmm_free_page(void* page) {
    if (!page) return;
    
    uint32_t pfn = ADDR_TO_PFN((uint32_t)page);
    
    // 检查是否在管理范围内
    if (pfn < pmm.start_pfn || pfn >= pmm.start_pfn + pmm.total_pages) {
        return;  // 无效地址
    }
    
    uint32_t idx = pfn - pmm.start_pfn;
    uint32_t i = idx / 32;
    uint32_t j = idx % 32;
    
    // 检查是否已分配
    if (!(pmm.bitmap[i] & (1 << j))) {
        return;  // 已经是空闲的
    }
    
    pmm.bitmap[i] &= ~(1 << j);  // 标记为空闲
    pmm.free_pages++;
    pmm.used_pages--;
}

// 获取空闲页数
uint32_t pmm_get_free_pages(void) {
    return pmm.free_pages;
}
