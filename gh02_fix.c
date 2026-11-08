// SPDX-License-Identifier: GPL-2.0
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/unaligned.h>
#include <linux/usb.h>

#include "gh02_cfg291.h"

#define GH02_VID		0x10d6
#define GH02_PID		0x4802
#define GH02_LINUX_TOTAL	261

struct gh02_call {
	struct usb_device *dev;
	unsigned char *buf;
	int size;
};

static bool gh02_is_dongle(const struct usb_device *dev)
{
	return le16_to_cpu(dev->descriptor.idVendor) == GH02_VID &&
	       le16_to_cpu(dev->descriptor.idProduct) == GH02_PID;
}

static int gh02_entry(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct gh02_call *call = (struct gh02_call *)ri->data;
	struct usb_device *dev = (struct usb_device *)regs_get_kernel_argument(regs, 0);
	unsigned char type = regs_get_kernel_argument(regs, 1);
	unsigned char index = regs_get_kernel_argument(regs, 2);

	if (!dev || type != USB_DT_CONFIG || index != 0 || !gh02_is_dongle(dev))
		return 1;

	call->dev = dev;
	call->buf = (unsigned char *)regs_get_kernel_argument(regs, 3);
	call->size = regs_get_kernel_argument(regs, 4);
	return 0;
}

static int gh02_return(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct gh02_call *call = (struct gh02_call *)ri->data;
	int ret = regs_return_value(regs);
	unsigned char *buf = call->buf;

	if (ret < 4 || buf[1] != USB_DT_CONFIG)
		return 0;
	if (get_unaligned_le16(buf + 2) != GH02_LINUX_TOTAL)
		return 0;

	if (call->size == USB_DT_CONFIG_SIZE) {
		put_unaligned_le16(sizeof(gh02_cfg), buf + 2);
		if (ret >= 5)
			buf[4] = gh02_cfg[4];
	} else if (call->size >= (int)sizeof(gh02_cfg)) {
		memcpy(buf, gh02_cfg, sizeof(gh02_cfg));
		regs_set_return_value(regs, sizeof(gh02_cfg));
		dev_info(&call->dev->dev, "gh02: using the Windows configuration descriptor (game output)\n");
	}
	return 0;
}

static struct kretprobe gh02_probe = {
	.kp.symbol_name	= "usb_get_descriptor",
	.entry_handler	= gh02_entry,
	.handler	= gh02_return,
	.data_size	= sizeof(struct gh02_call),
	.maxactive	= 16,
};

static int __init gh02_init(void)
{
	int ret = register_kretprobe(&gh02_probe);

	if (ret)
		pr_err("gh02: register_kretprobe failed: %d\n", ret);
	else
		pr_info("gh02: active; replug or re-enumerate the dongle if it was already present\n");
	return ret;
}

static void __exit gh02_exit(void)
{
	unregister_kretprobe(&gh02_probe);
}

module_init(gh02_init);
module_exit(gh02_exit);
MODULE_DESCRIPTION("Expose the game output of the Baseus GH02 2.4G dongle on Linux");
MODULE_LICENSE("GPL");
