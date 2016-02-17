#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x135dd1a3, __VMLINUX_SYMBOL_STR(module_layout) },
	{        0, __VMLINUX_SYMBOL_STR(sys_close) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0xbcf555af, __VMLINUX_SYMBOL_STR(init_task) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0xd1ec4656, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x6e0c8b24, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x4c0550b, __VMLINUX_SYMBOL_STR(pv_cpu_ops) },
	{ 0xb4390f9a, __VMLINUX_SYMBOL_STR(mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "24843877B0BFD287DC05F7E");
