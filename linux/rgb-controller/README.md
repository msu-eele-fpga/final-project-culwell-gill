# rgb_controller.c

## About
rgb_controller.c is the device driver for the rgb_controller HDL. 


## Function 
This file is the intermediary connection between the Linux Kernel and FPGA HDL.

## Dev Struct

```c
struct rgb_controller_dev {
	void __iomem *base_addr;
	void __iomem *period;
	void __iomem *red_duty_cycle;
	void __iomem *green_duty_cycle;
	void __iomem *blue_duty_cycle;
	struct miscdevice miscdev;
	struct mutex lock;
};
```
Holds instance specific info about the device

## Show and Store

```c
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
```
Utilizes Sysfs to read and write to device registers.

## Attribute Group

```c
static struct attribute *rgb_controller_attrs[] = {
	&dev_attr_period.attr,
	&dev_attr_red_duty_cycle.attr,
	&dev_attr_green_duty_cycle.attr,
	&dev_attr_blue_duty_cycle.attr,
	NULL,
};
ATTRIBUTE_GROUPS(rgb_controller);
```
Needed for each component

## Char Device Methods

```c
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
```
Needed for component as a whole

## File Operations

```c
static const struct file_operations rgb_controller_fops = {
	.owner = THIS_MODULE,
	.read = rgb_controller_read,
	.write = rgb_controller_write,
	.llseek = default_llseek,
};
```
Defines operations supported by device driver.

## Probe and Remove

```c
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
```
Function called when driver is loaded into the kernel.


## General Info

```c
static const struct of_device_id rgb_controller_of_match[] = {
	{ .compatible = "culwell,rgb_controller", },
	{ }
};
MODULE_DEVICE_TABLE(of, rgb_controller_of_match);
MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Joshua Culwell");
MODULE_DESCRIPTION("rgb_controller driver");
```
Provides general information such as author, license, and description.