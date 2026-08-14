# KForge

> 一个C艹 基础工具库与算法学习项目。
## 简介

KForge 是一个个人 C++ 学习项目，作为算法与 C++ 特性学习的实验场。

## 项目结构

```
kForge/
├── base/              # 核心基础库
│   ├── KF.hpp         # 模块声明与公共接口
│   ├── KFIO.cpp       # 文件读取
│   ├── KSON.cpp       # 自定义配置格式(json like)
│   ├── CLI.cpp        # 控制台交互
│   ├── KTIMER.cpp     # 计时器
│   ├── UTILITY.cpp    # 通用工具函数
│   └── references.md  # API 使用手册
├── test/              # 模块测试与系统测试
│   └── dbgxxx.cpp     # 自动化系统测试框架
├── study/             # 算法与 C++ 特性学习代码
│   ├── algorithm/     # 算法
│   └── cpp/           # C++ 进阶特性实验
├── config/            # 配置
├── init_build.bat     # 项目初始化
├── cancel_init.bat    # 取消初始化
└── projects/          # 项目（预留）
```
第一次使用一定要先点击`init_build`
且需要MSVC环境

## API 文档

详见 [`base/references.md`](base/references.md)。

## 变更日志

## 贡献与许可

本项目为个人学习项目，持续迭代中。
