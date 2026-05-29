#ifndef _PAGING_H
#define _PAGING_H

#include <stdint.h>

// 页大小
#define PAGE_SIZE 4096

// 页目录和页表的条目数
#define PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024

// 固定物理地址（选择不与PMM位图冲突的位置）
#define PAGE_DIR_PHYS 0x30000    // 页目录物理地址（4KB对齐）
#define PAGE_TABLE0_PHYS 0x31000 // 页表0物理地址（4KB对齐）

// 页目录项（PDE）
typedef struct {
    uint32_t present : 1;         // 存在位
    uint32_t writable : 1;        // 读写位（1=可写）
    uint32_t user : 1;           // 用户/超级visor位
    uint32_t write_through : 1;  // 写通位
    uint32_t cache_disable : 1;  // 缓存禁用位
    uint32_t accessed : 1;       // 访问位
    uint32_t dirty : 1;          // 脏位（对PDE未使用）
    uint32_t page_size : 1;      // 页大小（0=4KB, 1=4MB）
    uint32_t global : 1;         // 全局位
    uint32_t available : 3;      // 可用位（软件使用）
    uint32_t page_table_addr : 20;  // 页表物理地址 >> 12
} __attribute__((packed)) pde_t;

// 页表项（PTE）
typedef struct {
    uint32_t present : 1;         // 存在位
    uint32_t writable : 1;        // 读写位（1=可写）
    uint32_t user : 1;           // 用户/超级visor位
    uint32_t write_through : 1;  // 写通位
    uint32_t cache_disable : 1;  // 缓存禁用位
    uint32_t accessed : 1;       // 访问位
    uint32_t dirty : 1;          // 脏位
    uint32_t pat : 1;            // 页属性表位
    uint32_t global : 1;         // 全局位
    uint32_t available : 3;      // 可用位（软件使用）
    uint32_t page_addr : 20;     // 页物理地址 >> 12
} __attribute__((packed)) pte_t;

// 函数声明
void paging_init(void);

#endif // _PAGING_H
