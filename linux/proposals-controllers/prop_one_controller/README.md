# prop_one_controller.c

## About
prop_one_controller.c is the device driver for the prop_one HDL. 


## Function 
This file serves as an intermediary connection between the linux Kernel and the device hardware.

## Device tree node

Use the following device tree node:
```devicetree
prop_one_controller: prop_one_controller@ff200030 {
		compatible = "culwell,prop_one_controller";
		reg = <0xff200030 16>;
```

## Dev Struct

```c
struct prop_one_controller_dev {
	void __iomem *base_addr;
	void __iomem *button_1;
	void __iomem *button_2;
	void __iomem *leds;
	struct miscdevice miscdev;
	struct mutex lock;
};
```
Holds instance specific info about the device

## Show and Store

```c
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
```
Needed for component as a whole

## File Operations

```c
static const struct file_operations prop_one_controller_fops = {
	.owner = THIS_MODULE,
	.read = prop_one_controller_read,
	.write = prop_one_controller_write,
	.llseek = default_llseek,
};
```
Defines operations supported by device driver.

## Probe and Remove

```c
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
```
Function called when driver is loaded into the kernel.


## General Info

```c
static const struct of_device_id prop_one_controller_of_match[] = {
	{ .compatible = "culwell,prop_one_controller", },
	{ }
};
MODULE_DEVICE_TABLE(of, prop_one_controller_of_match);
MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Joshua Culwell");
MODULE_DESCRIPTION("prop_one_controller driver");
```
Provides general information such as author, license, and description.