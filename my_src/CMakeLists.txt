# 添加可执行文件
add_executable(main
    main.c
    mouse_cursor_icon.c
    game2048.c
    snake.c
    fivegame.c
    thunder.c
    music.c
    chinese_chess.c
    SiYuanSongTi.c
    SiYuanHeiTi.c
    # 自己写的其他.c也要加进来
    
)

# 确保在链接前lv_conf.h已被复制
add_dependencies(main lvgl)

# 链接库
target_link_libraries(main
    lvgl
    lvgl::examples
    lvgl::demos
    lvgl::thorvg
    m
    pthread
)

# 添加自定义目标以便运行
add_custom_target(run 
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/main 
    DEPENDS main
)