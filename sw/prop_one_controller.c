#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kstrtox.h>

struct prop_one_controller_dev {
	void __iomem *base_addr;
	void __iomem *button_1;
	void __iomem *button_2;
	void __iomem *leds;
	struct miscdevice miscdev;
	struct mutex lock;
};

static const struct of_device_id prop_one_controller_of_match[] = {
	{ .compatible = "culwell,prop_one_controller", },
	{ }
};
MODULE_DEVICE_TABLE(of, prop_one_controller_of_match);
MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Joshua Culwell");
MODULE_DESCRIPTION("prop_one_controller driver");

static ssize_t leds_show(struct device *dev, struct device_attribute *attr, char *buf){
	u8 leds;
	struct prop_one_controller_dev *priv = dev_get_drvdata(dev);
	leds = ioread32(priv->leds);

	return scnprintf(buf, PAGE_SIZE, "%u\n", leds);
}
static ssize_t leds_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size){
	u8 leds;
	int ret;
	struct prop_one_controller_dev *priv = dev_get_drvdata(dev);

	ret = kstrtou8(buf, 0, &leds);
	if(ret < 0){
		return ret;
	}
	iowrite32(leds, priv->leds);

	return size;
}

static ssize_t button_1_show(struct device *dev, struct device_attribute *attr, char *buf){
	bool button_1;

	struct prop_one_controller_dev *priv = dev_get_drvdata(dev);
	
	button_1 = ioread32(priv->button_1);

	return scnprintf(buf, PAGE_SIZE, "%u\n", button_1);
}
static ssize_t button_1_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size){
	bool button_1;
	int ret;

	struct prop_one_controller_dev *priv = dev_get_drvdata(dev);

	ret = kstrtobool(buf, &button_1);
	if(ret < 0){
		return ret;
	}
	iowrite32(button_1, priv->button_1);

	return size;
}

static ssize_t button_2_show(struct device *dev, struct device_attribute *attr, char *buf){
	bool button_2;

	struct prop_one_controller_dev *priv = dev_get_drvdata(dev);
	
	button_2 = ioread32(priv->button_2);

	return scnprintf(buf, PAGE_SIZE, "%u\n", button_2);
}
static ssize_t button_2_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size){
	bool button_2;
	int ret;

	struct prop_one_controller_dev *priv = dev_get_drvdata(dev);

	ret = kstrtobool(buf, &button_2);
	if(ret < 0){
		return ret;
	}
	iowrite32(button_2, priv->button_2);

	return size;
}
static ssize_t prop_one_controller_read(struct file *file, char __user *buf, size_t count, loff_t *offset){
	size_t ret;
	u32 val;

	struct prop_one_controller_dev *priv = container_of(file->private_data, struct prop_one_controller_dev, miscdev);

	if(*offset < 0){
		return -EINVAL;
	}
	if(*offset >= 16){
		return 0;
	}
	if((*offset % 0x4) != 0){
		pr_warn("prop_one_controller_read: unaligned access\n");
		return -EFAULT;
	}

	val = ioread32(priv->base_addr + *offset);

	ret = copy_to_user(buf, &val, sizeof(val));

	if(ret ==sizeof(val)){
		pr_warn("prop_one_controller_read: nothing copied\n");
		return -EFAULT;
	}

	*offset = *offset + sizeof(val);

	return sizeof(val);
}

static ssize_t prop_one_controller_write(struct file *file, const char __user *buf, size_t count, loff_t *offset){
	size_t ret;
	u32 val;

	struct prop_one_controller_dev *priv = container_of(file->private_data, struct prop_one_controller_dev, miscdev);

	if(*offset < 0){
		return -EINVAL;
	}
	if(*offset >= 16){
		return 0;
	}
	if((*offset % 0x4) != 0){
		pr_warn("prop_one_controller_write: unaligned access\n");
		return -EFAULT;
	}
	
	mutex_lock(&priv->lock);

	ret = copy_from_user(&val, buf, sizeof(val));
	if(ret != sizeof(val)){
		iowrite32(val, priv->base_addr + *offset);
		*offset = *offset + sizeof(val);
		ret = sizeof(val);
	}else{
		pr_warn("prop_one_controller_write: nothing copied from user space\n");
		ret = -EFAULT;
	}

	mutex_unlock(&priv->lock);
	return ret;
}

static const struct file_operations prop_one_controller_fops = {
	.owner = THIS_MODULE,
	.read = prop_one_controller_read,
	.write = prop_one_controller_write,
	.llseek = default_llseek,
};

static int prop_one_controller_probe(struct platform_device *pdev){
	struct prop_one_controller_dev *priv;

	priv = devm_kzalloc(&pdev->dev, sizeof(struct prop_one_controller_dev), GFP_KERNEL);
	if(!priv){
		pr_err("Failed to alocate memory\n");
		return -ENOMEM;
	}

	priv->base_addr = devm_platform_ioremap_resource(pdev, 0);
	if(IS_ERR(priv->base_addr)){
		pr_err("Failed to request/remap platform device resouce\n");
		return PTR_ERR(priv->base_addr);
	}

	priv->leds = priv->base_addr + 8;
	priv->button_1 = priv->base_addr + 0;
	priv->button_2 = priv->base_addr + 4;

	priv->miscdev.minor = MISC_DYNAMIC_MINOR;
	priv->miscdev.name = "prop_one_controller";
	priv->miscdev.fops = &prop_one_controller_fops;
	priv->miscdev.parent = &pdev->dev;

	size_t ret = misc_register(&priv->miscdev);
	if (ret) {
		pr_err("Failed to register misc device");
		return ret;
	}else{
		pr_info("prop_one_controller device registered successfully\n");
	}

	platform_set_drvdata(pdev, priv);

	pr_info("prop_one_controller_probe successful\n");

	return 0;
}

static int prop_one_controller_remove(struct platform_device *pdev){
	struct prop_one_controller_dev *priv = platform_get_drvdata(pdev);
	
	misc_deregister(&priv->miscdev);

	pr_info("prop_one_controller_remove successful\n");

	return 0;
}

static DEVICE_ATTR_RW(leds);
static DEVICE_ATTR_RW(button_1);
static DEVICE_ATTR_RW(button_2);

static struct attribute *prop_one_controller_attrs[] = {
	&dev_attr_leds.attr,
	&dev_attr_button_1.attr,
	&dev_attr_button_2.attr,
	NULL,
};
ATTRIBUTE_GROUPS(prop_one_controller);

static struct platform_driver prop_one_controller_driver = {
	.probe = prop_one_controller_probe,
	.remove = prop_one_controller_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "prop_one_controller",
		.of_match_table = prop_one_controller_of_match,
		.dev_groups = prop_one_controller_groups,
	},
};
module_platform_driver(prop_one_controller_driver);
