#ifndef MEM_DETECT_H
#define MEM_DETECT_H

#include <stdint.h>

// 内存区域类型
#define E820_USABLE     1   // 可用内存
#define E820_RESERVED   2   // 保留（不可用）
#define E820_ACPI       3   // ACPI 可回收
#define E820_NVS        4   // ACPI NVS
#define E820_UNUSABLE   5   // 不可用（损坏）

// E820 内存映射条目结构（20字节，标准 ARDS）
typedef struct {
    uint64_t base;      // 物理基地址
    uint64_t length;    // 长度
    uint32_t type;      // 类型
} __attribute__((packed)) e820_entry_t;

// 内存映射表（最多支持32个条目）
#define MAX_E820_ENTRIES 32

typedef struct {
    uint32_t count;                     // 条目数量
    e820_entry_t entries[MAX_E820_ENTRIES];  // 条目数组
} memory_map_t;

// 内存映射表存放在物理地址 0x1100 (由 setup.asm 填充)
#define MEMORY_MAP_ADDR 0x1100
#define g_memory_map ((memory_map_t*)MEMORY_MAP_ADDR)

// 函数声明
void print_memory_map(void);
uint32_t get_total_memory_bytes(void);
uint32_t get_total_memory_mb(void);

#endif // MEM_DETECT_H
