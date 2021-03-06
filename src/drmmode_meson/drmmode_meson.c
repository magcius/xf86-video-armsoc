/*
 * Copyright (C) 2014 Endless Mobile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by:
 *     Jasper St. Pierre <jstpierre@mecheye.net>
 */

#include "../drmmode_driver.h"

#include "meson_drm.h"
#include <xf86drm.h>
#include <xf86drmMode.h>

/* Cursor dimensions
 * Technically we probably don't have any size limit.. since we
 * are just using an overlay... but xserver will always create
 * cursor images in the max size, so don't use width/height values
 * that are too big
 */
#define CURSORW  (64)
#define CURSORH  (64)

#define CURSORPAD  (0)

#define ALIGN(val, align)	(((val) + (align) - 1) & ~((align) - 1))

static int create_custom_gem(int fd, struct armsoc_create_gem *create_gem)
{
	struct drm_meson_gem_create_with_ump create_meson;
	size_t pitch;
	int ret;

	assert((create_gem->buf_type == ARMSOC_BO_SCANOUT) ||
	       (create_gem->buf_type == ARMSOC_BO_NON_SCANOUT));

	/* make pitch a multiple of 64 bytes for best performance */
	pitch = ALIGN(create_gem->width * ((create_gem->bpp + 7) / 8), 64);
	create_meson.size = create_gem->height * pitch;
	create_meson.flags = 0;

	ret = drmIoctl(fd, DRM_IOCTL_MESON_GEM_CREATE_WITH_UMP, &create_meson);
	if (ret)
		return ret;

	/* Convert custom meson ioctl to generic create_gem */
	create_gem->handle = create_meson.handle;
	create_gem->pitch = pitch;
	create_gem->size = create_meson.size;
	create_gem->name = create_meson.ump_secure_id;

	return 0;
}

struct drmmode_interface meson_interface = {
	"meson",
	1                     /* use_page_flip_events */,
	CURSORW               /* cursor width */,
	CURSORH               /* cursor_height */,
	CURSORPAD             /* cursor padding */,
	HWCURSOR_API_STANDARD /* cursor_api */,
	NULL                  /* init_plane_for_cursor */,
	0                     /* vblank_query_supported */,
	create_custom_gem     /* create_custom_gem */,
};
