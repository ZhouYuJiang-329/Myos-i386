#include "mem_detect.h"
#include "serial.h"

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

// 获取总可用内存大小（字节）
uint32_t get_total_memory_bytes(void) {
    uint64_t total = 0;

    for (uint32_t i = 0; i < g_memory_map->count; i++) {
        if (g_memory_map->entries[i].type == E820_USABLE) {
            total += g_memory_map->entries[i].length;
        }
    }

    return (uint32_t)total;
}

// 获取总内存大小（MB）
uint32_t get_total_memory_mb(void) {
    return get_total_memory_bytes() / (1024 * 1024);
}
