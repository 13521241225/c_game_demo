# 塞尔达 · C 语言课设游戏 Demo

基于 **C 语言 + EasyX 图形库** 开发的 2D 俯视角动作游戏，是《高级语言程序设计》课程设计项目。

## 项目简介

玩家操控角色在迷宫地图中移动，用鼠标发射魔法弹消灭具有不同 AI 行为的恶魔怪物，逐步通关两个关卡。游戏支持注册登录、进度存档读档、排行榜查询和 PVP 双人对战模式。

## 功能特性

- **登录与注册**：玩家账号管理
- **游戏大厅**：新游戏 / 继续游戏 / 排行榜 / PVP 双人对战等入口
- **核心战斗**：鼠标发射魔法弹，消灭多种恶魔怪物
- **敌人 AI**：多种怪物拥有不同的行为模式
- **关卡系统**：两个关卡
- **存档与读档**：进度保存到 `save_玩家名.dat`

## 技术栈

- 语言：C（兼容 C++ 编译器，使用 vector / 引用 / bool）
- 图形库：EasyX（graphics.h），双缓冲 + 透明 PNG（AlphaBlend）
- IDE：Visual Studio 2022（MSVC x64）
- 系统：Windows 10/11 x64
- 编码：源文件 GBK，`#define _CRT_SECURE_NO_WARNINGS 1`

## 项目结构

```
c_game_demo/
├── Project1/            # VS 工程目录
│   ├── Start_Part.cpp   # 主源文件（游戏逻辑）
│   ├── *.png            # 背景与 UI 图片
│   ├── idle/ walk/ link/ ...   # 角色与怪物 PNG 精灵文件夹
│   ├── map2.txt         # 关卡地图
│   ├── pvp_map.txt      # PVP 对战地图
│   └── player_list.txt  # 玩家排行榜数据
├── Project1.slnx        # 解决方案文件
└── README.md
```

## 运行方式

1. 用 VS2022 打开 `Project1.slnx`，编译运行（x64）。
2. 或直接运行编译出的 `Project1.exe`。
3. 首次运行需确保同目录包含图片资源与数据文件；`player_list.txt` 内容为 `0`。

## 操作说明

- `WASD`：移动角色
- `鼠标`：发射魔法弹
- `F`：与传送门 / 符文等交互
