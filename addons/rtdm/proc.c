/*
 * Copyright (C) 2005 Jan Kiszka <jan.kiszka@web.de>.
 * Copyright (C) 2005 Joerg Langenberg <joerg.langenberg@gmx.net>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include "internal.h"
#include <rtdm/vfile.h>

struct xnvfile_directory rtdm_vfroot;	/* /proc/RTDM/rtdm */

struct vfile_device_data {
	int h;
	int hmax;
	struct list_head *devmap;
	struct list_head *curr;
};

static int get_nrt_lock(struct xnvfile *vfile)
{
	return down_interruptible(&nrt_dev_lock) ? -ERESTARTSYS : 0;
}

static void put_nrt_lock(struct xnvfile *vfile)
{
	up(&nrt_dev_lock);
}

static struct xnvfile_lock_ops lockops = {
	.get = get_nrt_lock,
	.put = put_nrt_lock,
};

static struct list_head *next_devlist(struct vfile_device_data *priv)
{
	struct list_head *head;

	while (priv->h < priv->hmax) {
		head = priv->devmap + priv->h;
		if (!list_empty(head))
			return head;
		priv->h++;
	}

	return NULL;
}

static void *next_dev(struct xnvfile_regular_iterator *it)
{
	struct vfile_device_data *priv = xnvfile_iterator_priv(it);
	struct list_head *next;

	next = priv->curr->next;
seek:
	if (next == priv->devmap + priv->h) {
		/* Done with the current hash slot, let's progress. */
		if (priv->h >= priv->hmax) {
			next = NULL; /* all done. */
			goto out;
		}

		priv->h++;
		next = next_devlist(priv);
		if (next) {
			next = next->next; /* skip head. */
			goto seek;
		}
	}
out:
	priv->curr = next;

	return next;
}

static void *named_begin(struct xnvfile_regular_iterator *it)
{
	struct vfile_device_data *priv = xnvfile_iterator_priv(it);
	struct list_head *devlist;
	loff_t pos = 0;

	priv->devmap = rtdm_named_devices;
	priv->hmax = devname_hashtab_size;
	priv->h = 0;

	devlist = next_devlist(priv);
	if (devlist == NULL)
		return NULL;	/* All devlists empty. */

	priv->curr = devlist->next;	/* Skip head. */

	/*
	 * priv->curr now points to the first device; advance to the requested
	 * position from there.
	 */
	while (priv->curr && pos++ < it->pos)
		priv->curr = next_dev(it);

	if (pos == 1)
		/* Output the header once, only if some device follows. */
		xnvfile_puts(it, "Hash\tName\t\t\t\tDriver\t\t/proc\n");

	return priv->curr;
}

static int named_show(struct xnvfile_regular_iterator *it, void *data)
{
	struct vfile_device_data *priv = xnvfile_iterator_priv(it);
	struct list_head *curr = data;
	struct rtdm_device *device;

	device = list_entry(curr, struct rtdm_device, reserved.entry);
	xnvfile_printf(it, "%02X\t%-31s\t%-15s\t%s\n",
		       priv->h, device->device_name,
		       device->driver_name,
		       device->proc_name);

	return 0;
}

static struct xnvfile_regular_ops named_vfile_ops = {
	.begin = named_begin,
	.next = next_dev,
	.show = named_show,
};

static struct xnvfile_regular named_vfile = {
	.privsz = sizeof(struct vfile_device_data),
	.ops = &named_vfile_ops,
	.entry = { .lockops = &lockops }
};

static void *proto_begin(struct xnvfile_regular_iterator *it)
{

	struct vfile_device_data *priv = xnvfile_iterator_priv(it);
	struct list_head *devlist;
	loff_t pos = 0;

	priv->devmap = rtdm_protocol_devices;
	priv->hmax = protocol_hashtab_size;
	priv->h = 0;

	devlist = next_devlist(priv);
	if (devlist == NULL)
		return NULL;	/* All devlists empty. */

	priv->curr = devlist->next;	/* Skip head. */

	/*
	 * priv->curr now points to the first device; advance to the requested
	 * position from there.
	 */
	while (priv->curr && pos++ < it->pos)
		priv->curr = next_dev(it);

	if (pos == 1)
		/* Output the header once, only if some device follows. */
		xnvfile_puts(it, "Hash\tName\t\t\t\tDriver\t\t/proc\n");

	return priv->curr;
}

static int proto_show(struct xnvfile_regular_iterator *it, void *data)
{
	struct vfile_device_data *priv = xnvfile_iterator_priv(it);
	struct list_head *curr = data;
	struct rtdm_device *device;
	char pnum[32];

	device = list_entry(curr, struct rtdm_device, reserved.entry);

	snprintf(pnum, sizeof(pnum), "%u:%u",
		 device->protocol_family, device->socket_type);

	xnvfile_printf(it, "%02X\t%-31s\t%-15s\t%s\n",
		       priv->h,
		       pnum, device->driver_name,
		       device->proc_name);
	return 0;
}

static struct xnvfile_regular_ops proto_vfile_ops = {
	.begin = proto_begin,
	.next = next_dev,
	.show = proto_show,
};

static struct xnvfile_regular proto_vfile = {
	.privsz = sizeof(struct vfile_device_data),
	.ops = &proto_vfile_ops,
	.entry = { .lockops = &lockops }
};

static void *openfd_begin(struct xnvfile_regular_iterator *it)
{
	if (it->pos == 0)
		return VFILE_SEQ_START;

	return it->pos <= RTDM_FD_MAX ? it : NULL;
}

static void *openfd_next(struct xnvfile_regular_iterator *it)
{
	if (it->pos > RTDM_FD_MAX)
		return NULL;

	return it;
}

static int openfd_show(struct xnvfile_regular_iterator *it, void *data)
{
	struct rtdm_dev_context *context;
	struct rtdm_device *device;
	struct rtdm_process owner;
	int close_lock_count, fd;
	spl_t s;

	if (data == NULL) {
		xnvfile_puts(it, "Index\tLocked\tDevice\t\t\t\tOwner [PID]\n");
		return 0;
	}

	fd = (int)it->pos - 1;

	xnlock_get_irqsave(&rt_fildes_lock, s);

	context = fildes_table[fd].context;
	if (context == NULL) {
		xnlock_put_irqrestore(&rt_fildes_lock, s);
		return VFILE_SEQ_SKIP;
	}

	close_lock_count = atomic_read(&context->close_lock_count);
	device = context->device;
	if (context->reserved.owner)
		memcpy(&owner, context->reserved.owner, sizeof(owner));
	else {
		strcpy(owner.name, "<kernel>");
		owner.pid = -1;
	}

	xnlock_put_irqrestore(&rt_fildes_lock, s);

	xnvfile_printf(it, "%d\t%d\t%-31s %s [%d]\n", fd,
		       close_lock_count,
		       (device->device_flags & RTDM_NAMED_DEVICE) ?
		       device->device_name : device->proc_name,
		       owner.name, owner.pid);
	return 0;
}

static ssize_t openfd_store(struct xnvfile_input *input)
{
	ssize_t ret, cret;
	long val;

	ret = xnvfile_get_integer(input, &val);
	if (ret < 0)
		return ret;

	cret = __rt_dev_close(current, (int)val);
	if (cret < 0)
		return cret;

	return ret;
}

static struct xnvfile_regular_ops openfd_vfile_ops = {
	.begin = openfd_begin,
	.next = openfd_next,
	.show = openfd_show,
	.store = openfd_store,
};

static struct xnvfile_regular openfd_vfile = {
	.ops = &openfd_vfile_ops,
	.entry = { .lockops = &lockops }
};

static int allfd_vfile_show(struct xnvfile_regular_iterator *it, void *data)
{
	xnvfile_printf(it, "total=%d:open=%d:free=%d\n", RTDM_FD_MAX,
		       open_fildes, RTDM_FD_MAX - open_fildes);
	return 0;
}

static struct xnvfile_regular_ops allfd_vfile_ops = {
	.show = allfd_vfile_show,
};

static struct xnvfile_regular allfd_vfile = {
	.ops = &allfd_vfile_ops,
};

static int devinfo_vfile_show(struct xnvfile_regular_iterator *it, void *data)
{
	struct rtdm_device *device;
	int i;

	if (down_interruptible(&nrt_dev_lock))
		return -ERESTARTSYS;

	/*
	 * As the device may have disappeared while the handler was called,
	 * first match the pointer against registered devices.
	 */
	for (i = 0; i < devname_hashtab_size; i++)
		list_for_each_entry(device, &rtdm_named_devices[i],
				    reserved.entry)
			if (device == xnvfile_priv(it->vfile))
				goto found;

	for (i = 0; i < protocol_hashtab_size; i++)
		list_for_each_entry(device, &rtdm_protocol_devices[i],
				    reserved.entry)
			if (device == xnvfile_priv(it->vfile))
				goto found;

	up(&nrt_dev_lock);
	return -ENODEV;

found:
	xnvfile_printf(it, "driver:\t\t%s\nversion:\t%d.%d.%d\n",
		       device->driver_name,
		       RTDM_DRIVER_MAJOR_VER(device->driver_version),
		       RTDM_DRIVER_MINOR_VER(device->driver_version),
		       RTDM_DRIVER_PATCH_VER(device->driver_version));

	xnvfile_printf(it, "peripheral:\t%s\nprovider:\t%s\n",
		       device->peripheral_name, device->provider_name);

	xnvfile_printf(it, "class:\t\t%d\nsub-class:\t%d\n",
		       device->device_class, device->device_sub_class);

	xnvfile_printf(it, "flags:\t\t%s%s%s\n",
		       (device->device_flags & RTDM_EXCLUSIVE) ?
		       "EXCLUSIVE  " : "",
		       (device->device_flags & RTDM_NAMED_DEVICE) ?
		       "NAMED_DEVICE  " : "",
		       (device->device_flags & RTDM_PROTOCOL_DEVICE) ?
		       "PROTOCOL_DEVICE  " : "");

	xnvfile_printf(it, "lock count:\t%d\n",
		       atomic_read(&device->reserved.refcount));

	up(&nrt_dev_lock);
	return 0;
}

static struct xnvfile_regular_ops devinfo_vfile_ops = {
	.show = devinfo_vfile_show,
};

int rtdm_proc_register_device(struct rtdm_device *device)
{
	int ret;

	ret = xnvfile_init_dir(device->proc_name,
			       &device->vfroot, &rtdm_vfroot);
	if (ret)
		goto err_out;

	memset(&device->info_vfile, 0, sizeof(device->info_vfile));
	device->info_vfile.ops = &devinfo_vfile_ops;

	ret = xnvfile_init_regular("information", &device->info_vfile,
				   &device->vfroot);
	if (ret) {
		xnvfile_destroy_dir(&device->vfroot);
		goto err_out;
	}

	xnvfile_priv(&device->info_vfile) = device;

	return 0;

      err_out:
	xnlogerr("RTDM: error while creating device vfile\n");
	return ret;
}

void rtdm_proc_unregister_device(struct rtdm_device *device)
{
	xnvfile_destroy_regular(&device->info_vfile);
	xnvfile_destroy_dir(&device->vfroot);
}

int __init rtdm_proc_init(void)
{
	int ret;

	/* Initialise vfiles */
	ret = xnvfile_init_dir("rtdm", &rtdm_vfroot, &nkvfroot);
	if (ret)
		goto error;

	ret = xnvfile_init_regular("named_devices", &named_vfile, &rtdm_vfroot);
	if (ret)
		goto error;

	ret = xnvfile_init_regular("protocol_devices", &proto_vfile, &rtdm_vfroot);
	if (ret)
		goto error;

	ret = xnvfile_init_regular("open_fildes", &openfd_vfile, &rtdm_vfroot);
	if (ret)
		goto error;

	ret = xnvfile_init_regular("fildes", &allfd_vfile, &rtdm_vfroot);
	if (ret)
		goto error;

	return 0;

error:
	rtdm_proc_cleanup();
	return ret;
}

void rtdm_proc_cleanup(void)
{
	xnvfile_destroy_regular(&allfd_vfile);
	xnvfile_destroy_regular(&openfd_vfile);
	xnvfile_destroy_regular(&proto_vfile);
	xnvfile_destroy_regular(&named_vfile);
	xnvfile_destroy_dir(&rtdm_vfroot);
}
