# Warrior

Warrior 是一个基于 Unreal Engine 5.7 开发的第三人称动作游戏项目。项目以近战战斗为核心，使用 Gameplay Ability System 构建角色能力、攻击、受击、怒气、格挡、翻滚、拾取和敌人行为等系统。

## 项目特性

- 第三人称角色控制与 Enhanced Input 输入系统
- 基于 Gameplay Ability System 的玩家和敌人能力框架
- 轻攻击、重攻击、格挡、翻滚、怒气和武器技能
- 敌人 AI、行为树、Boss 血条和波次生存模式
- Motion Warping 辅助近战攻击与动画对齐
- UMG 游戏界面，包括主菜单、暂停、胜利、失败、状态栏和技能图标
- 存档、拾取物、投射物、GameplayCue 和 Niagara 特效集成

## 技术栈

- Unreal Engine 5.7
- C++
- Blueprint
- Gameplay Ability System
- Enhanced Input
- Motion Warping
- UMG
- Niagara
- AI Module / Behavior Tree

## 目录结构

```text
Config/                 项目配置
Content/                游戏资产、蓝图、地图、UI、动画和特效
Source/Warrior/         C++ 游戏源码
Source/Warrior.Target.cs
Source/WarriorEditor.Target.cs
Warrior.uproject        UE 项目文件
```

主要内容目录：

```text
Content/Maps/           地图
Content/PlayerCharacter/ 玩家角色相关蓝图、动画和能力
Content/EnemyCharacter/  敌人角色、AI、行为树和能力
Content/GameModes/       游戏模式和波次配置
Content/Items/           武器、拾取物和投射物
Content/Widgets/         UMG UI
Content/GameplayCues/    GameplayCue 表现资源
```

## 获取项目

本项目使用 Git LFS 管理 `.uasset`、`.umap` 等大体积资源。克隆前请先安装 Git LFS。

```powershell
git lfs install
git clone https://github.com/Nmashiro/Warrior.git
cd Warrior
git lfs pull
```

## 打开方式

1. 安装 Unreal Engine 5.7。
2. 确认 Git LFS 资源已经拉取完成。
3. 双击 `Warrior.uproject` 打开项目。
4. 如需编译 C++，在 Unreal Editor 中选择重新生成项目文件或使用 IDE 编译 `WarriorEditor`。

## 注意事项

- 仓库不包含本机缓存、编译产物、打包成品、编辑器临时文件和私人辅助配置。
- 部分音乐素材未包含在仓库中，打开项目时如出现缺失音频引用，可自行替换对应资源。
- 第三方资产版权归原作者所有，请在使用、分发或商用前确认相关授权。

## License

本项目用于学习与作品展示。代码和自制内容的使用许可可根据实际需要补充。
