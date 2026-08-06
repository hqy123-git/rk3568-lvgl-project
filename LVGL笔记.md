
# 1. LVGL简介
## 1.1 什么是LVGL？
* **LVGL** (Light and Versatile Graphics Library) 是一个开源、轻量级的 GUI 图形库，广泛应用在嵌入式设备上。
* 特点：

  * **轻量级**：相比 Qt 等 GUI 框架更适合资源受限的单片机与嵌入式 Linux。
  * **跨平台**：支持单片机（裸机/RTOS）和 Linux。
  * **纯 C 语言实现**，方便移植和扩展。

## 1.2 与其他 GUI 框架对比（如 Qt）

| 特性    | LVGL               | Qt                    |
| ----- | ------------------ | --------------------- |
| 代码规模  | 轻量，核心库约几百 KB       | 庞大，库体积数十 MB           |
| 资源需求  | 适合单片机、低内存嵌入式 Linux | 需要较大内存和存储空间           |
| 开发语言  | C                  | C++                   |
| 可移植性  | 单片机、RTOS、Linux     | 主要面向 PC 与高性能嵌入式 Linux |
| UI 特性 | 基本控件、动画、样式系统       | 功能丰富，支持 QML、高级控件      |
| 学习曲线  | 相对简单，API 精简        | 功能复杂，学习曲线陡峭           |

总结：**Qt 更适合资源充足的嵌入式 Linux 或 PC 应用，而 LVGL 更适合资源受限的单片机和轻量 UI 应用场景。**

## 1.3 LVGL 应用场景

* 智能家居面板
* 工业控制屏幕
* 医疗设备 UI
* 车载中控系统


# 2. LVGL的移植
请参考《附件3-移植LVGL到嵌入式linux开发板.md》
建议：用 Source insight或vscode 把移植后的LVGL源代码建立一个工程



# 3. LVGL 运行机制
## 3.1  使用LVGL的代码总框架

```c++

int main()
{ 
    //... 你的初始化部分...如： LED

    //1. 初始化 LVGL库


    //2. Application's Job 应用程序的任务 (创建你的UI)


    //3. 事件循环


    return 0;

}

```

### 3.1.1 LVGL的初始化工作
> 主要初始化三个东西： LVGL库、显示器、输入设备。

#### (1) lv_init
```c++

/**
 * Initialize LVGL library.
 * Should be called before any other LVGL related function.
 */
void lv_init(void);
```

#### (2) 初始化显示器

什么是显示器?
> 在LVGL中， **lv_display_t**(不要与 Screen混淆)是一个数据类型，表示一个
> 一个单独的显示面板--即硬件设备，用于在你的设备上显示 LVGL渲染的像素。
> 在系统初始化期间，您必须为每个希望 LVGL使用的显示面板执行以下操作:
- 为其创建 lv_display_t 对象，用于描述你的一个显示面板(显示器)
- 设置显示器对应的设备文件

**为你的显示硬件分配一个 lv_display_t的对象**

> 调用 lv_linux_fbdev_create 初始化显示设备(linux framebuffer device),该函数返回一个指向
> lv_display_t 的指针。 在LVGL中用 结构体 lv_display_t 来描述一个显示屏
>
> >   lv_display_t这个结构体比较复杂，大家可以不用深究其内部成员!

```c++
  函数lv_linux_fbdev_create 用来分配一个 显示器结构体 lv_display_t
lv_display_t * lv_linux_fbdev_create(void);

```
> 在调用 lv_linux_fbdev_create 这个函数后，要调用另外一个函数:
>  lv_linux_set_file 来设置显示器对应的设备文件

```c++
  /*
    lv_linux_set_file 来设置显示习对应的设备文件
    @disp: 指向显示屏结构体lv_display_t
    @file: 该显示屏对应的设备文件名，如: "/dev/fb0"
  */
void lv_linux_fbdev_set_file(lv_display_t * disp, const char * file);

```

#### (3) 初始化输入设备
> 调用 lv_evdev_create来创建一个输入设备，在LVGL中用结构体 lv_indev_t
> 来描述一个输入设备。
>
> >   lv_evdev_create用来分配并返回一个指向 lv_indev_t 结构体的指针


```c++

/**
 * Create evdev input device.
 * @param type LV_INDEV_TYPE_POINTER or LV_INDEV_TYPE_KEYPAD
 *       
 * @param dev_path device path, e.g., /dev/input/event0  GEC6818是  event0 RK3568是 event6
 *        
 * @return pointer to input device or NULL if opening failed
 */
lv_indev_t * lv_evdev_create(lv_indev_type_t indev_type, const char * dev_path);
	indev_type： 输入设备的类型

输入设备的类型如下:
/** Possible input device types*/
typedef enum {
    LV_INDEV_TYPE_NONE,    /**< Uninitialized state*/
    LV_INDEV_TYPE_POINTER, /**< Touch pad, mouse, external button*/ 鼠标、触摸板、触摸屏
    LV_INDEV_TYPE_KEYPAD,  /**< Keypad or keyboard*/ 键盘
    LV_INDEV_TYPE_BUTTON,  /**< External (hardware button) which is assigned to a specific point of the screen*/
    LV_INDEV_TYPE_ENCODER, /**< Encoder with only Left, Right turn and a Button*/
} lv_indev_type_t;
	dev_path：输入设备对应的文件
        GEC6818是  /dev/input/event0 RK3568是 /dev/input/event6

```

### 3.1.2 Application's Job(应用程序的任务)
> 初始化之后，应用程序的任务仅仅是在需要时创建控件树，管理这些控件因用户交互及其他情况而生成的事件，并
> 在不需要时删除它们。 其余的工作则由 LVGL负责。


### 3.1.3 事件循环
> LVGL 运行的核心是  **事件循环**，通过不断地调用  **lv_timer_handler()** 来处理事件和刷新界面

```c++

while(1)
{
  lv_timer_handler() ;//让LVGL去处理事件和刷新界面

  usleep(5000); //防止CPU占用率过高
}
```

```c++
/**
 * Call it periodically to handle lv_timers.
 * @return time till it needs to be run next (in ms)
 */
LV_ATTRIBUTE_TIMER_HANDLER uint32_t lv_timer_handler(void);

```

关键点:
*  **单线程运行**, 任务由 `lv_timer_handler`轮询处理
*  需要保证定期调用该函数，否则会导致事件堆积和 UI卡顿。


# 4. LVGL中的主要概念(Major Concepts)
## 4.1 Screen Widgets(屏幕控件)
> Screen 是 Screen Widget 的缩写。
> Screen 只是任何没有父控件的控件。所以，Screen是其他控制的“根”.
>
> >     a "Screen" is simply any Widget created without a parent --- the "root" of each Widget Tree.

所有 Screens：
- 在创建 Screen 时自动附加到 default_display；
- 自动占用关联显示的整个区域；
- 不能被移动，即不能在屏幕上使用诸如 lv_obj_set_pos() 和 lv_obj_set_size() 等函数。


### 4.1.2 Display 与 Screen 的区别
> Display(  显示设备 或 显示面板) 是用于显示像素的物理硬件。
>  一个 Display (lv_display) 对象是内存中的一个对象，它代表一个供 LVGL 使用的 显示设备。
> Screen（ 屏幕 ）是上述控件树中的 “根” 控件，并且 “依附于” 特定的 Display (lv_display) 对象。
```
   Display
      |
     --- （一个或多个）
     /|\
Screen Widgets  （Widget 树的根）
      |
      O  （零个或多个）
     /|\
Child Widgets
```

### 4.1.3 如何创建Screen呢?

> 在LVGL中用，所有控件(widgets,包括：screen digets, button,label, ...)都用结构体 lv_obj_t 来描述
> 用函数 lv_obj_create 来创建一个控件
> 在用 lv_obj_create创建一个screen widget时，父控件必须指定为NULL(因为screen widgets它没有父控件 )

```c++
lv_obj_t* scr = lv_obj_create(NULL);
```
当然你也可以删除Screen:
```c++
    lv_obj_delete(scr); //删除scr指向的screen对象

        //但请确保不要删除 Active Screen.
```


什么是Active Screen(活动屏幕)?

### 4.1.4 Active Screen
> 虽然每个 Display (lv_display) 对象可以有关联的任意数量的 Screen Widgets，
> 但在任何给定时间只考虑其中一个 Screen 是“Active”的。该 Screen 被称为 Display 的“Active Screen”。
> 因此，一次只有一个 Screen 及其子 Widgets 会在一个显示器(display)上显示。
> 当每个 Display (lv_display) 对象被创建时，会与之一起创建一个默认屏幕，并设置为其“Active Screen”。
> 要获取指向“Active Screen”的指针，请调用 lv_screen_active()。
> 要将一个 Screen 设置为“Active Screen”，请调用 lv_screen_load()。

```c++

  lv_screen_active() 这个用来获取(返回)当前 “Active Screen”的指针

  lv_screen_load() 这个用来把一个指定的screen 设置(加载)为 “Active Screen”
```

### 4.1.5 Display的Screen Layer(屏幕层)
>  当一个 lv_display_t(Display) 对象被创建时，LVGL会为其创建 4 个永久的 Screens 并附加到它上面，这些屏幕有助于实现层次结构。

- 1. Bottom Layer Screen    底层（位于活动屏幕下方，透明、不可滚动，但可以点击）
- 2. Active Screen
- 3. Top Layer Screen  顶层（位于活动屏幕上方，透明且不可滚动或点击）
- 4. System Layer Screen   系统层（位于顶层上方，透明且不可滚动或点击）

1、3 和 4 层是独立于 Active Screen 的，它们会被显示（如果它们包含任何可见内容），无论当前活动屏幕是什么。

覆盖层次： User眼睛  -> 4 (System Layer) --> 3 (Top Layer) --> 2 (Active Screen) --> 1 (Bottom Layer)
备注:

> 为了使底层可见，活动屏幕的背景必须至少部分透明，甚至完全透明。
> 用户可以创建Screen, 也可以将其设置为Active Screen，但：
> 用户不能创建 System Layer Screen、Top Layer Screen、 Bottom Layer Screen !
> 因为:
>
> > LVGL 使用顶层和系统层来确保某些 Widgets（控件） 始终 位于其他层之上。
> >  你可以自由地将“弹出窗口”添加到 顶层。顶层旨在用于创建在显示器上所有屏幕上都可见的 Widgets。
> >  而 系统层 则用于系统级别的内容（例如，鼠标光标将通过 lv_indev_set_cursor() 被放置在该层）。
> > 这些层像任何其他 Widget 一样工作，这意味着它们有样式，并且可以在它们上创建任何类型的 Widget

你可以通过以下方式获取 default_display 上这些屏幕的指针（分别是）：
```c++
    lv_screen_active()，
    lv_layer_top()，
    lv_layer_sys()，和
    lv_layer_bottom()。
```
你也可以通过以下方式获取指定显示器上的这些屏幕的指针（分别是）：
```c++
    lv_display_get_screen_active(disp)，
    lv_display_get_layer_top(disp)，
    lv_display_get_layer_sys(disp)，和
    lv_display_get_layer_bottom(disp)。
```
要将创建的屏幕设置为 Active Screen，调用 lv_screen_load() 。


## 4.2 Widgets(控件)
### 4.2.1 基本概念
> 在初始化 LVGL 之后, 为了创建一个交互式用户界面，应用程序接下来会创建一个控件树，LVGL 可以将其渲染到相关显示器上，并与用户进行交互。
> 控件是“智能”的 LVGL 图形元素，例如： Screen、按钮、标签、复选框、开关、滑块、图表等。
> 为了构建这个控件树(Widget Tree)，应用程序首先获取一个屏幕控件(Screen Widget)的指针。
> 系统设计人员可以自由使用与 Display (lv_display) 一起创建的默认屏幕和/或创建自己的屏幕。
> 要创建一个新的Screen Widget，只需在创建控件时将父控件参数传递为 NULL。
> 然后，应用程序将其他控件(widgets)作为树中的子节点添加到这个Screen中。
> Widget在创建时会自动作为子节点添加到其父控件中——--控件的父级作为创建控件件的函数的第一个参数传递。
> 添加后，我们说父控件“包含”子控件。

任何控件都可以包含其他控件。例如，如果您希望按钮具有文本，请创建一个标签控件件并将其作为子节点添加到按钮中。
每个子控件件都成为其父控件的“一部分”。由于这种关系：

>    当父控件移动时，其子控件也会随之移动；
>    当父控件被删除时，其子控件也会随之删除；
>    子控件仅在其父控件的边界内可见；任何超出父控件边界的子控件部分都会被裁剪（即不渲染）。

屏幕（及其子控件）可以在任何时候创建和删除， 除了 当屏幕是 活动屏幕 时。

如何创建控件(widgets)呢?
### 4.2.2 创建控件(widgets)的函数

```c++
lv_<type>_create(parent)
```
该调用将返回一个 lv_obj_t * 指针，稍后可以使用该指针引用小控件以设置其属性。
<type>为要创建的控件(widget)的类型，如: button,lable,slider, checkbox, textarea, image, imagebuttton, ...

```c++

    lv_button_create(lv_obj_t * parent)
    lv_label_create(lv_obj_t * parent)
    lv_slider_create(lv_obj_t * parent)
    lv_checkbox_create(lv_obj_t * parent)
    lv_textarea_create(lv_obj_t * parent)
    lv_image_create(lv_obj_t * parent)
    lv_imagebutton_create(lv_obj_t * parent)
```


例如：
```c++
lv_obj_t * slider1 = lv_slider_create( lv_screen_active()  );
lv_obj_t * btn = lv_btn_create(lv_screen_active());  //lv_button_create
```
### 4.2.3  删除控件(widgets)的函数
要删除任何控件及子控件:
```c++
lv_obj_delete(lv_obj_t * widget)
```

### 4.2.4 修改控件(widgets)的函数
所有控件的通用属性通过如下函数设置:
```c++
lv_obj_set_<attribute_name>(widget, <value>)//设置控件xx属性的值
lv_obj_get_<attribute_name>(widget)  //获取控件的xx属性的值
```

常用函数接口:
```c++
/**
 * Set the size of an object.
 * @param obj       pointer to an object
 * @param w         the new width
 * @param h         the new height
 * @note            possible values are:
 *                  pixel               simple set the size accordingly
 *                  LV_SIZE_CONTENT     set the size to involve all children in the given direction
 *                  lv_pct(x)           to set size in percentage of the parent's content area size (the size without paddings).
 *                                      x should be in [0..1000]% range
 * 
 *            宽度 和 高度 的值可以是:
 *                pixel 像素，如: 100
 *                LV_SIZE_CONTENT    根据控件的内容大小自行设置宽或高
 *                lv_pct(x)      percentage 百分比 ,宽度 或  高度  占父控件的百分比  ,如 : lv_pct(50)
 */
void lv_obj_set_size(lv_obj_t * obj, int32_t w, int32_t h);

/**
 * Set the width of an object
 * @param obj       pointer to an object
 * @param w         the new width
 * @note            possible values are:
 *                  pixel               simple set the size accordingly
 *                  LV_SIZE_CONTENT     set the size to involve all children in the given direction
 *                  lv_pct(x)           to set size in percentage of the parent's content area size (the size without paddings).
 *                                      x should be in [0..1000]% range
 */
void lv_obj_set_width(lv_obj_t * obj, int32_t w);

/**
 * Set the height of an object
 * @param obj       pointer to an object
 * @param h         the new height
 * @note            possible values are:
 *                  pixel               simple set the size accordingly
 *                  LV_SIZE_CONTENT     set the size to involve all children in the given direction
 *                  lv_pct(x)           to set size in percentage of the parent's content area size (the size without paddings).
 *                                      x should be in [0..1000]% range
 */
void lv_obj_set_height(lv_obj_t * obj, int32_t h);


/**
 * Set the position of an object relative to the set alignment.
 * @param obj       pointer to an object
 * @param x         new x coordinate
 * @param y         new y coordinate
 * @note            With default alignment it's the distance from the top left corner
 * @note            E.g. LV_ALIGN_CENTER alignment it's the offset from the center of the parent
 * @note            The position is interpreted on the content area of the parent
 * @note            The values can be set in pixel or in percentage of parent size with `lv_pct(v)`
 */
void lv_obj_set_pos(lv_obj_t * obj, int32_t x, int32_t y);

/**
 * Set the x coordinate of an object
 * @param obj       pointer to an object
 * @param x         new x coordinate
 * @note            With default alignment it's the distance from the top left corner
 * @note            E.g. LV_ALIGN_CENTER alignment it's the offset from the center of the parent
 * @note            The position is interpreted on the content area of the parent
 * @note            The values can be set in pixel or in percentage of parent size with `lv_pct(v)`
 */
void lv_obj_set_x(lv_obj_t * obj, int32_t x);

/**
 * Set the y coordinate of an object
 * @param obj       pointer to an object
 * @param y         new y coordinate
 * @note            With default alignment it's the distance from the top left corner
 * @note            E.g. LV_ALIGN_CENTER alignment it's the offset from the center of the parent
 * @note            The position is interpreted on the content area of the parent
 * @note            The values can be set in pixel or in percentage of parent size with `lv_pct(v)`
 */
void lv_obj_set_y(lv_obj_t * obj, int32_t y);
```


- **Alignment 对齐 及位置**

对齐方式有如下:
```c++
    LV_ALIGN_TOP_LEFT
    LV_ALIGN_TOP_MID
    LV_ALIGN_TOP_RIGHT
    LV_ALIGN_BOTTOM_LEFT
    LV_ALIGN_BOTTOM_MID
    LV_ALIGN_BOTTOM_RIGHT
    LV_ALIGN_LEFT_MID
    LV_ALIGN_RIGHT_MID
    LV_ALIGN_CENTER
```
```c++

//修改控件的 对齐方式
/**
 * Change the alignment of an object.
 * @param obj       pointer to an object to align
 * @param align     type of alignment (see 'lv_align_t' enum) `LV_ALIGN_OUT_...` can't be used.
 */
void lv_obj_set_align(lv_obj_t * obj, lv_align_t align);
```




##  4.3 Events(事件)
> 事件用于通知应用程序某个控件发生了某些事情。
> 你可以为控件添加一个或多个回调函数，当控件被点击、释放、拖动、删除等等时，
> 这些回调函数将被调用。

函数接口: （如何为一个控件添加 相应事件的  回调函数?)
```c++

/**
 * Add an event handler function for an object.
 * Used by the user to react on event which happens with the object.
 * An object can have multiple event handler. They will be called in the same order as they were added.
 * @param obj       pointer to an object
 * @param filter    an event code (e.g. `LV_EVENT_CLICKED`) on which the event should be called. `LV_EVENT_ALL` can be used to receive all the events.
 * @param event_cb  the new event function
 * @param           user_data custom data data will be available in `event_cb`
 */
void lv_obj_add_event_cb(lv_obj_t * obj, lv_event_cb_t event_cb, lv_event_code_t filter,
                         void * user_data);


typedef void (*lv_event_cb_t)(lv_event_t * e); //回调函数类型

    lv_event_t
    struct _lv_event_t {
        void * current_target;	//当前事件的对象
        ...
        lv_event_code_t code;	//发生的事件码
        void * user_data;		//注册/添加事件的时候传入的用户数据地址
        ...
    };
filter:事件编码,指定控件具体发生的事件
      		typedef enum {
                LV_EVENT_ALL = 0,

                /** Input device events*/
                LV_EVENT_PRESSED,            按下事件
                LV_EVENT_CLICKED,            点击事件
                LV_EVENT_RELEASED,           松开事件
                ...
                LV_EVENT_FOCUSED,            聚焦事件
    			      LV_EVENT_DEFOCUSED,          失去焦点
                
			}lv_event_code_t;
		user_data:用户需要传递到回调函数中的数据
	
	// 示例:我们关心一个按键的点击事件,点击按钮会打印数据到终端
	// 	lv_obj_add_event_cb(but, 		//绑定哪一个按钮
	// 					my_event_cb, 	//按钮发生事件之后做什么
	// 					LV_EVENT_CLICKED,  //关心点击事件
    //                      NULL);			//根据实际情况传递
    //   	//自己实现的回调函数
    //     static void my_event_cb(lv_event_t *e)
    //     {
    //     	printf("clicked button!");
    //     }
```



## 4.4 States (状态)
控件可以处于以下状态的组合中：
```c++
    LV_STATE_DEFAULT: 正常，释放状态
    LV_STATE_CHECKED: 切换或选中状态
    LV_STATE_FOCUSED: 通过键盘或编码器聚焦或通过触摸板/鼠标点击
    LV_STATE_FOCUS_KEY: 通过键盘或编码器聚焦，但不通过触摸板/鼠标
    LV_STATE_EDITED: 通过编码器编辑
    LV_STATE_HOVERED: 被鼠标悬停
    LV_STATE_PRESSED: 正在被按下
    LV_STATE_SCROLLED: 正在滚动
    LV_STATE_DISABLED: 禁用
```
例如，如果按下一个控件，它将自动进入 LV_STATE_FOCUSED 和 LV_STATE_PRESSED 状态，当你释放它时，LV_STATE_PRESSED 状态将被移除，
而 LV_STATE_FOCUSED 状态仍然保持活动。



跟控件状态相关的函数接口，如下:

```c++

/**
 * lv_obj_has_state: 检查指定的控件是否处于指定的状态
 * Check if the object is in a given state or not.
 * @param obj       pointer to an object
 * @param state     a state or combination of states to check
 * @return          true: `obj` is in `state`; false: `obj` is not in `state`
 */
bool lv_obj_has_state(const lv_obj_t * obj, lv_state_t state);


/**
 * Add one or more states to the object. The other state bits will remain unchanged.
 * If specified in the styles, transition animation will be started from the previous state to the current.
 * @param obj       pointer to an object
 * @param state     the states to add. E.g `LV_STATE_PRESSED | LV_STATE_FOCUSED`
 */
void lv_obj_add_state(lv_obj_t * obj, lv_state_t state);


/**
 * Remove one or more states to the object. The other state bits will remain unchanged.
 * If specified in the styles, transition animation will be started from the previous state to the current.
 * @param obj       pointer to an object
 * @param state     the states to add. E.g `LV_STATE_PRESSED | LV_STATE_FOCUSED`
 */
void lv_obj_remove_state(lv_obj_t * obj, lv_state_t state);
```

## 4.5 Styles(样式)
样式设计是指对控件进行界面美化，例如设计颜色、边框、字体等等，如果控件没有指定特别的样式，就会
继承父控件的样式， 样式在LVGL中用结构体 lv_style_t 来描述。一个样式可以设置到多个控件中去。
一般来说，定义样式对象时，不能是普通的局部变量，一般设置为全局变量或静态变量，因为全局变量或静态
变量能够保证函数结束之后，样式对象不被销毁。

使用样式来设计坐标等属性，具有一些很大的优势:
- 使得可以很容易的为多个对象同时设置宽度、高度等
- 还可以在一个位置修改值
- 这些数值可以部分地被其他样式覆盖
- 对象的位置或大小可以根据状态而有所不同
- 可以使用样式转换使坐标变化更加平滑

使用样式的步骤(大概有三个步骤):
- (1) 初始化样式(定义并且初始化变量)
```c++
lv_style_t style1; //一般定义为全局变量 或 静态变量
lv_style_init(&style1); //初始化样式

/**
 * Initialize a style
 * @param style pointer to a style to initialize
 * @note Do not call `lv_style_init` on styles that already have some properties
 *       because this function won't free the used memory, just sets a default state for the style.
 *       In other words be sure to initialize styles only once!
 */
void lv_style_init(lv_style_t * style);
```

- (2) 设置样式(设置样式变量的数据)
> 样式(style)就是属性的集合。如：样式中有 size, bg_color, border, ...
> 样式的各种设置样式的函数:   lv_style_set_xxx

```c++
//设置背景颜色
void lv_style_set_bg_color(lv_style_t * style, lv_color_t value);
      style: 你要设置的格式变量指针
      value: 指定颜色
          typedef struct  {
              uint8_t  blue;
              uint8_t  green;
              uint8_t  red;
          }lv_color_t;
      示例: 
            //0x234567
        lv_color_t value = {0x67,0x45, 0x23};
        lv_style_set_bg_color(&style1, value);

        或者
        lv_style_set_bg_color(&style1, lv_color_hex(0x234567) );

        lv_color_hex是一个函数，可以把一个RGB的十六进制数字，转换成一个lv_color_t类型的变量

//设置背景 透明度
void lv_style_set_bg_opa(lv_style_t * style, lv_opa_t value);
		value:0表示全透明(看不见了),值越大越不透明   [0-255]

    /**
   * Opacity percentages.
   */
      enum _lv_opa_t {
          LV_OPA_TRANSP = 0,
          LV_OPA_0      = 0,   // 0%的 不透明度, 全透明
          LV_OPA_10     = 25,  // 10%的 不透明度  90%透明
          LV_OPA_20     = 51,  //  20%的
          LV_OPA_30     = 76,
          LV_OPA_40     = 102,
          LV_OPA_50     = 127,  //LV_OPA_50  50%的透明度
          LV_OPA_60     = 153,
          LV_OPA_70     = 178,
          LV_OPA_80     = 204,
          LV_OPA_90     = 229,
          LV_OPA_100    = 255,
          LV_OPA_COVER  = 255,   //cover覆盖，不透明
      };
      示例:
		  lv_style_set_opa(&style,200);
      lv_style_set_opa(&style, OV_OPA_90)

设置边框弧度
	void lv_style_set_radius(lv_style_t * style, int32_t value)
        value表示弧度半径,0表示没有弧度,就是直角,半径越大,弧度越大

设置边框宽度
	void lv_style_set_border_width(lv_style_t * style, int32_t value);
		示例:
		lv_style_set_border_width(&style,5);

设置边框颜色
	void lv_style_set_border_color(lv_style_t * style, lv_color_t value)

设置边框透明度
  void lv_style_set_border_opa(lv_style_t * style, lv_opa_t value)

```

- (3) 把样式作用于指定的控件(给对象添加样式)
```c++
void lv_obj_add_style(lv_obj_t * obj, const lv_style_t * style,
                          lv_style_selector_t selector)
		obj:一个对象的地址(你要把样式作用于哪一个控件)
    style:样式变量的地址
    
    selector:在什么情况下这个样式生效
            	控件可以处于以下状态的组合：
                LV_STATE_DEFAULT：正常释放状态
                LV_STATE_CHECKED：切换或选中状态
                LV_STATE_FOCUSED：通过键盘或编码器聚焦或通过触摸板/鼠标点击
                LV_STATE_FOCUS_KEY：通过键盘或编码器聚焦但不通过触摸板/鼠标点击
                LV_STATE_EDITED：由编码器编辑
                LV_STATE_HOVERED：由鼠标悬停（目前不支持）
                LV_STATE_PRESSED：正在按下
                LV_STATE_SCROLLED：正在滚动
                LV_STATE_DISABLED：禁用状态
                LV_STATE_USER_1：自定义状态
                LV_STATE_USER_2：自定义状态
                LV_STATE_USER_3：自定义状态
                LV_STATE_USER_4：自定义状态
    示例:
		lv_obj_add_style(obj,&style,LV_STATE_DEFAULT);

```
