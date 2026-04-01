#ifndef VELADEMO_CONFIG_H
#define VELADEMO_CONFIG_H

// 颜色定义
#define COLOR_BACKGROUND        0x000000  ///< 背景色
#define COLOR_SECONDARY_BG      0x1a1a1a  ///< 次要背景色
#define COLOR_APP_BG            0x2d2d2d  ///< 应用界面背景色
#define COLOR_PRIMARY           0x4CAF50  ///< 主色调（绿色）
#define COLOR_ACCENT            0x2196F3  ///< 强调色（蓝色）
#define COLOR_TEXT_PRIMARY      0xFFFFFF  ///< 主要文字颜色
#define COLOR_TEXT_SECONDARY    0xCCCCCC  ///< 次要文字颜色
#define COLOR_TEXT_DISABLED     0x888888  ///< 禁用文字颜色

// 尺寸定义
#define SCREEN_WIDTH            LV_HOR_RES ///< 屏幕宽度
#define SCREEN_HEIGHT           LV_VER_RES ///< 屏幕高度
#define TITLE_BAR_HEIGHT        40         ///< 标题栏高度
#define ALBUM_COVER_SIZE        130        ///< 专辑封面尺寸
#define CONTROL_BUTTON_SIZE     50         ///< 控制按钮尺寸
#define PLAY_BUTTON_SIZE        60         ///< 播放按钮尺寸
#define VOLUME_SLIDER_HEIGHT    15         ///< 音量滑块高度

// 默认值
#define DEFAULT_VOLUME          50         ///< 默认音量
#define DEFAULT_PLAY_MODE       PLAY_MODE_ORDER ///< 默认播放模式

#endif /* VELADEMO_CONFIG_H */