#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kstrtox.h>

struct rgb_controller_dev {
	void __iomem *base_addr;
	void __iomem *period;
	void __iomem *red_duty_cycle;
	void __iomem *green_duty_cycle;
	void __iomem *blue_duty_cycle;
	struct miscdevice miscdev;
	struct mutex lock;
};

static const struct of_device_id rgb_controller_of_match[] = {
	{ .compatible = "culwell,rgb_controller", },
	{ }
};
MODULE_DEVICE_TABLE(of, rgb_controller_of_match);
MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Joshua Culwell");
MODULE_DESCRIPTION("rgb_controller driver");

static ssize_t period_show(struct device *dev, struct device_attribute *attr, char *buf){
	u16 period;
	struct rgb_controller_dev *priv = dev_get_drvdata(dev);
	period = ioread32(priv->period);

	return scnprintf(buf, PAGE_SIZE, "%u\n", period);
}
static ssize_t period_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size){
	u16 period;
	int ret;
	struct rgb_controller_dev *priv = dev_get_drvdata(dev);

	ret = kstrtou16(buf, 0, &period);
	if(ret < 0){
		return ret;
	}
	iowrite32(period, priv->period);

	return size;
}

static ssize_t red_duty_cycle_show(struct device *dev, struct device_attribute *attr, char *buf){
	u16 red_duty_cycle;

	struct rgb_controller_dev *priv = dev_get_drvdata(dev);
	
	red_duty_cycle = ioread32(priv->red_duty_cycle);

	return scnprintf(buf, PAGE_SIZE, "%u\n", red_duty_cycle);
}
static ssize_t red_duty_cycle_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size){
	u16 red_duty_cycle;
	int ret;

	struct rgb_controller_dev *priv = dev_get_drvdata(dev);

	ret = kstrtou16(buf, 0, &red_duty_cycle);
	if(ret < 0){
		return ret;
	}
	iowrite32(red_duty_cycle, priv->red_duty_cycle);

	return size;
}

static ssize_t green_duty_cycle_show(struct device *dev, struct device_attribute *attr, char *buf){
	u16 green_duty_cycle;

	struct rgb_controller_dev *priv = dev_get_drvdata(dev);
	
	green_duty_cycle = ioread32(priv->green_duty_cycle);

	return scnprintf(buf, PAGE_SIZE, "%u\n", green_duty_cycle);
}
static ssize_t green_duty_cycle_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size){
	u16 green_duty_cycle;
	int ret;

	struct rgb_controller_dev *priv = dev_get_drvdata(dev);

	ret = kstrtou16(buf, 0, &green_duty_cycle);
	if(ret < 0){
		return ret;
	}
	iowrite32(green_duty_cycle, priv->green_duty_cycle);

	return size;
}

static ssize_t blue_duty_cycle_show(struct device *dev, struct device_attribute *attr, char *buf){
	u16 blue_duty_cycle;

	struct rgb_controller_dev *priv = dev_get_drvdata(dev);
	
	blue_duty_cycle = ioread32(priv->blue_duty_cycle);

	return scnprintf(buf, PAGE_SIZE, "%u\n", blue_duty_cycle);
}
static ssize_t blue_duty_cycle_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size){
	u16 blue_duty_cycle;
	int ret;

	struct rgb_controller_dev *priv = dev_get_drvdata(dev);

	ret = kstrtou16(buf, 0, &blue_duty_cycle);
	if(ret < 0){
		return ret;
	}
	iowrite32(blue_duty_cycle, priv->blue_duty_cycle);

	return size;
}

static ssize_t rgb_controller_read(struct file *file, char __user *buf, size_t count, loff_t *offset){
	size_t ret;
	u32 val;

	struct rgb_controller_dev *priv = container_of(file->private_data, struct rgb_controller_dev, miscdev);

	if(*offset < 0){
		return -EINVAL;
	}
	if(*offset >= 16){
		return 0;
	}
	if((*offset % 0x4) != 0){
		pr_warn("rgb_controller_read: unaligned access\n");
		return -EFAULT;
	}

	val = ioread32(priv->base_addr + *offset);

	ret = copy_to_user(buf, &val, sizeof(val));

	if(ret ==sizeof(val)){
		pr_warn("rgb_controller_read: nothing copied\n");
		return -EFAULT;
	}

	*offset = *offset + sizeof(val);

	return sizeof(val);
}

static ssize_t rgb_controller_write(struct file *file, const char __user *buf, size_t count, loff_t *offset){
	size_t ret;
	u32 val;

	struct rgb_controller_dev *priv = container_of(file->private_data, struct rgb_controller_dev, miscdev);

	if(*offset < 0){
		return -EINVAL;
	}
	if(*offset >= 16){
		return 0;
	}
	if((*offset % 0x4) != 0){
		pr_warn("rgb_controller_write: unaligned access\n");
		return -EFAULT;
	}
	
	mutex_lock(&priv->lock);

	ret = copy_from_user(&val, buf, sizeof(val));
	if(ret != sizeof(val)){
		iowrite32(val, priv->base_addr + *offset);
		*offset = *offset + sizeof(val);
		ret = sizeof(val);
	}else{
		pr_warn("rgb_controller_write: nothing copied from user space\n");
		ret = -EFAULT;
	}

	mutex_unlock(&priv->lock);
	return ret;
}

static const struct file_operations rgb_controller_fops = {
	.owner = THIS_MODULE,
	.read = rgb_controller_read,
	.write = rgb_controller_write,
	.llseek = default_llseek,
};

static int rgb_controller_probe(struct platform_device *pdev){
	struct rgb_controller_dev *priv;

	priv = devm_kzalloc(&pdev->dev, sizeof(struct rgb_controller_dev), GFP_KERNEL);
	if(!priv){
		pr_err("Failed to alocate memory\n");
		return -ENOMEM;
	}

	priv->base_addr = devm_platform_ioremap_resource(pdev, 0);
	if(IS_ERR(priv->base_addr)){
		pr_err("Failed to request/remap platform device resouce\n");
		return PTR_ERR(priv->base_addr);
	}

	priv->period = priv->base_addr + 0;
	priv->red_duty_cycle = priv->base_addr + 4;
	priv->green_duty_cycle = priv->base_addr + 8;
	priv->blue_duty_cycle = priv->base_addr + 12;

	priv->miscdev.minor = MISC_DYNAMIC_MINOR;
	priv->miscdev.name = "rgb_controller";
	priv->miscdev.fops = &rgb_controller_fops;
	priv->miscdev.parent = &pdev->dev;

	size_t ret = misc_register(&priv->miscdev);
	if (ret) {
		pr_err("Failed to register misc device");
		return ret;
	}else{
		pr_info("rgb_controller device registered successfully\n");
	}

	platform_set_drvdata(pdev, priv);

	pr_info("rgb_controller_probe successful\n");

	return 0;
}

static int rgb_controller_remove(struct platform_device *pdev){
	struct rgb_controller_dev *priv = platform_get_drvdata(pdev);
	
	misc_deregister(&priv->miscdev);
	
	pr_info("rgb_controller_remove successful\n");

	return 0;
}

static DEVICE_ATTR_RW(period);
static DEVICE_ATTR_RW(red_duty_cycle);
static DEVICE_ATTR_RW(green_duty_cycle);
static DEVICE_ATTR_RW(blue_duty_cycle);

static struct attribute *rgb_controller_attrs[] = {
	&dev_attr_period.attr,
	&dev_attr_red_duty_cycle.attr,
	&dev_attr_green_duty_cycle.attr,
	&dev_attr_blue_duty_cycle.attr,
	NULL,
};
ATTRIBUTE_GROUPS(rgb_controller);

static struct platform_driver rgb_controller_driver = {
	.probe = rgb_controller_probe,
	.remove = rgb_controller_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "rgb_controller",
		.of_match_table = rgb_controller_of_match,
		.dev_groups = rgb_controller_groups,
	},
};
module_platform_driver(rgb_controller_driver);
