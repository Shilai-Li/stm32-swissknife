/**
 * @file lvgl.h
 * Include all LVGL related headers
 */

#ifndef LVGL_H
#define LVGL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************
 * CURRENT VERSION OF LVGL
 ***************************/
#include "lv_version.h"

/*********************
 *      INCLUDES
 *********************/
#include "csrc/lv_init.h"

#include "csrc/stdlib/lv_mem.h"
#include "csrc/stdlib/lv_string.h"
#include "csrc/stdlib/lv_sprintf.h"

#include "csrc/misc/lv_log.h"
#include "csrc/misc/lv_timer.h"
#include "csrc/misc/lv_math.h"
#include "csrc/misc/lv_array.h"
#include "csrc/misc/lv_async.h"
#include "csrc/misc/lv_anim_timeline.h"
#include "csrc/misc/lv_profiler_builtin.h"
#include "csrc/misc/lv_rb.h"
#include "csrc/misc/lv_utils.h"
#include "csrc/misc/lv_iter.h"
#include "csrc/misc/lv_circle_buf.h"
#include "csrc/misc/lv_tree.h"

#include "csrc/osal/lv_os.h"

#include "csrc/tick/lv_tick.h"

#include "csrc/core/lv_obj.h"
#include "csrc/core/lv_group.h"
#include "csrc/indev/lv_indev.h"
#include "csrc/indev/lv_indev_gesture.h"
#include "csrc/core/lv_refr.h"
#include "csrc/display/lv_display.h"

#include "csrc/font/lv_font.h"
#include "csrc/font/lv_binfont_loader.h"
#include "csrc/font/lv_font_fmt_txt.h"

#include "csrc/widgets/animimage/lv_animimage.h"
#include "csrc/widgets/arc/lv_arc.h"
#include "csrc/widgets/arclabel/lv_arclabel.h"
#include "csrc/widgets/bar/lv_bar.h"
#include "csrc/widgets/button/lv_button.h"
#include "csrc/widgets/buttonmatrix/lv_buttonmatrix.h"
#include "csrc/widgets/calendar/lv_calendar.h"
#include "csrc/widgets/canvas/lv_canvas.h"
#include "csrc/widgets/chart/lv_chart.h"
#include "csrc/widgets/checkbox/lv_checkbox.h"
#include "csrc/widgets/dropdown/lv_dropdown.h"
#include "csrc/widgets/image/lv_image.h"
#include "csrc/widgets/imagebutton/lv_imagebutton.h"
#include "csrc/widgets/keyboard/lv_keyboard.h"
#include "csrc/widgets/label/lv_label.h"
#include "csrc/widgets/led/lv_led.h"
#include "csrc/widgets/line/lv_line.h"
#include "csrc/widgets/list/lv_list.h"
#include "csrc/widgets/lottie/lv_lottie.h"
#include "csrc/widgets/menu/lv_menu.h"
#include "csrc/widgets/msgbox/lv_msgbox.h"
#include "csrc/widgets/roller/lv_roller.h"
#include "csrc/widgets/scale/lv_scale.h"
#include "csrc/widgets/slider/lv_slider.h"
#include "csrc/widgets/span/lv_span.h"
#include "csrc/widgets/spinbox/lv_spinbox.h"
#include "csrc/widgets/spinner/lv_spinner.h"
#include "csrc/widgets/switch/lv_switch.h"
#include "csrc/widgets/table/lv_table.h"
#include "csrc/widgets/tabview/lv_tabview.h"
#include "csrc/widgets/textarea/lv_textarea.h"
#include "csrc/widgets/tileview/lv_tileview.h"
#include "csrc/widgets/win/lv_win.h"
#include "csrc/widgets/3dtexture/lv_3dtexture.h"

#include "csrc/others/snapshot/lv_snapshot.h"
#include "csrc/others/sysmon/lv_sysmon.h"
#include "csrc/others/monkey/lv_monkey.h"
#include "csrc/others/gridnav/lv_gridnav.h"
#include "csrc/others/fragment/lv_fragment.h"
#include "csrc/others/imgfont/lv_imgfont.h"
#include "csrc/others/observer/lv_observer.h"
#include "csrc/others/ime/lv_ime_pinyin.h"
#include "csrc/others/file_explorer/lv_file_explorer.h"
#include "csrc/others/font_manager/lv_font_manager.h"
#include "csrc/others/translation/lv_translation.h"
#include "csrc/others/xml/lv_xml.h"
#include "csrc/others/test/lv_test.h"

#include "csrc/libs/barcode/lv_barcode.h"
#include "csrc/libs/bin_decoder/lv_bin_decoder.h"
#include "csrc/libs/bmp/lv_bmp.h"
#include "csrc/libs/rle/lv_rle.h"
#include "csrc/libs/fsdrv/lv_fsdrv.h"
#include "csrc/libs/lodepng/lv_lodepng.h"
#include "csrc/libs/libpng/lv_libpng.h"
#include "csrc/libs/gltf/gltf_data/lv_gltf_model.h"
#include "csrc/libs/gltf/gltf_view/lv_gltf.h"
#include "csrc/libs/gif/lv_gif.h"
#include "csrc/libs/gstreamer/lv_gstreamer.h"
#include "csrc/libs/qrcode/lv_qrcode.h"
#include "csrc/libs/tjpgd/lv_tjpgd.h"
#include "csrc/libs/libjpeg_turbo/lv_libjpeg_turbo.h"
#include "csrc/libs/freetype/lv_freetype.h"
#include "csrc/libs/rlottie/lv_rlottie.h"
#include "csrc/libs/ffmpeg/lv_ffmpeg.h"
#include "csrc/libs/tiny_ttf/lv_tiny_ttf.h"
#include "csrc/libs/svg/lv_svg.h"
#include "csrc/libs/svg/lv_svg_render.h"

#include "csrc/layouts/lv_layout.h"

#include "csrc/draw/lv_draw_buf.h"
#include "csrc/draw/lv_draw_vector.h"
#include "csrc/draw/sw/lv_draw_sw_utils.h"
#include "csrc/draw/eve/lv_draw_eve_target.h"

#include "csrc/themes/lv_theme.h"

#include "csrc/drivers/lv_drivers.h"

/* Define LV_DISABLE_API_MAPPING using a compiler option 
 * to make sure your application is not using deprecated names */
#ifndef LV_DISABLE_API_MAPPING
    #include "csrc/lv_api_map_v8.h"
    #include "csrc/lv_api_map_v9_0.h"
    #include "csrc/lv_api_map_v9_1.h"
    #include "csrc/lv_api_map_v9_2.h"
    #include "csrc/lv_api_map_v9_3.h"
#endif /*LV_DISABLE_API_MAPPING*/

#if LV_USE_PRIVATE_API
#include "csrc/lvgl_private.h"
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/** Gives 1 if the x.y.z version is supported in the current version
 * Usage:
 *
 * - Require v6
 * #if LV_VERSION_CHECK(6,0,0)
 *   new_func_in_v6();
 * #endif
 *
 *
 * - Require at least v5.3
 * #if LV_VERSION_CHECK(5,3,0)
 *   new_feature_from_v5_3();
 * #endif
 *
 *
 * - Require v5.3.2 bugfixes
 * #if LV_VERSION_CHECK(5,3,2)
 *   bugfix_in_v5_3_2();
 * #endif
 *
 */
#define LV_VERSION_CHECK(x,y,z) (x == LVGL_VERSION_MAJOR && (y < LVGL_VERSION_MINOR || (y == LVGL_VERSION_MINOR && z <= LVGL_VERSION_PATCH)))

/**
 * Wrapper functions for VERSION macros
 */

static inline int lv_version_major(void)
{
    return LVGL_VERSION_MAJOR;
}

static inline int lv_version_minor(void)
{
    return LVGL_VERSION_MINOR;
}

static inline int lv_version_patch(void)
{
    return LVGL_VERSION_PATCH;
}

static inline const char * lv_version_info(void)
{
    return LVGL_VERSION_INFO;
}

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVGL_H*/
