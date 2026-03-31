#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x2c635209, "module_layout" },
	{ 0xdff7669f, "device_destroy" },
	{ 0x3ee2b36d, "class_destroy" },
	{ 0xcf2a881c, "device_create" },
	{ 0x37ce6741, "cdev_del" },
	{ 0x9b0fb107, "__class_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xabac4112, "cdev_add" },
	{ 0x51b1c11d, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xdd64e639, "strscpy" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xa916b694, "strnlen" },
	{ 0x11089ac7, "_ctype" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x69acdf38, "memcpy" },
	{ 0x92997ed8, "_printk" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "03576AB9EE12AFA28A794AC");
