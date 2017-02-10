#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/vmalloc.h>
#include <linux/disp_assert_layer.h>
#include <linux/semaphore.h>
#include <linux/xlog.h>
#include <linux/mutex.h>
#include <linux/leds-mt65xx.h>
#include <linux/file.h>
#include <linux/ion_drv.h>
#include <linux/list.h>


#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/mach-types.h>
#include <asm/cacheflush.h>
#include <asm/io.h>

#include <mach/dma.h>
#include <mach/irqs.h>
#include <mach/m4u_port.h>
#include <linux/dma-mapping.h>
#include <mach/m4u.h>

/* driver internal header files */
#include "debug.h"
#include "disp_drv_log.h"
#include "mtkfb.h"
#include "mtkfb_info.h"
#include "mtkfb_priv.h"
#include "disp_hal.h"
#include "mtkfb_1.h"

#define MAX_FB_W 1920
#define MAX_FB_H 1080
#define FB_PAGES 2 /* double buffer */

#define MTK_FB_ALIGNMENT 16
#define ALIGN_TO(x, n)  \
    (((x) + ((n) - 1)) & ~((n) - 1))

static u32 MTK_FB_XRES  = 0;
static u32 MTK_FB_YRES  = 0;
static u32 MTK_FB_BPP   = 0;
static u32 MTK_FB_PAGES = 0;
static u32 fb_xres_update = 0;
static u32 fb_yres_update = 0;

#define MTK_FB_XRESV (ALIGN_TO(MTK_FB_XRES, disphal_get_fb_alignment()))
#define MTK_FB_YRESV (ALIGN_TO(MTK_FB_YRES, disphal_get_fb_alignment()) * MTK_FB_PAGES) /* For page flipping */
#define MTK_FB_BYPP  ((MTK_FB_BPP + 7) >> 3)
#define MTK_FB_LINE  (ALIGN_TO(MTK_FB_XRES, disphal_get_fb_alignment()) * MTK_FB_BYPP)
#define MTK_FB_SIZE  (MTK_FB_LINE * ALIGN_TO(MTK_FB_YRES, disphal_get_fb_alignment()))

#define MTK_FB_SIZEV (MTK_FB_LINE * ALIGN_TO(MTK_FB_YRES, disphal_get_fb_alignment()) * MTK_FB_PAGES)

static size_t mtkfb_1_log_on = true;

void mtkfb_1_log_enable(int enable)
{
    mtkfb_1_log_on = enable;
    MTKFB_1_LOG("mtkfb 1 log %s\n", enable?"enabled":"disabled");
}

// ---------------------------------------------------------------------------
//  local variables
// ---------------------------------------------------------------------------
struct fb_info         *mtkfb_1_fbi = NULL;
HOOK_PAN_DISPLAY_IMPL  mtkfb_1_hook_pan_display = NULL;

/*
 * ---------------------------------------------------------------------------
 * fbdev framework callbacks and the ioctl interface
 * ---------------------------------------------------------------------------
 */
/* Called each time the mtkfb device is opened */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static int mtkfb_1_open(struct file *file, struct fb_info *info, int user)
#else
static int mtkfb_1_open(struct fb_info *info, int user)
#endif
{
    MTKFB_1_API_ENTRY();

    return 0;
}

/* Called when the mtkfb device is closed. We make sure that any pending
 * gfx DMA operations are ended, before we return. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static int mtkfb_1_release(struct file *file, struct fb_info *info, int user)
#else
static int mtkfb_1_release(struct fb_info *info, int user)
#endif

{
    MTKFB_1_API_ENTRY();

    return 0;
}

/* Store a single color palette entry into a pseudo palette or the hardware
 * palette if one is available. For now we support only 16bpp and thus store
 * the entry only to the pseudo palette.
 */
static int mtkfb_1_setcolreg(u_int regno, u_int red, u_int green,
                           u_int blue, u_int transp,
                           struct fb_info *info)
{
    int r = 0;
    unsigned bpp, m;

    MTKFB_1_API_ENTRY();

    bpp = info->var.bits_per_pixel;
    m = 1 << bpp;

    if (regno >= m)
    {
        r = -EINVAL;
        goto exit;
    }

    switch (bpp)
    {
    case 16:
        /* RGB 565 */
        ((u32 *)(info->pseudo_palette))[regno] =
            ((red & 0xF800) |
            ((green & 0xFC00) >> 5) |
            ((blue & 0xF800) >> 11));
        break;
    case 32:
        /* ARGB8888 */
        ((u32 *)(info->pseudo_palette))[regno] =
             (0xff000000)           |
            ((red   & 0xFF00) << 8) |
            ((green & 0xFF00)     ) |
            ((blue  & 0xFF00) >> 8);
        break;

    // TODO: RGB888, BGR888, ABGR8888

    default:
        ASSERT(0);
    }

exit:

    return r;
}

/*
static void mtkfb_1_update_screen_impl(void)
{
    MTKFB_1_API_ENTRY();

}

static int mtkfb_1_update_screen(struct fb_info *info)
{
    MTKFB_1_API_ENTRY();

    mtkfb_1_update_screen_impl();

    return 0;
}
*/

static int mtkfb_1_pan_display_impl(struct fb_var_screeninfo *var, struct fb_info *info)
{
    /* Default can not go here. */
    MTKFB_1_MSG("Implemented in V4L2.");

    return -1;
}


static int mtkfb_1_pan_display_proxy(struct fb_var_screeninfo *var, struct fb_info *info)
{
    MTKFB_1_API_ENTRY();

    //~~~ V4L2 used section START {{{
    /* the section code is no need to be contained by macro.*/
    /* To use function pointer call instead of function call can avoid
     *  unknown symbol error when the function is not compiled.
     */
    if (mtkfb_1_hook_pan_display)
    {
        if (0 == (*mtkfb_1_hook_pan_display)(var, info))
        {
            return 0;
        }
    }
    //~~~ V4L2 used section END }}}

    return mtkfb_1_pan_display_impl(var, info);
}

/* Set fb_info.fix fields and also updates fbdev.
 * When calling this fb_info.var must be set up already.
 */
static void set_fb_1_fix(struct mtkfb_device *fbdev)
{
    struct fb_info           *fbi   = fbdev->fb_info;
    struct fb_fix_screeninfo *fix   = &fbi->fix;
    struct fb_var_screeninfo *var   = &fbi->var;
    struct fb_ops            *fbops = fbi->fbops;

    MTKFB_1_API_ENTRY();

    strncpy(fix->id, MTKFB_1_DRIVER, sizeof(fix->id));
    fix->type = FB_TYPE_PACKED_PIXELS;

    switch (var->bits_per_pixel)
    {
    case 16:
    case 24:
    case 32:
        fix->visual = FB_VISUAL_TRUECOLOR;
        break;
    case 1:
    case 2:
    case 4:
    case 8:
        fix->visual = FB_VISUAL_PSEUDOCOLOR;
        break;
    default:
        ASSERT(0);
    }

    fix->accel       = FB_ACCEL_NONE;
    fix->line_length = ALIGN_TO(var->xres_virtual, disphal_get_fb_alignment()) * var->bits_per_pixel / 8;
    fix->smem_len    = fbdev->fb_size_in_byte;
    fix->smem_start  = fbdev->fb_pa_base;

    fix->xpanstep = 0;
    fix->ypanstep = 1;

    fbops->fb_fillrect  = cfb_fillrect;
    fbops->fb_copyarea  = cfb_copyarea;
    fbops->fb_imageblit = cfb_imageblit;
}


/* Check values in var, try to adjust them in case of out of bound values if
 * possible, or return error.
 */
static int mtkfb_1_check_var(struct fb_var_screeninfo *var, struct fb_info *fbi)
{
    unsigned int bpp;
    unsigned long max_frame_size;
    unsigned long line_size;

    struct mtkfb_device *fbdev = (struct mtkfb_device *)fbi->par;

    MTKFB_1_API_ENTRY();

    MTKFB_1_LOG("xres=%u, yres=%u, xres_virtual=%u, yres_virtual=%u, "
              "xoffset=%u, yoffset=%u, bits_per_pixel=%u)\n",
        var->xres, var->yres, var->xres_virtual, var->yres_virtual,
        var->xoffset, var->yoffset, var->bits_per_pixel);

    bpp = var->bits_per_pixel;

    if (bpp != 16 && bpp != 24 && bpp != 32) {
        MTKFB_1_LOG("[%s]unsupported bpp: %d", __func__, bpp);
        return -1;
    }

    switch (var->rotate) {
    case 0:
    case 180:
        var->xres = MTK_FB_XRES;
        var->yres = MTK_FB_YRES;
        break;
    case 90:
    case 270:
        var->xres = MTK_FB_YRES;
        var->yres = MTK_FB_XRES;
        break;
    default:
        return -1;
    }

    if (var->xres_virtual < var->xres)
        var->xres_virtual = var->xres;
    if (var->yres_virtual < var->yres)
        var->yres_virtual = var->yres;

    max_frame_size = fbdev->fb_size_in_byte;
    line_size = var->xres_virtual * bpp / 8;

    if (line_size * var->yres_virtual > max_frame_size) {
        /* Try to keep yres_virtual first */
        line_size = max_frame_size / var->yres_virtual;
        var->xres_virtual = line_size * 8 / bpp;
        if (var->xres_virtual < var->xres) {
            /* Still doesn't fit. Shrink yres_virtual too */
            var->xres_virtual = var->xres;
            line_size = var->xres * bpp / 8;
            var->yres_virtual = max_frame_size / line_size;
        }
    }
    if (var->xres + var->xoffset > var->xres_virtual)
        var->xoffset = var->xres_virtual - var->xres;
    if (var->yres + var->yoffset > var->yres_virtual)
        var->yoffset = var->yres_virtual - var->yres;

    if (16 == bpp) {
        var->red.offset    = 11;  var->red.length    = 5;
        var->green.offset  =  5;  var->green.length  = 6;
        var->blue.offset   =  0;  var->blue.length   = 5;
        var->transp.offset =  0;  var->transp.length = 0;
    }
    else if (24 == bpp)
    {
        var->blue.offset = 0;
        var->green.offset = 8;
        var->red.offset = 16;
        var->red.length = var->green.length = var->blue.length = 8;
        var->transp.length = 0;
    }
    else if (32 == bpp)
    {
        var->blue.offset = 0;
        var->green.offset = 8;
        var->red.offset = 16;
        var->transp.offset = 24;
        var->red.length = var->green.length =
        var->blue.length = var->transp.length = 8;
    }

    var->red.msb_right = var->green.msb_right =
    var->blue.msb_right = var->transp.msb_right = 0;

    var->activate = FB_ACTIVATE_NOW;

    var->height    = UINT_MAX;
    var->width     = UINT_MAX;
    var->grayscale = 0;
    var->nonstd    = 0;

    var->pixclock     = UINT_MAX;
    var->left_margin  = UINT_MAX;
    var->right_margin = UINT_MAX;
    var->upper_margin = UINT_MAX;
    var->lower_margin = UINT_MAX;
    var->hsync_len    = UINT_MAX;
    var->vsync_len    = UINT_MAX;

    var->vmode = FB_VMODE_NONINTERLACED;
    var->sync  = 0;

    return 0;
}


/* Switch to a new mode. The parameters for it has been check already by
 * mtkfb_check_var.
 */
static int mtkfb_1_set_par(struct fb_info *fbi)
{
    MTKFB_1_API_ENTRY();

    set_fb_1_fix(fbi->par);

    return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,7,0)
int mtkfb_1_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
#else
int mtkfb_1_ioctl(struct file *file, struct fb_info *info, unsigned int cmd, unsigned long arg)
#endif
{
    MTKFB_1_API_ENTRY();

    MTKFB_1_LOG("mtkfb_ioctl, info=0x%08x, cmd=0x%08x, arg=0x%08x\n", (unsigned int)info,
        (unsigned int)cmd, (unsigned int)arg);

    switch (cmd)
    {
    default:
        MTKFB_1_LOG("mtkfb_ioctl 0x%08x Not support", (unsigned int)cmd);
        //pr_info("mtkfb_ioctl Not support,MTKFB_QUEUE_OVERLAY_CONFIG=0x%08x info=0x%08x, cmd=0x%08x, arg=0x%08x\n",MTKFB_QUEUE_OVERLAY_CONFIG, (unsigned int)info, (unsigned int)cmd, (unsigned int)arg);
        return -EINVAL;
    }

    return 0;
}

static int mtkfb_1_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
    int ret;
    struct mtkfb_device *fbdev = (struct mtkfb_device *)info->par;
    unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);

    MTKFB_1_API_ENTRY();

    if (size > info->fix.smem_len)
    {
        return -EINVAL;
    }

    ret = remap_vmalloc_range(vma, fbdev->fb_va_base, vma->vm_pgoff);
    if (ret)
    {
        MTKFB_1_MSG("remap_vmalloc_range failed:%d", ret);
        return (-EAGAIN);
    }

    return 0;
}


/* Callback table for the frame buffer framework. Some of these pointers
 * will be changed according to the current setting of fb_info->accel_flags.
 */
static struct fb_ops mtkfb_ops_1 = {
    .owner          = THIS_MODULE,
    .fb_open        = mtkfb_1_open,
    .fb_release     = mtkfb_1_release,
    .fb_setcolreg   = mtkfb_1_setcolreg,
    .fb_pan_display = mtkfb_1_pan_display_proxy,
    .fb_check_var   = mtkfb_1_check_var,
    .fb_set_par     = mtkfb_1_set_par,
    .fb_ioctl       = mtkfb_1_ioctl,
    .fb_mmap        = mtkfb_1_mmap,
};

/*
 * ---------------------------------------------------------------------------
 * Sysfs interface
 * ---------------------------------------------------------------------------
 */

static int mtkfb_1_register_sysfs(struct mtkfb_device *fbdev)
{
    MTKFB_1_API_ENTRY();

    return 0;
}

static void mtkfb_1_unregister_sysfs(struct mtkfb_device *fbdev)
{
    MTKFB_1_API_ENTRY();
}

/*
 * ---------------------------------------------------------------------------
 * LDM callbacks
 * ---------------------------------------------------------------------------
 */
/* Initialize system fb_info object and set the default video mode.
 * The frame buffer memory already allocated by lcddma_init
 */
static int mtkfb_1_fbinfo_init(struct fb_info *info)
{
    struct mtkfb_device *fbdev = (struct mtkfb_device *)info->par;
    int r = 0;

    MTKFB_1_API_ENTRY();

    BUG_ON(!fbdev->fb_va_base);
    info->fbops = &mtkfb_ops_1;
    info->flags = FBINFO_FLAG_DEFAULT;
    info->screen_base = (char *) fbdev->fb_va_base;
    info->screen_size = fbdev->fb_size_in_byte;
    info->pseudo_palette = fbdev->pseudo_palette;

    r = fb_alloc_cmap(&info->cmap, 16, 0);
    if (r != 0)
    {
        MTKFB_1_LOG("unable to allocate color map memory\n");
    }

    return r;
}

/* Release the fb_info object */
static void mtkfb_1_fbinfo_cleanup(struct mtkfb_device *fbdev)
{
    MTKFB_1_API_ENTRY();

    fb_dealloc_cmap(&fbdev->fb_info->cmap);
}

void mtkfb_1_disable_non_fb_layer(void)
{
    MTKFB_1_API_ENTRY();
}

static int mtkfb_1_allocate_fb(struct mtkfb_device *fbdev)
{
    int ret = 0;
    unsigned int alloc_size;
    unsigned int fb_width;
    unsigned int fb_height;
    unsigned int buffer_bpp;
    unsigned int pages;

    MTKFB_1_API_ENTRY();

    ASSERT(fbdev && (0 == fbdev->fb_pa_base) && (0 == fbdev->fb_va_base));

    fb_width = ALIGN_TO(MAX_FB_W, MTK_FB_ALIGNMENT); /* MAX */
    fb_height = ALIGN_TO(MAX_FB_H, MTK_FB_ALIGNMENT); /* MAX */
    buffer_bpp = 4; /* MAX, ARGB8888 */
    pages = FB_PAGES;

    alloc_size = fb_width * fb_height * buffer_bpp * pages;
    MTKFB_1_LOG("init fb_width:%d, fb_height:%d, buffer_bpp:%d, page=%d\n",
        fb_width, fb_height, buffer_bpp, pages);

    // alloc virtual memory
    // NOTICE: m4u need 64 bytes align!!!
    fbdev->fb_va_base = vmalloc_user(alloc_size);

    if (!fbdev->fb_va_base)
    {
        MTKFB_1_LOG("vmalloc %dbytes fail\n", alloc_size);
        return -ENOMEM;
    }

    fbdev->fb_size_in_byte = alloc_size;

    if (m4u_alloc_mva(DISP_WDMA,
                (unsigned int)fbdev->fb_va_base,
                alloc_size,
                0,
                0,
                &fbdev->fb_pa_base))
    {
        vfree((void *)fbdev->fb_va_base);
        fbdev->fb_va_base = 0;
        MTKFB_1_LOG("m4u_alloc_mva for fb1 fail.\n");
        return -ENOMEM;
    }

    m4u_dma_cache_maint(DISP_WDMA,
            (void const *)fbdev->fb_va_base,
            alloc_size,
            DMA_BIDIRECTIONAL);

    MTKFB_1_LOG("M4U alloc mva: 0x%x va: 0x%x size: 0x%x\n",
            fbdev->fb_pa_base, fbdev->fb_va_base, alloc_size);

    return ret;
}

static int mtkfb_1_free_fb(struct mtkfb_device *fbdev)
{
    int ret = 0;

    MTKFB_1_API_ENTRY();

    ASSERT(fbdev);

    if (fbdev->fb_va_base)
    {
        ASSERT(fbdev->fb_pa_base);

        m4u_dealloc_mva(DISP_WDMA,
            (unsigned int)fbdev->fb_va_base,
            fbdev->fb_size_in_byte,
            (unsigned int)fbdev->fb_pa_base);

        vfree((void *)fbdev->fb_va_base);
    }

    fbdev->fb_va_base = 0;
    fbdev->fb_pa_base = 0;
    fbdev->fb_size_in_byte = 0;

    return ret;
}


/* Free driver resources. Can be called to rollback an aborted initialization
 * sequence.
 */
static void mtkfb_1_free_resources(struct mtkfb_device *fbdev, int state)
{
    int r = 0;

    MTKFB_1_API_ENTRY();

    switch (state) {
    case MTKFB_ACTIVE:
        r = unregister_framebuffer(fbdev->fb_info);
        ASSERT(0 == r);
      //lint -fallthrough
    case 5:
        mtkfb_1_unregister_sysfs(fbdev);
      //lint -fallthrough
    case 4:
        mtkfb_1_fbinfo_cleanup(fbdev);
      //lint -fallthrough
    case 3:
      // DISP_CHECK_RET(DISP_Deinit());
      //lint -fallthrough
    case 2:
      mtkfb_1_free_fb(fbdev);
      //lint -fallthrough
    case 1:
        dev_set_drvdata(fbdev->dev, NULL);
        framebuffer_release(fbdev->fb_info);
      //lint -fallthrough
    case 0:
      /* nothing to free */
        break;
    default:
        BUG();
    }
}

/* This API may be one common api if there is more than one frame buffer drivers. */
int mtkfb_1_update_res(struct fb_info *fbi, unsigned int xres, unsigned int yres)
{
    int ret;
    struct mtkfb_device *fbdev;
    struct fb_var_screeninfo *var;
    unsigned int real_size;
    unsigned int bytes_per_pixel = 4; // ARGB8888

    fbdev = (struct mtkfb_device *)fbi->par;
    real_size = xres * yres * bytes_per_pixel * FB_PAGES;
    if (real_size > fbdev->fb_size_in_byte)
    {
        MTKFB_1_MSG("visuable resolution(%d,%d) overflow max(%d,%d)\n",
            xres, yres, MAX_FB_W, MAX_FB_H);
        return -1;
    }

    MTK_FB_XRES  = xres;
    MTK_FB_YRES  = yres;
    fb_xres_update = MTK_FB_XRES;
    fb_yres_update = MTK_FB_YRES;

    MTKFB_1_MSG("[MTKFB1] XRES=%d, YRES=%d\n", MTK_FB_XRES, MTK_FB_YRES);

    MTK_FB_BPP   = bytes_per_pixel;
    MTK_FB_PAGES = FB_PAGES;

    var = &fbi->var;
    memset(var, 0, sizeof(*var));

    var->xres         = MTK_FB_XRES;
    var->yres         = MTK_FB_YRES;
    var->xres_virtual = MTK_FB_XRESV;
    var->yres_virtual = MTK_FB_YRESV;

    var->bits_per_pixel = bytes_per_pixel * 8;
    var->activate = FB_ACTIVATE_NOW;

    ret = mtkfb_1_check_var(var, fbi);
    if (ret != 0)
    {
        MTKFB_1_LOG("failed to mtkfb_check_var:%d\n", ret);
    }

    ret = mtkfb_1_set_par(fbi);
    if (ret != 0)
    {
        MTKFB_1_LOG("failed to mtkfb_set_par:%d\n", ret);
    }

    return ret;
}

/* Called by LDM binding to probe and attach a new device.
 * Initialization sequence:
 *   1. allocate system fb_info structure
 *      select panel type according to machine type
 *   2. init LCD panel
 *   3. init LCD controller and LCD DMA
 *   4. init system fb_info structure
 *   5. init gfx DMA
 *   6. enable LCD panel
 *      start LCD frame transfer
 *   7. register system fb_info structure
 */

static int mtkfb_1_probe(struct device *dev)
{
    struct mtkfb_device    *fbdev;
    struct fb_info         *fbi;
    int                    init_state;
    int                    r = 0;

    MTKFB_1_API_ENTRY();

    init_state = 0;

    fbi = framebuffer_alloc(sizeof(struct mtkfb_device), dev);
    if (!fbi) {
        MTKFB_1_LOG("unable to allocate memory for device info\n");
        r = -ENOMEM;
        goto cleanup;
    }
    mtkfb_1_fbi = fbi;

    fbdev = (struct mtkfb_device *)fbi->par;
    memset(fbdev, 0, sizeof(*fbdev));
    fbdev->fb_info = fbi;
    fbdev->dev = dev;

    dev_set_drvdata(dev, fbdev);

    init_state++;   // 1

    /* Allocate and initialize video frame buffer */
    mtkfb_1_allocate_fb(fbdev);

    MTKFB_1_MSG("[FB Driver] fbdev->fb_pa_base = %x, fbdev->fb_va_base = %x\n", fbdev->fb_pa_base, (unsigned int)(fbdev->fb_va_base));

    if (!fbdev->fb_va_base) {
        MTKFB_1_LOG("unable to allocate memory for frame buffer\n");
        r = -ENOMEM;
        goto cleanup;
    }
    init_state++;   // 2

    init_state++;   // 3

    /* Register to system */

    r = mtkfb_1_fbinfo_init(fbi);
    if (r)
        goto cleanup;
    init_state++;   // 4

    r = mtkfb_1_register_sysfs(fbdev);
    if (r)
        goto cleanup;
    init_state++;   // 5

    r = register_framebuffer(fbi);
    if (r != 0) {
        MTKFB_1_LOG("register_framebuffer failed\n");
        goto cleanup;
    }

    // initialize with max resolution
    mtkfb_1_update_res(fbi, MAX_FB_W, MAX_FB_H);

    fbdev->state = MTKFB_ACTIVE;

    MTKFB_1_MSG("MTK framebuffer initialized vram=%lu\n", fbdev->fb_size_in_byte);

    return 0;

cleanup:
    mtkfb_1_free_resources(fbdev, init_state);

    return r;
}

/* Called when the device is being detached from the driver */
static int mtkfb_1_remove(struct device *dev)
{
    struct mtkfb_device *fbdev = dev_get_drvdata(dev);
    enum mtkfb_state saved_state = fbdev->state;

    MTKFB_1_API_ENTRY();
    /* FIXME: wait till completion of pending events */

    fbdev->state = MTKFB_DISABLED;
    mtkfb_1_free_resources(fbdev, saved_state);

    return 0;
}

/* PM suspend */
static int mtkfb_1_suspend(struct device *pdev, pm_message_t mesg)
{
    MTKFB_1_API_ENTRY();

    return 0;
}

/* PM resume */
static int mtkfb_1_resume(struct device *pdev)
{
    MTKFB_1_API_ENTRY();

    return 0;
}

/*---------------------------------------------------------------------------*/
#ifdef CONFIG_PM
/*---------------------------------------------------------------------------*/
int mtkfb_1_pm_suspend(struct device *device)
{
    struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    MTKFB_1_API_ENTRY();

    return mtkfb_1_suspend((struct device *)pdev, PMSG_SUSPEND);
}

int mtkfb_1_pm_resume(struct device *device)
{
    struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    MTKFB_1_API_ENTRY();

    return mtkfb_1_resume((struct device *)pdev);
}

int mtkfb_1_pm_restore_noirq(struct device *device)
{
    // do nothing
    MTKFB_1_API_ENTRY();

    return 0;
}
/*---------------------------------------------------------------------------*/
#else /*CONFIG_PM*/
/*---------------------------------------------------------------------------*/
#define mtkfb_1_pm_suspend NULL
#define mtkfb_1_pm_resume  NULL
#define mtkfb_1_pm_restore_noirq NULL
/*---------------------------------------------------------------------------*/
#endif /*CONFIG_PM*/
/*---------------------------------------------------------------------------*/
struct dev_pm_ops mtkfb_1_pm_ops = {
    .suspend = mtkfb_1_pm_suspend,
    .resume = mtkfb_1_pm_resume,
    .freeze = mtkfb_1_pm_suspend,
    .thaw = mtkfb_1_pm_resume,
    .poweroff = mtkfb_1_pm_suspend,
    .restore = mtkfb_1_pm_resume,
    .restore_noirq = mtkfb_1_pm_restore_noirq,
};

static struct platform_driver mtkfb_1_driver =
{
    .driver = {
        .name    = MTKFB_1_DRIVER,
#ifdef CONFIG_PM
        .pm     = &mtkfb_1_pm_ops,
#endif
        .bus     = &platform_bus_type,
        .probe   = mtkfb_1_probe,
        .remove  = mtkfb_1_remove,
        .suspend = mtkfb_1_suspend,
        .resume  = mtkfb_1_resume,
    },
};

static struct platform_device mtkfb_1_device = {
    .name = MTKFB_1_DRIVER,
    .id   = 0,
};

int __init mtkfb_1_init(void)
{
    int ret;

    ret = platform_device_register(&mtkfb_device);
    // return 0: success, non-0: failed.
    printk("[%s] dev register return 0x%08X (%d)\n", __FUNCTION__, ret, ret);

    ret = platform_driver_register(&mtkfb_1_driver);
    // return 0: success, non-0: failed.
    printk("[%s] drv register return 0x%08X (%d)\n", __FUNCTION__, ret, ret);

    return ret;
}

static void __exit mtkfb_1_cleanup(void)
{
	platform_driver_unregister(&mtkfb_1_driver);
    platform_device_unregister(&mtkfb_1_device);
}

late_initcall(mtkfb_1_init);
module_exit(mtkfb_1_cleanup);

MODULE_DESCRIPTION("MEDIATEK framebuffer driver[1]");
MODULE_AUTHOR("Houlong Wei <houlong.wei@mediatek.com>");
MODULE_LICENSE("GPL");

