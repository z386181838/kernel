#ifndef __MTKFB_1_H
#define __MTKFB_1_H


#if 1
#define mtkfb_1_device mtkfb_device
#else
struct mtkfb_1_device {
    int             state;                  /* mtkfb_state */
    unsigned int    fb_va_base;             /* vitrual address base */
    unsigned int    fb_pa_base;             /* mva address base */
    unsigned long   fb_size_in_byte;        /* frame buffers total size */
    //unsigned long   layer_enable;
    //MTK_FB_FORMAT   *layer_format;
    //unsigned int    layer_config_dirty;

    u32             pseudo_palette[17];

    struct fb_info *fb_info;                /* Linux fbdev framework data */
    struct device  *dev;
};
#endif

#define MTKFB_1_DRIVER "mtkfb1"

#define MTKFB_1_LOG(fmt, arg...) \
    do { \
        if (mtkfb_1_log_on) DISP_LOG_PRINT(ANDROID_LOG_WARN, "MTKFB1", fmt, ##arg); \
    }while (0)


#define MTKFB_1_API_ENTRY()    \
    do { \
        if(mtkfb_1_log_on) {pr_err("[MTKFB1] %s, %d\n", __FUNCTION__, __LINE__);} \
    }while (0)

#define MTKFB_1_MSG(fmt, arg...) \
    do { \
        pr_err("[MTKFB1]"fmt, ##arg); \
    }while (0)

#define MTKFB_1_CHECK_RET(expr)    \
    do {                   \
        int ret = (expr);  \
        ASSERT(0 == ret);  \
    } while (0)

#endif /* __MTKFB_1_H */
