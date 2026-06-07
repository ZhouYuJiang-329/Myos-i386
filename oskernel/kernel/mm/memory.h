#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "../lib/bitmap.h"


#define PG_SIZE 4096    //一页的大小
#define MEM_BITMAP_BASE 0xc009a000  //这个地址是位图的起始地址，1MB内存布局中，9FBFF是最大一段可用区域的边界，而我们计划这个可用空间最后的位置将来用来
        //放PCB，而PCB占用内存是一个自然页，所以起始地址必须是0xxxx000这种形式，离0x9fbff最近的符合这个形式的地址是0x9f000。我们又为了将来可能的拓展，
        // 所以让位图可以支持管理512MB的内存空间，所以预留位图大小为16KB，也就是4页，所以选择0x9a000作为位图的起始地址

//定义内核堆区起始地址，堆区就是用来进行动态内存分配的地方，咱们的系统内核运行在c00000000开始的1MB虚拟地址空间，所以自然要跨过这个空间，
//堆区的起始地址并没有跨过256个页表，没关系，反正使用虚拟地址最终都会被我们的页表转换为物理地址，我们建立物理映射的时候，跳过256个页表就行了
#define K_HEAP_START 0xc0100000
// 物理内存映射表存放在物理地址 0x1100 (由 setup.asm 填充)
#define MEMORY_MAP_ADDR 0x1100

//核心数据结构，虚拟内存池，有一个位图与其管理的起始虚拟地址
struct virtual_addr {
   struct bitmap vaddr_bitmap;      // 虚拟地址用到的位图结构 
   uint32_t vaddr_start;            // 虚拟地址起始地址
};
/* 核心数据结构，物理内存池， 生成两个实例用于管理内核物理内存池和用户物理内存池 */
struct pool {
   struct bitmap pool_bitmap;	 // 本内存池用到的位图结构,用于管理物理内存
   uint32_t phy_addr_start;	     // 本内存池所管理物理内存的起始地址
   uint32_t pool_size;		    // 本内存池字节容量
};

extern struct pool kernel_pool, user_pool;
void mem_init(void);
#endif
