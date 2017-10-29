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
	{ 0x5a7ba343, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x9e33c816, __VMLINUX_SYMBOL_STR(cdev_alloc) },
	{ 0xf7e79095, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0x6c4c447, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x44fe75f2, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0xd0ee38b8, __VMLINUX_SYMBOL_STR(schedule_timeout_uninterruptible) },
	{ 0xd8e484f0, __VMLINUX_SYMBOL_STR(register_chrdev_region) },
	{ 0x24b598d6, __VMLINUX_SYMBOL_STR(find_vpid) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xec438af3, __VMLINUX_SYMBOL_STR(pid_task) },
	{ 0x21eba563, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0x9777bbd5, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x9d669763, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x8484b413, __VMLINUX_SYMBOL_STR(send_sig_info) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "704513C7B262847106A21D8");
